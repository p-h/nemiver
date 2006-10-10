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
#include <glib/gi18n.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treerowreference.h>
#include "nmv-breakpoints-view.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"

namespace nemiver {

struct BPColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<int> id ;
    Gtk::TreeModelColumn<bool> enabled ;
    Gtk::TreeModelColumn<Glib::ustring> filename ;
    Gtk::TreeModelColumn<int> line ;

    BPColumns ()
    {
        add (id) ;
        add (enabled) ;
        add (filename) ;
        add (line) ;
    }
};//end Cols

static BPColumns&
get_bp_columns ()
{
    static BPColumns s_cols ;
    return s_cols ;
}

struct BreakpointsView::Priv {
public:
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::ListStore> list_store ;
    //std::map<UString, IDebugger::VariableSafePtr> local_vars_to_set ;
    //std::map<UString, IDebugger::VariableSafePtr> function_arguments_to_set ;

    Priv ()
    {
        build_tree_view () ;
        void set_breakpoints (const std::map<int, IDebugger::BreakPoint> &a_breakpoints);
    }

    void build_tree_view ()
    {
        if (tree_view) {return;}
        //create a default tree store and a tree view
        list_store = Gtk::ListStore::create (get_bp_columns ()) ;
        tree_view = new Gtk::TreeView (list_store) ;

        //create the columns of the tree view
        //tree_view->append_column ("", get_bp_columns ().enabled) ;
        tree_view->append_column (_("ID"), get_bp_columns ().id) ;
        tree_view->append_column (_("Filename"), get_bp_columns ().filename) ;
        tree_view->append_column (_("Line"), get_bp_columns ().line) ;
    }

    void set_breakpoints (const std::map<int, IDebugger::BreakPoint> &a_breakpoints)
    {
        THROW_IF_FAIL (list_store) ;
        list_store->clear();
        std::map<int, IDebugger::BreakPoint>::const_iterator break_iter;
        for (break_iter = a_breakpoints.begin (); break_iter != a_breakpoints.end ();
                ++break_iter)
        {
            Gtk::TreeModel::iterator tree_iter = list_store->append();
            (*tree_iter)[get_bp_columns ().id] = break_iter->first;
            (*tree_iter)[get_bp_columns ().enabled] = break_iter->second.enabled () ;
            (*tree_iter)[get_bp_columns ().filename] = break_iter->second.file_name () ;
            (*tree_iter)[get_bp_columns ().line] = break_iter->second.line () ;
        }
    }

};//end class BreakpointsView::Priv

BreakpointsView::BreakpointsView ()
{
    m_priv = new Priv ();
}

BreakpointsView::~BreakpointsView ()
{
}

Gtk::Widget&
BreakpointsView::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    THROW_IF_FAIL (m_priv->list_store) ;
    return *m_priv->tree_view ;
}

void
BreakpointsView::set_breakpoints
                        (const std::map<int, IDebugger::BreakPoint> &a_breakpoints)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->set_breakpoints (a_breakpoints) ;
}

}//end namespace nemiver

