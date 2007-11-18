//Author: Dodji Seketeli
/*
 *This file is part of the Nemiver project
 *
 *Nemiver is free software; you can redistribute
 *it and/or modify it under the terms of
 *the GNU General Public License as published by the
 *Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Nemiver is distributed in the hope that it will
 *be useful, but WITHOUT ANY WARRANTY;
 *without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include "config.h"

#ifdef WITH_VARIABLE_WALKER

#include <map>
#include <list>
#include <glib/gi18n.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treerowreference.h>
#include "common/nmv-exception.h"
#include "nmv-local-vars-inspector2.h"
#include "nmv-variables-utils2.h"
#include "nmv-ui-utils.h"
#include "nmv-i-workbench.h"
#include "nmv-i-var-list-walker.h"

using namespace nemiver::common ;
namespace vutil=nemiver::variables_utils2 ;

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct LocalVarsInspector2::Priv {
private:
    Priv ();
public:
    IDebuggerSafePtr debugger ;
    IVarListWalkerSafePtr local_var_list_walker;
    IVarListWalkerSafePtr function_args_var_list_walker;
#ifdef WITH_GLOBAL_VARIABLES
    IVarListWalkerSafePtr global_variables_walker_list;
#endif //WITH_GLOBAL_VARIABLES

    IWorkbench &workbench ;
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_store ;
    Gtk::TreeModel::iterator cur_selected_row ;
#ifdef WITH_GLOBAL_VARIABLES
    SafePtr<Gtk::TreeRowReference> global_variables_row_ref ;
#endif //WITH_GLOBAL_VARIABLES
    SafePtr<Gtk::TreeRowReference> local_variables_row_ref ;
    SafePtr<Gtk::TreeRowReference> function_arguments_row_ref ;
    std::map<UString, IDebugger::VariableSafePtr> local_vars_to_set ;
    std::map<UString, IDebugger::VariableSafePtr> function_arguments_to_set ;
    SafePtr<Gtk::Menu> contextual_menu ;
    Gtk::MenuItem *dereference_mi ;
    UString previous_function_name ;
    bool is_new_frame ;

    Priv (IDebuggerSafePtr &a_debugger,
          IWorkbench &a_workbench) :
        workbench (a_workbench),
        dereference_mi (0),
        is_new_frame (false)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (a_debugger) ;
        debugger = a_debugger ;
        build_tree_view () ;
        re_init_tree_view () ;
        connect_to_debugger_signals () ;
        //init_graphical_signals () ;
    }

    void build_tree_view ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (tree_view) {return;}
        //create a default tree store and a tree view
        tree_store = Gtk::TreeStore::create
            (vutil::get_variable_columns ()) ;
        tree_view.reset (new Gtk::TreeView (tree_store)) ;
        tree_view->set_headers_clickable (true) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        sel->set_mode (Gtk::SELECTION_SINGLE) ;

        //create the columns of the tree view
        tree_view->append_column (_("Variable"),
                                 vutil::get_variable_columns ().name) ;
        Gtk::TreeViewColumn * col = tree_view->get_column (0) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        col->add_attribute (*col->get_first_cell_renderer (),
                            "foreground-gdk",
                            vutil::VariableColumns::FG_COLOR_OFFSET) ;

        tree_view->append_column (_("Value"), vutil::get_variable_columns ().value) ;
        col = tree_view->get_column (1) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        col->add_attribute (*col->get_first_cell_renderer (),
                            "foreground-gdk",
                            vutil::VariableColumns::FG_COLOR_OFFSET) ;

        tree_view->append_column (_("Type"),
                                  vutil::get_variable_columns ().type_caption);
        col = tree_view->get_column (2) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
    }

    void re_init_tree_view ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;
        tree_store->clear () ;
        previous_function_name = "" ;
        is_new_frame = true ;

        //****************************************************
        //add two rows: local variables and function arguments.
        //Store row refs off both rows
        //****************************************************
        Gtk::TreeModel::iterator it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[vutil::get_variable_columns ().name] = _("Local Variables");
        local_variables_row_ref.reset
                    (new Gtk::TreeRowReference (tree_store,
                                                tree_store->get_path (it))) ;
        THROW_IF_FAIL (local_variables_row_ref) ;

        it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[vutil::get_variable_columns ().name] = _("Function Arguments");
        function_arguments_row_ref.reset
            (new Gtk::TreeRowReference (tree_store, tree_store->get_path (it)));
        THROW_IF_FAIL (function_arguments_row_ref) ;

#ifdef WITH_GLOBAL_VARIABLES
        it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[vutil::get_variable_columns ().name] = _("Global Variables");
        global_variables_row_ref.reset
            (new Gtk::TreeRowReference (tree_store, tree_store->get_path (it)));
        THROW_IF_FAIL (global_variables_row_ref) ;
#endif //WITH_GLOBAL_VARIABLES
    }

    void get_function_arguments_row_iterator (Gtk::TreeModel::iterator &a_it)
    {
        THROW_IF_FAIL (function_arguments_row_ref) ;
        a_it = tree_store->get_iter (function_arguments_row_ref->get_path ()) ;
    }

    void get_local_variables_row_iterator (Gtk::TreeModel::iterator &a_it)
    {
        THROW_IF_FAIL (local_variables_row_ref) ;
        a_it = tree_store->get_iter (local_variables_row_ref->get_path ()) ;
    }

#ifdef WITH_GLOBAL_VARIABLES
    void get_global_variables_row_iterator (Gtk::TreeModel::iterator &a_it)
    {
        THROW_IF_FAIL (global_variables_row_ref) ;
        a_it = tree_store->get_iter (global_variables_row_ref->get_path ()) ;
    }
#endif //WITH_GLOBAL_VARIABLES

    IVarListWalkerSafePtr get_local_vars_walker_list ()
    {
        if (!local_var_list_walker) {
            local_var_list_walker = create_variable_walker_list () ;
            THROW_IF_FAIL (local_var_list_walker) ;
            local_var_list_walker->variable_visited_signal ().connect
                (sigc::mem_fun
                 (*this, &LocalVarsInspector2::Priv::on_local_variable_visited_signal)) ;
        }
        return local_var_list_walker ;
    }

    IVarListWalkerSafePtr get_function_args_vars_walker_list ()
    {
        if (!function_args_var_list_walker) {
            function_args_var_list_walker = create_variable_walker_list () ;
            THROW_IF_FAIL (function_args_var_list_walker) ;
            function_args_var_list_walker->variable_visited_signal ().connect
                (sigc::mem_fun
                 (*this, &LocalVarsInspector2::Priv::on_func_arg_visited_signal)) ;
        }
        return function_args_var_list_walker ;
    }

#ifdef WITH_GLOBAL_VARIABLES
    IVarListWalkerSafePtr get_global_variables_walker_list ()
    {
        if (!global_variables_walker_list) {
            global_variables_walker_list = create_variable_walker_list () ;
            THROW_IF_FAIL (global_variables_walker_list) ;
            global_variables_walker_list->variable_visited_signal ().connect
                (sigc::mem_fun
                 (*this, &LocalVarsInspector2::Priv::on_global_variable_visited_signal)) ;
        }
        return global_variables_walker_list ;
    }
#endif //WITH_GLOBAL_VARIABLES

    IVarListWalkerSafePtr create_variable_walker_list ()
    {
        DynamicModule::Loader *loader =
            workbench.get_dynamic_module ().get_module_loader ();
        THROW_IF_FAIL (loader) ;
        DynamicModuleManager *module_manager = loader->get_dynamic_module_manager ();
        THROW_IF_FAIL (module_manager) ;
        IVarListWalkerSafePtr result =
            module_manager->load_iface<IVarListWalker> ("varwalkerlist",
                                                        "IVarListWalker");
        THROW_IF_FAIL (result) ;
        result->initialize (debugger) ;
        return result ;
    }


    void connect_to_debugger_signals ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (debugger) ;
        debugger->stopped_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_stopped_signal)) ;
        debugger->local_variables_listed_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_local_variables_listed_signal)) ;
        debugger->frames_arguments_listed_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_frames_params_listed_signal)) ;
#ifdef WITH_GLOBAL_VARIABLES
        debugger->global_variables_listed_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_global_variables_listed_signal)) ;
#endif //WITH_GLOBAL_VARIABLES

        /*
        debugger->variable_value_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_variable_value_signal)) ;
        debugger->variable_type_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_variable_type_signal)) ;
        debugger->pointed_variable_value_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_pointed_variable_value_signal)) ;
            */
    }

    void set_local_variables (const std::list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        clear_local_variables () ;
        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            append_a_local_variable (*it) ;
        }
    }

    void set_function_arguments (const std::list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        clear_function_arguments () ;
        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            append_a_function_argument (*it) ;
        }
    }

