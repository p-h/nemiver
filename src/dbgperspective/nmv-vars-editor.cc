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
    SafePtr<Gtk::TreeRowReference> local_variables_row_ref ;
    SafePtr<Gtk::TreeRowReference> function_arguments_row_ref ;
    std::map<UString, IDebugger::VariableSafePtr> local_vars_to_set ;
    std::map<UString, IDebugger::VariableSafePtr> function_arguments_to_set ;
    SafePtr<Gtk::Menu> contextual_menu ;
    Gtk::MenuItem *dereference_mi ;

    Priv (IDebuggerSafePtr &a_debugger,
          IWorkbench &a_workbench) :
        workbench (a_workbench),
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

    void set_a_variable_type (const UString &a_var_name,
                              const UString &a_type,
                              Gtk::TreeModel::iterator &a_it)
    {
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
        THROW_IF_FAIL (ret) ;
        THROW_IF_FAIL (row_it) ;
        row_it->set_value (get_variable_columns ().type, (Glib::ustring)a_type) ;
        int nb_lines = a_type.get_number_of_lines () ;
        UString type_caption = a_type ;
        if (nb_lines) {--nb_lines;}
        if (nb_lines) {
            UString::size_type i = a_type.find ('\n') ;
            type_caption.erase (i) ;
            type_caption += "..." ;
        }
        (*row_it)[get_variable_columns ().type_caption] =  type_caption ;
        IDebugger::VariableSafePtr variable =
        (IDebugger::VariableSafePtr)(*row_it)[get_variable_columns ().variable];
        THROW_IF_FAIL (variable) ;
        variable->type (type_caption) ;
        a_it = row_it ;
    }

    void set_a_variable_type (const UString &a_var_name, const UString &a_type)
    {
        Gtk::TreeModel::iterator it ;
        set_a_variable_type (a_var_name, a_type, it) ;
    }

    void set_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                              const Gtk::TreeModel::iterator &a_parent,
                              Gtk::TreeModel::iterator &a_result)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (a_var) ;
        THROW_IF_FAIL (a_parent) ;

        Gtk::TreeModel::iterator cur_row_it  = a_parent->children ().begin () ;
        if (!cur_row_it
            ||(Glib::ustring)
                cur_row_it->get_value (get_variable_columns ().name) != "") {
            cur_row_it = tree_store->append (a_parent->children ()) ;
        }
        THROW_IF_FAIL (cur_row_it) ;

        (*cur_row_it)[get_variable_columns ().variable] = a_var ;
        UString var_name = a_var->name ();
        var_name.chomp () ;
        (*cur_row_it)[get_variable_columns ().name] = var_name ;
        if (a_var->value () != "") {
            (*cur_row_it)[get_variable_columns ().value] = a_var->value () ;
        }
        (*cur_row_it)[get_variable_columns ().type] = a_var->type () ;

        UString qname ;
        a_var->build_qname (qname) ;
        LOG_DD ("set variable with qname '" << qname << "'") ;
        debugger->print_variable_type (qname) ;
        LOG_DD ("did query type of variable '" << qname << "'") ;
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


    void show_variable_type_in_dialog ()
    {
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
                                            (get_variable_columns ().variable) ;
        THROW_IF_FAIL (variable) ;
        UString qname ;
        variable->build_qname (qname) ;
        debugger->print_pointed_variable_value (qname) ;
    }

    bool break_qname_into_name_elements (const UString &a_qname,
                                         std::list<UString> &a_name_elems)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD

        THROW_IF_FAIL (a_qname != "") ;

        UString::size_type len=a_qname.size (),cur=0, name_start=0, name_end=0 ;
        UString name_element ;
        std::list<UString> name_elems ;

