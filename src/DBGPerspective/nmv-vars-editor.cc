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
#include <glib/gi18n.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treerowreference.h>
#include "nmv-vars-editor.h"
#include "nmv-exception.h"

namespace nemiver {

struct VariableColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> name ;
    Gtk::TreeModelColumn<Glib::ustring> type ;
    Gtk::TreeModelColumn<Glib::ustring> value ;

    VariableColumns ()
    {
        add (name) ;
        add (type) ;
        add (value) ;
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
    SafePtr<Gtk::TreeRowReference> local_variables_row_ref ;
    SafePtr<Gtk::TreeRowReference> global_variables_row_ref ;

    Priv (IDebuggerSafePtr &a_debugger)
    {
        THROW_IF_FAIL (a_debugger) ;
        debugger = a_debugger ;
    }

    void build_tree_view ()
    {
        if (tree_view) {return;}
        //create a default tree store and a tree view
        tree_store = Gtk::TreeStore::create (get_variable_columns ()) ;
        tree_view = new Gtk::TreeView (tree_store) ;

        //create the columns of the tree view
        tree_view->append_column (_("variable"), get_variable_columns ().name) ;
        tree_view->append_column (_("type"), get_variable_columns ().type) ;
        tree_view->append_column (_("value"), get_variable_columns ().value) ;

        //****************************************************
        //add two rows: local variables and global variable.
        //Store row refs on both rows
        //****************************************************
        Gtk::TreeModel::iterator it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[get_variable_columns ().name] = _("local variables");
        local_variables_row_ref =
            new Gtk::TreeRowReference (tree_store, tree_store->get_path (it)) ;

        it = tree_store->append () ;
        THROW_IF_FAIL (it) ;
        (*it)[get_variable_columns ().name] = _("global variables");
        global_variables_row_ref =
            new Gtk::TreeRowReference (tree_store, tree_store->get_path (it)) ;
        THROW_IF_FAIL (global_variables_row_ref) ;
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
VarsEditor::get_widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    return *m_priv->tree_view ;
}

void
VarsEditor::set_local_variables (std::list<IDebugger::VariableSafePtr> &a_vars)
{
    THROW_IF_FAIL (m_priv) ;
    THROW (_("not implemented yet")) ;
}

void
VarsEditor::set_global_variables (std::list<IDebugger::VariableSafePtr> &a_vars)
{
    THROW_IF_FAIL (m_priv) ;
    THROW (_("not implemented yet")) ;
}

}//end namespace nemiver