#ifdef WITH_GLOBAL_VARIABLES
    void set_global_variables (const std::list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        clear_global_variables () ;
        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            append_a_global_variable (*it) ;
        }
    }
#endif //WITH_GLOBAL_VARIABLES

    void clear_local_variables ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_store) ;
        Gtk::TreeModel::iterator row_it;
        get_local_variables_row_iterator (row_it) ;
        Gtk::TreeModel::Children rows = row_it->children ();
        for (row_it = rows.begin (); row_it != rows.end (); ++row_it) {
            tree_store->erase (row_it) ;
        }
    }

    void clear_function_arguments ()
    {
        THROW_IF_FAIL (tree_store) ;
        Gtk::TreeModel::iterator row_it;
        get_function_arguments_row_iterator (row_it) ;
        Gtk::TreeModel::Children rows = row_it->children ();
        for (row_it = rows.begin (); row_it != rows.end (); ++row_it) {
            tree_store->erase (*row_it) ;
        }
    }

#ifdef WITH_GLOBAL_VARIABLES
    void clear_global_variables ()
    {
        THROW_IF_FAIL (tree_store) ;
        Gtk::TreeModel::iterator row_it;
        get_global_variables_row_iterator (row_it) ;
        Gtk::TreeModel::Children rows = row_it->children ();
        for (row_it = rows.begin (); row_it != rows.end (); ++row_it) {
            tree_store->erase (*row_it) ;
        }
    }
