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
#include "nmv-local-vars-inspector.h"
#include "nmv-variables-utils.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"
#include "nmv-i-workbench.h"

NEMIVER_BEGIN_NAMESPACE (nemiver) ;

struct LocalVarsInspector::Priv {
private:
    Priv ();
public:
    IDebuggerSafePtr debugger ;
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
        init_graphical_signals () ;
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

    bool set_a_variable_type (const UString &a_var_name,
                              const UString &a_type,
                              Gtk::TreeModel::iterator &a_it)
    {
        using namespace variables_utils ;

        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;
        Gtk::TreeModel::iterator row_it ;
        LOG_DD ("going to get iter on variable of name: '" << a_var_name << "'") ;

        Gtk::TreeModel::iterator root_it ;
        get_local_variables_row_iterator (root_it) ;
        bool ret = get_variable_iter_from_qname (a_var_name, root_it, row_it) ;
        if (!ret) {
            get_function_arguments_row_iterator (root_it) ;
            ret = get_variable_iter_from_qname (a_var_name, root_it, row_it) ;
        }
        if (!ret) {
            return false ;
        }
        THROW_IF_FAIL (row_it) ;
        set_a_variable_type_real (row_it, a_type) ;
        a_it = row_it ;
        return true ;
    }

    bool set_a_variable_type (const UString &a_var_name, const UString &a_type)
    {
        Gtk::TreeModel::iterator it ;
        return set_a_variable_type (a_var_name, a_type, it) ;
    }

