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
#include "nmv-vars-editor.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"
#include "nmv-i-workbench.h"

NEMIVER_BEGIN_NAMESPACE (nemiver) ;

struct VariableColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> name ;
    Gtk::TreeModelColumn<Glib::ustring> value ;
    Gtk::TreeModelColumn<Glib::ustring> type ;
    Gtk::TreeModelColumn<Glib::ustring> type_caption ;
    Gtk::TreeModelColumn<IDebugger::VariableSafePtr> variable ;

    VariableColumns ()
    {
        add (name) ;
        add (value) ;
        add (type) ;
        add (type_caption) ;
        add (variable) ;
    }
};//end Cols

static VariableColumns&
get_variable_columns ()
{
    static VariableColumns s_cols ;
    return s_cols ;
}

struct VarsEditor::Priv {
private:
    Priv ();
public:
    IDebuggerSafePtr debugger ;
    IWorkbench &workbench ;
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_store ;
    Gtk::TreeModel::iterator cur_selected_row ;
    Gtk::TreeRowReference *local_variables_row_ref ;
    Gtk::TreeRowReference *function_arguments_row_ref ;
    std::map<UString, IDebugger::VariableSafePtr> local_vars_to_set ;
    std::map<UString, IDebugger::VariableSafePtr> function_arguments_to_set ;
    SafePtr<Gtk::Menu> contextual_menu ;
    Gtk::MenuItem *dereference_mi ;

    Priv (IDebuggerSafePtr &a_debugger,
          IWorkbench &a_workbench) :
        workbench (a_workbench),
        local_variables_row_ref (0),
        function_arguments_row_ref (0),
        dereference_mi (0)
    {
        THROW_IF_FAIL (a_debugger) ;
        debugger = a_debugger ;
        build_tree_view () ;
        re_init_tree_view () ;
        connect_to_debugger_signals () ;
        init_graphical_signals () ;
    }

    void build_tree_view ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (tree_view) {return;}
        //create a default tree store and a tree view
        tree_store = Gtk::TreeStore::create (get_variable_columns ()) ;
        tree_view = new Gtk::TreeView (tree_store) ;
        tree_view->set_headers_clickable (true) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        sel->set_mode (Gtk::SELECTION_SINGLE) ;

        //create the columns of the tree view
        tree_view->append_column (_("variable"), get_variable_columns ().name) ;
        Gtk::TreeViewColumn * col = tree_view->get_column (0) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        tree_view->append_column (_("value"), get_variable_columns ().value) ;
        col = tree_view->get_column (1) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;

        tree_view->append_column (_("type"),
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
        //Store row refs on both rows
        //****************************************************
        Gtk::TreeModel::iterator it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[get_variable_columns ().name] = _("local variables");
        local_variables_row_ref =
            new Gtk::TreeRowReference (tree_store, tree_store->get_path (it)) ;
        THROW_IF_FAIL (local_variables_row_ref) ;

        it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[get_variable_columns ().name] = _("function arguments");
        function_arguments_row_ref =
            new Gtk::TreeRowReference (tree_store, tree_store->get_path (it)) ;
        THROW_IF_FAIL (function_arguments_row_ref) ;
    }

