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

namespace nemiver {

struct VariableColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> name ;
    Gtk::TreeModelColumn<Glib::ustring> type ;
    Gtk::TreeModelColumn<Glib::ustring> value ;
    Gtk::TreeModelColumn<IDebugger::VariableSafePtr> variable ;

    VariableColumns ()
    {
        add (name) ;
        add (type) ;
        add (value) ;
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
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_store ;
    Gtk::TreeRowReference *local_variables_row_ref ;
    Gtk::TreeRowReference *function_arguments_row_ref ;
    std::map<UString, IDebugger::VariableSafePtr> local_vars_to_set ;
    std::map<UString, IDebugger::VariableSafePtr> function_arguments_to_set ;

    Priv (IDebuggerSafePtr &a_debugger) :
        local_variables_row_ref (NULL),
        function_arguments_row_ref (NULL)
    {
        THROW_IF_FAIL (a_debugger) ;
        debugger = a_debugger ;
        build_tree_view () ;
        re_init_tree_view () ;
        connect_to_debugger_signals () ;
    }

    void build_tree_view ()
    {
        if (tree_view) {return;}
        //create a default tree store and a tree view
        tree_store = Gtk::TreeStore::create (get_variable_columns ()) ;
        tree_view = new Gtk::TreeView (tree_store) ;

        //create the columns of the tree view
        tree_view->append_column (_("variable"), get_variable_columns ().name) ;
        tree_view->append_column (_("value"), get_variable_columns ().value) ;
        tree_view->append_column (_("type"), get_variable_columns ().type) ;

    }

    void re_init_tree_view ()
    {
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

    void set_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                              const Gtk::TreeModel::iterator &a_parent,
                              Gtk::TreeModel::iterator &a_result)
    {
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
        LOG_FUNCTION_SCOPE_NORMAL_D (NMV_DEFAULT_DOMAIN) ;
        THROW_IF_FAIL (tree_store) ;
        THROW_IF_FAIL (function_arguments_row_ref) ;

        std::list<IDebugger::VariableSafePtr>::const_iterator it ;
        for (it = a_vars.begin () ; it != a_vars.end () ; ++it) {
            THROW_IF_FAIL ((*it)->name () != "") ;
            function_arguments_to_set[(*it)->name ()] = *it ;
            debugger->print_variable_value ((*it)->name ()) ;
        }

    }


    void on_local_variables_listed_signal
                                (const list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (NMV_DEFAULT_DOMAIN) ;
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
        LOG_FUNCTION_SCOPE_NORMAL_D (NMV_DEFAULT_DOMAIN) ;

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

    void connect_to_debugger_signals ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (NMV_DEFAULT_DOMAIN) ;

        THROW_IF_FAIL (debugger) ;
        debugger->local_variables_listed_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_local_variables_listed_signal)) ;
        debugger->stopped_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_stopped_signal)) ;
        debugger->variable_value_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_variable_value_signal)) ;
        debugger->frames_params_listed_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_frames_params_listed_signal)) ;
    }

};//end class VarsEditor

VarsEditor::VarsEditor (IDebuggerSafePtr &a_debugger)
{
    m_priv = new Priv (a_debugger);
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

}//end namespace nemiver

