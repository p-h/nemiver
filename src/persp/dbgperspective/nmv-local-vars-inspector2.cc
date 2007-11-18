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
#include <map>
#include <list>
#include <glib/gi18n.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treerowreference.h>
#include "common/nmv-exception.h"
#include "nmv-local-vars-inspector2.h"
#include "nmv-variables-utils.h"
#include "nmv-ui-utils.h"
#include "nmv-i-workbench.h"
#include "nmv-i-var-walker-list.h"

using namespace nemiver::common ;
using namespace nemiver::variables_utils ;

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct LocalVarsInspector2::Priv {
private:
    Priv ();
public:
    IDebuggerSafePtr debugger ;
    IVarWalkerListSafePtr local_var_walker_list;
    IVarWalkerListSafePtr function_args_var_walker_list;

    IWorkbench &workbench ;
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_store ;
    Gtk::TreeModel::iterator cur_selected_row ;
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
        using namespace variables_utils ;

        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (tree_view) {return;}
        //create a default tree store and a tree view
        tree_store = Gtk::TreeStore::create
            (variables_utils::get_variable_columns ()) ;
        tree_view.reset (new Gtk::TreeView (tree_store)) ;
        tree_view->set_headers_clickable (true) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        sel->set_mode (Gtk::SELECTION_SINGLE) ;

        //create the columns of the tree view
        tree_view->append_column (_("Variable"),
                                 variables_utils::get_variable_columns ().name) ;
        Gtk::TreeViewColumn * col = tree_view->get_column (0) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        col->add_attribute (*col->get_first_cell_renderer (),
                            "foreground-gdk",
                            variables_utils::VariableColumns::FG_COLOR_OFFSET) ;

        tree_view->append_column (_("Value"), get_variable_columns ().value) ;
        col = tree_view->get_column (1) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        col->add_attribute (*col->get_first_cell_renderer (),
                            "foreground-gdk",
                            variables_utils::VariableColumns::FG_COLOR_OFFSET) ;

        tree_view->append_column (_("Type"),
                                  get_variable_columns ().type_caption);
        col = tree_view->get_column (2) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
    }

    void re_init_tree_view ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;
        tree_store->clear () ;

        //****************************************************
        //add two rows: local variables and function arguments.
        //Store row refs off both rows
        //****************************************************
        Gtk::TreeModel::iterator it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[variables_utils::get_variable_columns ().name] =
                                                        _("Local Variables");
        local_variables_row_ref.reset
                    (new Gtk::TreeRowReference (tree_store,
                                                tree_store->get_path (it))) ;
        THROW_IF_FAIL (local_variables_row_ref) ;

        it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[variables_utils::get_variable_columns ().name] =
                                                        _("Function Arguments");
        function_arguments_row_ref.reset
            (new Gtk::TreeRowReference (tree_store, tree_store->get_path (it))) ;
        THROW_IF_FAIL (function_arguments_row_ref) ;
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

    IVarWalkerListSafePtr get_local_vars_walker_list ()
    {
        if (!local_var_walker_list) {
            local_var_walker_list = create_variable_walker_list () ;
        }
        THROW_IF_FAIL (local_var_walker_list) ;
        return local_var_walker_list ;
    }

    IVarWalkerListSafePtr get_function_args_vars_walker_list ()
    {
        if (!function_args_var_walker_list) {
            function_args_var_walker_list = create_variable_walker_list () ;
        }
        THROW_IF_FAIL (function_args_var_walker_list) ;
        return function_args_var_walker_list ;
    }

    IVarWalkerListSafePtr create_variable_walker_list ()
    {
        DynamicModule::Loader *loader =
            workbench.get_dynamic_module ().get_module_loader ();
        THROW_IF_FAIL (loader) ;
        DynamicModuleManager *module_manager = loader->get_dynamic_module_manager ();
        THROW_IF_FAIL (module_manager) ;
        IVarWalkerListSafePtr result =
            module_manager->load_iface<IVarWalkerList> ("varwalkerlist",
                                                        "IVarWalkerList");
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
        /*
        debugger->variable_value_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_variable_value_signal)) ;
        debugger->variable_type_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_variable_type_signal)) ;
        debugger->pointed_variable_value_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_pointed_variable_value_signal)) ;
            */
    }

    void update_a_variable_node (const IDebugger::VariableSafePtr &a_var,
                                 Gtk::TreeModel::iterator &a_iter,
                                 bool a_handle_highlight,
                                 bool a_is_new_frame)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (a_var) {
            LOG_DD ("going to really update variable '" << a_var->name () << "'") ;
        } else {
            LOG_DD ("eek, got null variable") ;
            return ;
        }
        THROW_IF_FAIL (tree_view) ;

        (*a_iter)[get_variable_columns ().variable] = a_var ;
        UString var_name = a_var->name ();
        var_name.chomp () ;
        UString prev_var_name =
                (Glib::ustring)(*a_iter)[get_variable_columns ().name] ;
        LOG_DD ("Prev variable name: " << prev_var_name) ;
        LOG_DD ("new variable name: " << var_name) ;
        LOG_DD ("Didn't update variable name") ;
        if (prev_var_name == "") {
            (*a_iter)[get_variable_columns ().name] = var_name;
        }
        (*a_iter)[get_variable_columns ().is_highlighted]=false ;
        bool do_highlight = false ;
        if (a_handle_highlight && !a_is_new_frame) {
            UString prev_value =
                (UString) (*a_iter)[get_variable_columns ().value] ;
            if (prev_value != a_var->value ()) {
                do_highlight = true ;
            }
        }

        if (do_highlight) {
            LOG_DD ("do highlight variable") ;
            (*a_iter)[get_variable_columns ().is_highlighted]  = true;
            (*a_iter)[get_variable_columns ().fg_color]  = Gdk::Color ("red");
        } else {
            LOG_DD ("remove highlight from variable") ;
            (*a_iter)[get_variable_columns ().is_highlighted]  = false ;
            (*a_iter)[get_variable_columns ().fg_color]  =
                tree_view->get_style ()->get_text (Gtk::STATE_NORMAL);
        }

        (*a_iter)[get_variable_columns ().value] = a_var->value () ;
        (*a_iter)[get_variable_columns ().type] = a_var->type () ;
    }

    void set_local_variables (const std::list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_store) ;

        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            append_a_local_variable (*it) ;
        }
    }

    void set_function_arguments (const std::list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (tree_store) ;
        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            append_a_function_argument (*it) ;
        }
    }

    void append_a_local_variable (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        Gtk::TreeModel::iterator parent_row_it ;
        get_local_variables_row_iterator (parent_row_it) ;
        append_a_variable (a_var, parent_row_it) ;

    }

    void append_a_function_argument (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (function_arguments_row_ref) ;
        THROW_IF_FAIL (tree_store) ;

        Gtk::TreeModel::iterator parent_row_it ;
        get_local_variables_row_iterator (parent_row_it) ;
        append_a_variable (a_var, parent_row_it) ;
    }

    void append_a_variable (const IDebugger::VariableSafePtr &a_var,
                            Gtk::TreeModel::iterator &a_parent_row_it)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;

        Gtk::TreeModel::iterator row_it =
            tree_store->append (a_parent_row_it->children ()) ;
        update_a_variable_node (a_var, row_it, false, false) ;
        list<IDebugger::VariableSafePtr>::const_iterator it;
        for (it = a_var->members ().begin (); it != a_var->members ().end (); ++it) {
            append_a_variable (*it, row_it) ;
        }
    }

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
            previous_function_name = a_frame.function_name () ;
            THROW_IF_FAIL (tree_store) ;
            if (is_new_frame) {
                LOG_DD ("init tree view") ;
                re_init_tree_view () ;
                debugger->list_local_variables () ;
            } else {
                IVarWalkerListSafePtr walker_list = get_local_vars_walker_list () ;
                THROW_IF_FAIL (walker_list) ;
                walker_list->do_walk_variables () ;
                walker_list = get_function_args_vars_walker_list () ;
                THROW_IF_FAIL (walker_list) ;
                walker_list->do_walk_variables () ;
                //walk the variables to detect the
                //pointer (direct or indirect) members and update them.
                //update_derefed_pointer_variables () ;
            }
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

        IVarWalkerListSafePtr walker_list = get_local_vars_walker_list () ;
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

        IVarWalkerListSafePtr walker_list = get_function_args_vars_walker_list () ;
        THROW_IF_FAIL (walker_list) ;

        map<int, list<IDebugger::VariableSafePtr> >::const_iterator it ;
        it = a_frames_params.find (0) ;
        if (it == a_frames_params.end ()) {return;}

        walker_list->remove_variables () ;
        walker_list->append_variables (it->second) ;
        walker_list->do_walk_variables () ;

        NEMIVER_CATCH
    }
    //****************************
    //</debugger signal handlers>
    //****************************

    void on_variable_visited_signal (const IVarWalkerSafePtr &a_walker)
    {
        NEMIVER_TRY

        THROW_IF_FAIL (a_walker->get_variable ()) ;

        append_a_local_variable (a_walker->get_variable ()) ;

        NEMIVER_CATCH
    }
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