#endif //WITH_GLOBAL_VARIABLES

    void append_a_local_variable (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_view && tree_store) ;

        Gtk::TreeModel::iterator parent_row_it ;
        get_local_variables_row_iterator (parent_row_it) ;
        vutil::append_a_variable (a_var, *tree_view, tree_store, parent_row_it) ;
        tree_view->expand_row (tree_store->get_path (parent_row_it), false) ;

    }

    void append_a_function_argument (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_view && tree_store) ;

        Gtk::TreeModel::iterator parent_row_it ;
        get_function_arguments_row_iterator (parent_row_it) ;
        vutil::append_a_variable (a_var, *tree_view, tree_store, parent_row_it) ;
        tree_view->expand_row (tree_store->get_path (parent_row_it), false) ;
    }

#ifdef WITH_GLOBAL_VARIABLES
    void append_a_global_variable (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_view && tree_store) ;

        Gtk::TreeModel::iterator parent_row_it ;
        get_global_variables_row_iterator (parent_row_it) ;
        vutil::append_a_variable (a_var, *tree_view, tree_store, parent_row_it) ;
        tree_view->expand_row (tree_store->get_path (parent_row_it), false) ;
    }
#endif //WITH_GLOBAL_VARIABLES

    void update_a_local_variable (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_view) ;

        Gtk::TreeModel::iterator parent_row_it ;
        get_local_variables_row_iterator (parent_row_it) ;
        vutil::update_a_variable (a_var, *tree_view,
                                  parent_row_it,
                                  true, false);
    }

    void update_a_function_argument (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_view) ;
        Gtk::TreeModel::iterator parent_row_it ;
        get_function_arguments_row_iterator (parent_row_it) ;
        vutil::update_a_variable (a_var, *tree_view, parent_row_it,
                                  true, false) ;
    }

#ifdef WITH_GLOBAL_VARIABLES
    void update_a_global_variable (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_view) ;
        Gtk::TreeModel::iterator parent_row_it ;
        get_global_variables_row_iterator (parent_row_it) ;
        vutil::update_a_variable (a_var, *tree_view, parent_row_it,
                                  true, false) ;
    }
