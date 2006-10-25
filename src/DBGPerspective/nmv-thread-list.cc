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

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include "nmv-thread-list.h"
#include "nmv-exception.h"
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct ThreadListColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<int> thread_id ;

    ThreadListColumns ()
    {
        add (thread_id) ;
    }
};//end class ThreadListColumns

static ThreadListColumns&
columns ()
{
    static ThreadListColumns s_thread_list_columns ;
    return s_thread_list_columns ;
}

struct ThreadList::Priv {
    IDebuggerSafePtr debugger ;
    std::list<int> thread_ids ;
    int current_thread ;
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_store ;
    sigc::signal<void, int> thread_selected_signal ;

    Priv (IDebuggerSafePtr &a_debugger) :
        debugger (a_debugger),
        current_thread (0)
    {
    }

    void build_widget ()
    {
        tree_store = Gtk::TreeStore::create (columns ()) ;
        tree_view = new Gtk::TreeView () ;
        tree_view->set_model (tree_store) ;
        int col_num =
            tree_view->append_column ("Thread ID", columns ().thread_id) ;
        Gtk::TreeViewColumn *column = tree_view->get_column (col_num) ;
        THROW_IF_FAIL (column) ;
        column->set_clickable (false) ;
        column->set_reorderable (false) ;
    }

    void set_a_thread_id (int a_id)
    {
        THROW_IF_FAIL (tree_store) ;
        Gtk::TreeModel::iterator iter = tree_store->append () ;
        iter->set_value (columns ().thread_id, a_id) ;
    }

    void set_thread_id_list (const std::list<int> &a_list)
    {
        std::list<int>::const_iterator it ;
        for (it = a_list.begin () ; it != a_list.end () ; ++it) {
            set_a_thread_id (*it) ;
        }
    }
    //TODO: connect to signals and make this live !
};//end ThreadList::Priv

ThreadList::ThreadList (IDebuggerSafePtr &a_debugger)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv = new ThreadList::Priv (a_debugger);
}

ThreadList::~ThreadList ()
{
}

const list<int>&
ThreadList::thread_ids () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->thread_ids ;
}

int
ThreadList::current_thread_id () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->current_thread ;
}

Gtk::Widget&
ThreadList::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    return *m_priv->tree_view ;
}

sigc::signal<void, int>&
ThreadList::thread_selected_signal () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->thread_selected_signal ;
}

NEMIVER_END_NAMESPACE (nemiver)