fetch_element:
        name_start = cur ;
        name_element="" ;
        for (;
             cur < len && (isalnum (a_qname[cur])
                           || a_qname[cur] == '_'
                           || isspace (a_qname[cur]))
             ; ++cur) {
        }
        if (cur == name_start) {
            name_element = "";
        } else if (a_qname[cur] == '.'
                   || (cur + 1 < len
                       && a_qname[cur] == '-'
                       && a_qname[cur+1] == '>')
                   || cur >= len){
            name_end = cur - 1 ;
            name_element = a_qname.substr (name_start, name_end - name_start + 1);
            if (a_qname[cur] == '-') {
                name_element = '*' + name_element ;
                cur += 2 ;
            } else if (cur < len){
                ++cur ;
            }
        } else {
            LOG_ERROR ("failed to parse name element. Index was: "
                       << (int) cur << " buf was '" << a_qname << "'") ;
            return false ;
        }
        name_element.chomp () ;
        LOG_DD ("got name element: '" << name_element << "'") ;
        name_elems.push_back (name_element) ;
        if (cur < len) {
            LOG_DD ("go fetch next name element") ;
            goto fetch_element ;
        }
        LOG_DD ("getting out") ;
        a_name_elems = name_elems ;
        return true ;
    }

    bool get_variable_iter_from_qname
                        (const std::list<UString> &a_name_elems,
                         const std::list<UString>::const_iterator &a_cur_elem_it,
                         const Gtk::TreeModel::iterator &a_from_it,
                         Gtk::TreeModel::iterator &a_result)
    {
        THROW_IF_FAIL (!a_name_elems.empty ()) ;
        if (!a_from_it) {
            LOG_ERROR ("got null a_from iterator") ;
            return false ;
        }
        std::list<UString>::const_iterator cur_elem_it = a_cur_elem_it;
        if (cur_elem_it == a_name_elems.end ()) {
            a_result = a_from_it ;
            return true;
        }

        Gtk::TreeModel::const_iterator row_it ;
        for (row_it = a_from_it->children ().begin () ;
             row_it != a_from_it->children ().end () ;
             ++row_it) {
            if ((Glib::ustring)((*row_it)[get_variable_columns ().name])
                  == *cur_elem_it) {
                LOG_DD ("walked to path element: '" << *cur_elem_it) ;
                return get_variable_iter_from_qname (a_name_elems,
                                                     ++cur_elem_it,
                                                     row_it,
                                                     a_result) ;
            } else if (row_it->children () && row_it->children ().begin ()) {
                Gtk::TreeModel::iterator it = row_it->children ().begin () ;
                if ((Glib::ustring) (*it)[get_variable_columns ().name]
                    == *cur_elem_it) {
                    LOG_DD ("walked to path element: '" << *cur_elem_it) ;
                    return get_variable_iter_from_qname (a_name_elems,
                                                         ++cur_elem_it,
                                                         it,
                                                         a_result) ;
                }
            }
        }
        return false ;
    }

    bool get_variable_iter_from_qname (const UString &a_var_qname,
                                       const Gtk::TreeModel::iterator &a_from,
                                       Gtk::TreeModel::iterator &a_result)
    {
        THROW_IF_FAIL (a_var_qname != "") ;

        if (!a_from) {
            LOG_ERROR ("got null a_from iterator") ;
            return false ;
        }
        std::list<UString> name_elems ;
        bool is_ok = break_qname_into_name_elements (a_var_qname, name_elems) ;
        if (!is_ok) {
            LOG_ERROR ("failed to break qname into path elements") ;
            return false ;
        }

        bool ret = get_variable_iter_from_qname (name_elems,
                                                 name_elems.begin (),
                                                 a_from,
                                                 a_result) ;
        return ret ;
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


    void on_stopped_signal (const UString &a_str,
                            bool a_has_frame,
                            const IDebugger::Frame &a_frame,
                            int a_thread_id)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (NMV_DEFAULT_DOMAIN) ;
        if (a_str == "" || a_frame.line () || a_thread_id) {}

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

        Gtk::TreeModel::iterator row_it ;
        set_a_variable_type (a_var_name, a_type, row_it) ;
        if (!row_it) {return;}

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
        LOG_FUNCTION_SCOPE_NORMAL_DD

        NEMIVER_TRY
        THROW_IF_FAIL (tree_store) ;

        Gtk::TreeModel::iterator row_it ;
        bool ret = get_variable_iter_from_qname
                                        (a_var_name,
                                         tree_store->children ().begin (),
                                         row_it) ;
        THROW_IF_FAIL (ret) ;
        THROW_IF_FAIL (row_it) ;
        Gtk::TreeModel::iterator result_row_it ;
        set_a_variable (a_var, row_it, result_row_it) ;
        debugger->print_variable_type (a_var_name) ;
        NEMIVER_CATCH
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

void
VarsEditor::show_local_variables_of_current_function ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->debugger) ;

    m_priv->re_init_tree_view () ;
    m_priv->debugger->list_local_variables () ;
}

NEMIVER_END_NAMESPACE (nemiver)