#endif //WITH_GLOBAL_VARIABLES

    //****************************
    //<debugger signal handlers>
    //****************************
    void on_stopped_signal (const UString &a_reason,
                            bool a_has_frame,
                            const IDebugger::Frame &a_frame,
                            int a_thread_id,
                            const UString &a_cookie)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        if (a_frame.line () || a_thread_id || a_cookie.empty ()) {}

        NEMIVER_TRY

        LOG_DD ("stopped, reason: " << a_reason) ;
        if (a_reason == "exited-signaled"
            || a_reason == "exited-normally"
            || a_reason == "exited") {
            return ;
        }

        THROW_IF_FAIL (debugger) ;
        if (a_has_frame) {
            LOG_DD ("prev frame address: '" << previous_function_name << "'") ;
            LOG_DD ("cur frame address: " << a_frame.function_name () << "'") ;
            if (previous_function_name == a_frame.function_name ()) {
                is_new_frame = false ;
            } else {
                is_new_frame = true ;
            }
            THROW_IF_FAIL (tree_store) ;
            if (is_new_frame) {
                LOG_DD ("init tree view") ;
                re_init_tree_view () ;
                debugger->list_local_variables () ;
#ifdef WITH_GLOBAL_VARIABLES
                debugger->list_global_variables () ;
#endif //WITH_GLOBAL_VARIABLES
            } else {
                IVarListWalkerSafePtr walker_list = get_local_vars_walker_list () ;
                THROW_IF_FAIL (walker_list) ;
                walker_list->do_walk_variables () ;
                walker_list = get_function_args_vars_walker_list () ;
                THROW_IF_FAIL (walker_list) ;
                walker_list->do_walk_variables () ;
                //walk the variables to detect the
                //pointer (direct or indirect) members and update them.
                //update_derefed_pointer_variables () ;
            }
            previous_function_name = a_frame.function_name () ;
        }

        NEMIVER_CATCH
    }

    void on_local_variables_listed_signal
                            (const list<IDebugger::VariableSafePtr> &a_vars,
                             const UString &a_cookie)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        if (a_cookie == "") {}

        NEMIVER_TRY

        IVarListWalkerSafePtr walker_list = get_local_vars_walker_list () ;
        THROW_IF_FAIL (walker_list) ;
        walker_list->remove_variables () ;
        walker_list->append_variables (a_vars) ;
        walker_list->do_walk_variables () ;
        NEMIVER_CATCH
    }

    void on_frames_params_listed_signal
            (const map<int, list<IDebugger::VariableSafePtr> >&a_frames_params,
             const UString &a_cookie)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        if (a_cookie == "") {}

        NEMIVER_TRY

        IVarListWalkerSafePtr walker_list = get_function_args_vars_walker_list () ;
        THROW_IF_FAIL (walker_list) ;

        map<int, list<IDebugger::VariableSafePtr> >::const_iterator it ;
        it = a_frames_params.find (0) ;
        if (it == a_frames_params.end ()) {return;}

        walker_list->remove_variables () ;
        walker_list->append_variables (it->second) ;
        walker_list->do_walk_variables () ;

        NEMIVER_CATCH
    }

#ifdef WITH_GLOBAL_VARIABLES
    void on_global_variables_listed_signal
                                (const list<IDebugger::VariableSafePtr> &a_vars,
                                 const UString &a_cookie)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        if (a_cookie == "") {}

        NEMIVER_TRY

        IVarListWalkerSafePtr walker_list = get_global_variables_walker_list () ;
        THROW_IF_FAIL (walker_list) ;

        walker_list->remove_variables () ;
        walker_list->append_variables (a_vars) ;
        walker_list->do_walk_variables () ;

        NEMIVER_CATCH
    }
#endif //WITH_GLOBAL_VARIABLES

    //****************************
    //</debugger signal handlers>
    //****************************

    void on_local_variable_visited_signal (const IVarWalkerSafePtr &a_walker)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (a_walker->get_variable ()) ;

        if (is_new_frame) {
            LOG_DD ("going to append: " << a_walker->get_variable ()->name ()) ;
            append_a_local_variable (a_walker->get_variable ()) ;
        } else {
            LOG_DD ("going to update: " << a_walker->get_variable ()->name ()) ;
            update_a_local_variable (a_walker->get_variable ()) ;
        }

        NEMIVER_CATCH
    }

    void on_func_arg_visited_signal (const IVarWalkerSafePtr &a_walker)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (a_walker->get_variable ()) ;

        if (is_new_frame) {
            append_a_function_argument (a_walker->get_variable ()) ;
        } else {
            update_a_function_argument (a_walker->get_variable ()) ;
        }

        NEMIVER_CATCH
    }

#ifdef WITH_GLOBAL_VARIABLES
    void on_global_variable_visited_signal (const IVarWalkerSafePtr &a_walker)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (a_walker->get_variable ()) ;

        if (is_new_frame) {
            append_a_global_variable (a_walker->get_variable ()) ;
        } else {
            update_a_global_variable (a_walker->get_variable ()) ;
        }

        NEMIVER_CATCH
    }
#endif //WITH_GLOBAL_VARIABLES
};//end LocalVarInspector2::Priv

LocalVarsInspector2::LocalVarsInspector2 (IDebuggerSafePtr &a_debugger,
                                          IWorkbench &a_workbench)
{
    m_priv.reset (new Priv (a_debugger, a_workbench));
}

LocalVarsInspector2::~LocalVarsInspector2 ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

Gtk::Widget&
LocalVarsInspector2::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    return *m_priv->tree_view ;
}

void
LocalVarsInspector2::show_local_variables_of_current_function ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->debugger) ;

    re_init_widget () ;
    m_priv->debugger->list_local_variables () ;
}

void
LocalVarsInspector2::re_init_widget ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    THROW_IF_FAIL (m_priv) ;

    m_priv->re_init_tree_view () ;
}

NEMIVER_END_NAMESPACE (nemiver)

#endif //WITH_VARIABLE_WALKER