    void set_variables_real (const std::list<IDebugger::VariableSafePtr> &a_vars,
                             Gtk::TreeRowReference &a_vars_row_ref)
    {
        using namespace variables_utils ;
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        Gtk::TreeModel::iterator vars_row_it =
            tree_store->get_iter (a_vars_row_ref.get_path ()) ;
        Gtk::TreeModel::iterator tmp_iter ;

        THROW_IF_FAIL (vars_row_it) ;
        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            if (!(*it)) {continue ;}
            append_a_variable (*it, vars_row_it, tree_store,
                               *tree_view, *debugger,
                               true, is_new_frame, tmp_iter) ;
        }
    }

    //*******************************************************
    //before setting a variable, query its full description
    //*******************************************************

    void set_local_variables (const std::list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;
        THROW_IF_FAIL (local_variables_row_ref) ;

        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            local_vars_to_set[(*it)->name ()] = *it ;
            debugger->print_variable_value ((*it)->name ()) ;
        }
    }

    void set_function_arguments
                (const std::list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;
        THROW_IF_FAIL (function_arguments_row_ref) ;

        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            function_arguments_to_set[(*it)->name ()] = *it ;
            debugger->print_variable_value ((*it)->name ()) ;
        }

    }

    void show_variable_type_in_dialog ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        using namespace variables_utils ;

        if (!cur_selected_row) {return;}
        UString type =
            (Glib::ustring) (*cur_selected_row)[get_variable_columns ().type] ;
        UString message ;
        message.printf (_("Variable type is: \n %s"), type.c_str ()) ;

        IDebugger::VariableSafePtr variable =
            (IDebugger::VariableSafePtr)
                cur_selected_row->get_value (get_variable_columns ().variable);
        THROW_IF_FAIL (variable) ;
        //message += "\nDumped for debug: \n" ;
        //variable->to_string (message, false) ;
        ui_utils::display_info (message) ;
    }

    void print_pointed_variable_value ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        if (!cur_selected_row) {return;}

        IDebugger::VariableSafePtr  variable =
            (IDebugger::VariableSafePtr) cur_selected_row->get_value
                            (variables_utils::get_variable_columns ().variable) ;
        THROW_IF_FAIL (variable) ;
        UString qname ;
        variable->build_qname (qname) ;
        debugger->print_pointed_variable_value (qname) ;
    }

    void connect_to_debugger_signals ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        THROW_IF_FAIL (debugger) ;
        debugger->local_variables_listed_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_local_variables_listed_signal)) ;
        debugger->stopped_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_stopped_signal)) ;
        debugger->variable_value_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_variable_value_signal)) ;
        debugger->variable_type_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_variable_type_signal)) ;
        debugger->frames_params_listed_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_frames_params_listed_signal)) ;
        debugger->pointed_variable_value_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_pointed_variable_value_signal)) ;
    }

    void init_graphical_signals ()
    {
        THROW_IF_FAIL (tree_view) ;
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection () ;
        THROW_IF_FAIL (selection) ;
        selection->signal_changed ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_selection_changed_signal));
        tree_view->signal_row_expanded ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_row_expanded_signal)) ;
        tree_view->signal_row_activated ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_row_activated_signal)) ;
    }

    void on_local_variables_listed_signal
                                (const list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        set_local_variables (a_vars) ;

        NEMIVER_CATCH
    }

    void on_frames_params_listed_signal
            (const map<int, list<IDebugger::VariableSafePtr> > &a_frames_params)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (NMV_DEFAULT_DOMAIN) ;
        NEMIVER_TRY

        map<int, list<IDebugger::VariableSafePtr> >::const_iterator it ;
        it = a_frames_params.find (0) ;
        if (it == a_frames_params.end ()) {return;}

        set_function_arguments (it->second) ;

        NEMIVER_CATCH
    }


    void on_stopped_signal (const UString &a_reason,
                            bool a_has_frame,
                            const IDebugger::Frame &a_frame,
                            int a_thread_id)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (a_frame.line () || a_thread_id) {}

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
            }
            debugger->list_local_variables () ;
        }

        NEMIVER_CATCH
    }

    void on_variable_value_signal (const UString &a_var_name,
                                   const IDebugger::VariableSafePtr &a_var)
    {
        using namespace variables_utils ;
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (tree_store) ;

        std::map<UString, IDebugger::VariableSafePtr>::const_iterator it, nil ;
        std::map<UString, IDebugger::VariableSafePtr> *map_ptr = NULL ;
        Gtk::TreeRowReference *row_ref = NULL ;

        it = local_vars_to_set.find (a_var_name) ;
        nil = local_vars_to_set.end () ;
        row_ref = local_variables_row_ref.get () ;
        map_ptr = &local_vars_to_set ;

        if (it == nil) {
            it = function_arguments_to_set.find (a_var_name) ;
            nil = function_arguments_to_set.end () ;
            row_ref = function_arguments_row_ref.get () ;
            map_ptr = &function_arguments_to_set ;
        }

        if (it == nil) {
            LOG_DD ("variable '" << a_var_name
                    << " does not belong in frame, arg vars or global vars") ;
            return ;
        }

        THROW_IF_FAIL (map_ptr) ;
        THROW_IF_FAIL (row_ref) ;

        Gtk::TreeModel::iterator row_it =
                                tree_store->get_iter (row_ref->get_path ()) ;

        LOG_DD ("adding variable '" << a_var_name << " to vars editor") ;

        a_var->type (it->second->type ()) ;
        Gtk::TreeModel::iterator var_iter ;
        if (get_variable_iter_from_qname
                                (a_var_name,
                                 tree_store->get_iter (row_ref->get_path ()),
                                 var_iter)) {
            THROW_IF_FAIL (var_iter) ;
            update_a_variable (a_var, *tree_view, is_new_frame, var_iter) ;
        } else {
            append_a_variable (a_var,
                               tree_store->get_iter (row_ref->get_path ()),
                               tree_store,
                               *tree_view,
                               *debugger,
                               true,
                               is_new_frame,
                               row_it) ;
        }

        map_ptr->erase (a_var_name) ;
        tree_view->expand_row (row_ref->get_path (), false) ;

        NEMIVER_CATCH
    }

    void on_variable_type_signal (const UString &a_var_name,
                                  const UString &a_type)
    {
        using namespace variables_utils ;
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        LOG_DD ("var: '" << a_var_name << "', type: '" << a_type << "'") ;

        Gtk::TreeModel::iterator row_it ;
        if (!set_a_variable_type (a_var_name, a_type, row_it)) {
            return ;
        }
        THROW_IF_FAIL (row_it) ;

        UString type =
            (Glib::ustring) row_it->get_value (get_variable_columns ().type) ;
        if (type == "" || !is_type_a_pointer (type)) {return;}

        THROW_IF_FAIL (tree_store) ;

        if (!row_it->children ().begin ()) {
            tree_store->append (row_it->children ()) ;
        }

        NEMIVER_CATCH
    }

    void on_tree_view_selection_changed_signal ()
    {
        using namespace variables_utils ;
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        THROW_IF_FAIL (tree_view) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        cur_selected_row = sel->get_selected () ;
        if (!cur_selected_row) {return;}
        IDebugger::VariableSafePtr variable =
        (IDebugger::VariableSafePtr)cur_selected_row->get_value
                                            (get_variable_columns ().variable) ;
        if (!variable) {return;}
        UString qname ;
        variable->build_qname (qname) ;
        LOG_DD ("row of variable '" << qname << "'") ;

        NEMIVER_CATCH
    }

    void on_tree_view_row_expanded_signal (const Gtk::TreeModel::iterator &a_it,
                                           const Gtk::TreeModel::Path &a_path)
    {
        using namespace variables_utils ;
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (a_it) ;

        if (a_path.get_depth ()) {}

        IDebugger::VariableSafePtr var =
            (IDebugger::VariableSafePtr)
                a_it->get_value (get_variable_columns ().variable) ;
        if (!var) {return;}
        Gtk::TreeModel::iterator child_it = a_it->children ().begin ();
        if (!child_it) {return;}
        var = child_it->get_value (get_variable_columns ().variable) ;
        if (var) {return;}

        cur_selected_row = a_it ;
        print_pointed_variable_value () ;

        NEMIVER_CATCH
    }

    void on_tree_view_row_activated_signal (const Gtk::TreeModel::Path &a_path,
                                            Gtk::TreeViewColumn *a_col)
    {
        using namespace variables_utils ;
        NEMIVER_TRY
        THROW_IF_FAIL (tree_store) ;
        Gtk::TreeModel::iterator it = tree_store->get_iter (a_path) ;
        UString type =
            (Glib::ustring) it->get_value (get_variable_columns ().type) ;
        if (type == "") {return;}

        if (a_col != tree_view->get_column (2)) {return;}
        cur_selected_row = it ;
        show_variable_type_in_dialog () ;
        NEMIVER_CATCH
    }


    void on_pointed_variable_value_signal
                                (const UString &a_var_name,
                                 const IDebugger::VariableSafePtr &a_var)
    {
        using namespace variables_utils ;
        LOG_FUNCTION_SCOPE_NORMAL_DD

        NEMIVER_TRY

        LOG_DD ("a_var_name: '" << a_var_name << "'") ;
        THROW_IF_FAIL (tree_store) ;

        Gtk::TreeModel::iterator start_row_it, row_it ;
        get_local_variables_row_iterator (start_row_it) ;
        bool ret = get_variable_iter_from_qname
                                        (a_var_name,
                                         start_row_it,
                                         row_it) ;
        if (!ret) {
            get_function_arguments_row_iterator (start_row_it) ;
            ret = get_variable_iter_from_qname (a_var_name, start_row_it, row_it);
        }
        THROW_IF_FAIL (ret) ;
        THROW_IF_FAIL (row_it) ;
        Gtk::TreeModel::iterator result_row_it ;
        append_a_variable (a_var, row_it, tree_store,
                           *tree_view, *debugger,
                           true, is_new_frame, result_row_it) ;
        debugger->print_variable_type (a_var_name) ;
        NEMIVER_CATCH
    }

};//end class LocalVarsInspector

LocalVarsInspector::LocalVarsInspector (IDebuggerSafePtr &a_debugger,
                                        IWorkbench &a_workbench)
{
    m_priv.reset (new Priv (a_debugger, a_workbench));
}

LocalVarsInspector::~LocalVarsInspector ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

Gtk::Widget&
LocalVarsInspector::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    return *m_priv->tree_view ;
}

void
LocalVarsInspector::set_local_variables
                        (const std::list<IDebugger::VariableSafePtr> &a_vars)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->debugger) ;

    m_priv->set_local_variables (a_vars) ;
}

void
LocalVarsInspector::show_local_variables_of_current_function ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->debugger) ;

    re_init_widget () ;
    m_priv->debugger->list_local_variables () ;
}

void
LocalVarsInspector::re_init_widget ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    THROW_IF_FAIL (m_priv) ;

    m_priv->re_init_tree_view () ;
}

NEMIVER_END_NAMESPACE (nemiver)