    bool set_variable_type_walker (const Gtk::TreeModel::iterator a_it,
                                   const UString &a_var,
                                   const UString &a_type)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (!a_it) {return false;}
        guint nb_lines = 0 ;
        UString type_caption,tmp_str ;
        tmp_str = (Glib::ustring) (*a_it)[get_variable_columns ().name]  ;
        LOG_DD ("scanning row of name '"
                << tmp_str << "'") ;
        if (tmp_str == a_var) {
            LOG_DD ("found variable '" << a_var << "', of type '"
                    << a_type << "'") ;
            (*a_it)[get_variable_columns ().type] =  a_type ;
            nb_lines = a_type.get_number_of_lines () ;
            type_caption = a_type ;
            if (nb_lines) {--nb_lines;}
            if (nb_lines) {
                UString::size_type i = a_type.find ('\n') ;
                type_caption.erase (i) ;
                type_caption += "..." ;
            }
            (*a_it)[get_variable_columns ().type_caption] =  type_caption ;
            return true;
        }
        return false ;
    }

    void set_a_variable_type (const UString &a_var, const UString &a_type)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;
        tree_store->foreach_iter (sigc::bind
                (sigc::mem_fun (*this, &Priv::set_variable_type_walker),
                 a_var, a_type)) ;
    }

    void set_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                              const Gtk::TreeModel::iterator &a_parent,
                              Gtk::TreeModel::iterator &a_result)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (a_var) ;
        THROW_IF_FAIL (a_parent) ;

        Gtk::TreeModel::iterator cur_row_it =
                            tree_store->append (a_parent->children ()) ;
        THROW_IF_FAIL (cur_row_it) ;

        (*cur_row_it)[get_variable_columns ().variable] = a_var ;
        (*cur_row_it)[get_variable_columns ().name] = a_var->name () ;
        if (a_var->value () != "") {
            (*cur_row_it)[get_variable_columns ().value] = a_var->value () ;
        }
        (*cur_row_it)[get_variable_columns ().type] = a_var->type () ;
        a_result = cur_row_it ;
    }

    void set_a_variable (const IDebugger::VariableSafePtr &a_var,
                         const Gtk::TreeModel::iterator &a_parent,
                         Gtk::TreeModel::iterator &a_result)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (a_result) {}
        Gtk::TreeModel::iterator parent_iter, tmp_iter;

        set_a_variable_real (a_var, a_parent, parent_iter);

        if (a_var->members ().empty ()) {return;}

        std::list<IDebugger::VariableSafePtr>::const_iterator member_iter ;
        for (member_iter = a_var->members ().begin ();
             member_iter != a_var->members ().end ();
             ++member_iter) {
            set_a_variable (*member_iter, parent_iter, tmp_iter) ;
        }
    }

    void set_variables_real (const std::list<IDebugger::VariableSafePtr> &a_vars,
                             Gtk::TreeRowReference &a_vars_row_ref)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        Gtk::TreeModel::iterator vars_row_it =
            tree_store->get_iter (a_vars_row_ref.get_path ()) ;
        Gtk::TreeModel::iterator tmp_iter ;

        THROW_IF_FAIL (vars_row_it) ;
        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            if (!(*it)) {continue ;}
            set_a_variable (*it, vars_row_it, tmp_iter) ;
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
            debugger->print_variable_type ((*it)->name ()) ;
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

    bool is_type_a_pointer (const UString &a_type)
    {
        int i = a_type.size ();
        if (i) {--i;}
        while (i > 0 && isspace (a_type[i])) {--i;}
        if (a_type[i] == '*') {
            return true ;
        }
        return false ;
    }

    Gtk::Menu& get_contextual_menu ()
    {
        if (!contextual_menu) {
            contextual_menu.reset ( new Gtk::Menu) ;
            Gtk::MenuItem *mi =
                Gtk::manage (new Gtk::MenuItem (_("show variable type"))) ;
            mi->signal_activate ().connect (sigc::mem_fun
                (*this, &Priv::on_show_variable_type_menu_item)) ;
            contextual_menu->append (*mi) ;
            mi = Gtk::manage (new Gtk::MenuItem (_("Dereference"))) ;
            mi->signal_activate ().connect (sigc::mem_fun
                (*this, &Priv::on_dereference_variable_menu_item)) ;
            contextual_menu->append (*mi) ;
            dereference_mi = mi;
            dereference_mi->set_sensitive (false) ;
        }
        THROW_IF_FAIL (contextual_menu) ;
        if (cur_selected_row) {
            THROW_IF_FAIL (dereference_mi) ;
            UString type =
            (Glib::ustring) (*cur_selected_row)[get_variable_columns ().type] ;
            if (is_type_a_pointer (type)) {
                dereference_mi->set_sensitive (true) ;
            } else {
                dereference_mi->set_sensitive (false) ;
            }
        }
        return *contextual_menu ;
    }

    void popup_contextual_menu (GdkEventButton *a_event)
    {
        get_contextual_menu ().show_all () ;
        get_contextual_menu ().popup (a_event->button, a_event->time) ;
    }

    void show_variable_type_in_dialog ()
    {
        if (!cur_selected_row) {return;}
        UString type =
            (Glib::ustring) (*cur_selected_row)[get_variable_columns ().type] ;
        UString message ;
        message.printf (_("Variable type is: \n %s"), type.c_str ()) ;
        ui_utils::display_info (message) ;
    }

    void print_pointed_variable_value ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        if (!cur_selected_row) {return;}
        UString name =
            (Glib::ustring) (*cur_selected_row)[get_variable_columns ().name] ;
        THROW_IF_FAIL (name!= "") ;
        debugger->print_pointed_variable_value (name) ;
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


    void on_stopped_signal (const UString &a_str,
                            bool a_has_frame,
                            const IDebugger::Frame &a_frame)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (NMV_DEFAULT_DOMAIN) ;
        if (a_str == "" || a_frame.line ()) {}

        NEMIVER_TRY

        THROW_IF_FAIL (debugger) ;
        if (a_has_frame) {
            THROW_IF_FAIL (tree_store) ;
            re_init_tree_view () ;
            debugger->list_local_variables () ;
        }

        NEMIVER_CATCH
    }

    void on_variable_value_signal (const UString &a_var_name,
                                   const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (tree_store) ;

        std::map<UString, IDebugger::VariableSafePtr>::const_iterator it, nil ;
        std::map<UString, IDebugger::VariableSafePtr> *map_ptr = NULL ;
        Gtk::TreeRowReference *row_ref = NULL ;

        it = local_vars_to_set.find (a_var_name) ;
        nil = local_vars_to_set.end () ;
        row_ref = local_variables_row_ref ;
        map_ptr = &local_vars_to_set ;

        if (it == nil) {
            it = function_arguments_to_set.find (a_var_name) ;
            nil = function_arguments_to_set.end () ;
            row_ref = function_arguments_row_ref ;
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
        set_a_variable (a_var,
                        tree_store->get_iter (row_ref->get_path ()),
                        row_it) ;

        map_ptr->erase (a_var_name) ;
        tree_view->expand_row (row_ref->get_path (), false) ;

        NEMIVER_CATCH
    }

    void on_variable_type_signal (const UString &a_var_name,
                                  const UString &a_type)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        LOG_DD ("var: '" << a_var_name << "', type: '" << a_type << "'") ;
        set_a_variable_type (a_var_name, a_type) ;

        NEMIVER_CATCH
    }

    void on_tree_view_selection_changed_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        THROW_IF_FAIL (tree_view) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        cur_selected_row = sel->get_selected () ;

        NEMIVER_CATCH
    }

    void on_button_press_event_signal (GdkEventButton *a_event)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        if (!cur_selected_row) {return ;}
        THROW_IF_FAIL (a_event) ;
        if (a_event->button != 3) {return ;}
        popup_contextual_menu (a_event) ;

        NEMIVER_CATCH
    }

    void on_show_variable_type_menu_item ()
    {
        NEMIVER_TRY

        show_variable_type_in_dialog () ;

        NEMIVER_CATCH
    }

    void on_dereference_variable_menu_item ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        print_pointed_variable_value () ;

        NEMIVER_CATCH
    }

    bool add_dereferenced_pointer_walker (const Gtk::TreeModel::iterator &a_it,
                                          const UString &a_var_name,
                                          const IDebugger::VariableSafePtr &a_var)
    {
        NEMIVER_TRY

        THROW_IF_FAIL (a_it) ;
        UString name = (Glib::ustring) (*a_it)[get_variable_columns ().name] ;

        if (name != a_var_name) {return false;}

        Gtk::TreeModel::iterator it ;
        set_a_variable (a_var, a_it, it) ;
        debugger->print_variable_type (a_var->name ()) ;

        NEMIVER_CATCH
        return true ;
    }

    void on_pointed_variable_value_signal
                                (const UString &a_var_name,
                                 const IDebugger::VariableSafePtr &a_value)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD
        if (a_var_name == "" || a_value) {}
        THROW_IF_FAIL (tree_store) ;
        tree_store->foreach_iter (sigc::bind
                (sigc::mem_fun (*this, &Priv::add_dereferenced_pointer_walker),
                 a_var_name, a_value)) ;
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
        tree_view->signal_button_press_event ().connect_notify (sigc::mem_fun
                (*this, &Priv::on_button_press_event_signal)) ;
    }

};//end class VarsEditor

VarsEditor::VarsEditor (IDebuggerSafePtr &a_debugger,
                        IWorkbench &a_workbench)
{
    m_priv = new Priv (a_debugger, a_workbench);
}

VarsEditor::~VarsEditor ()
{
}

Gtk::Widget&
VarsEditor::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    return *m_priv->tree_view ;
}

void
VarsEditor::set_local_variables
                        (const std::list<IDebugger::VariableSafePtr> &a_vars)
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->debugger) ;

    m_priv->set_local_variables (a_vars) ;
}

NEMIVER_END_NAMESPACE (nemiver)
