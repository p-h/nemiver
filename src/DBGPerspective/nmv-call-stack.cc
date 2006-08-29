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
#include <gtkmm/liststore.h>
#include "nmv-call-stack.h"

namespace nemiver {

struct GObjectMMRef {

    void operator () (Glib::Object *a_obj)
    {
        if (a_obj) {
            a_obj->reference () ;
        }
    }
};//end GlibObjectRef

struct GObjectMMUnref {
    void operator () (Glib::Object *a_obj)
    {
        if (a_obj) {
            a_obj->unreference () ;
        }
    }
};//end GlibObjectRef

typedef SafePtr<Glib::Object, GObjectMMRef, GObjectMMUnref> GObjectMMSafePtr ;

struct CallStackCols : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> location ;
    Gtk::TreeModelColumn<Glib::ustring> function_name ;

    enum Index {
        LOCATION=0,
        FUNCTION_NAME
    };

    CallStackCols ()
    {
        add (location) ;
        add (function_name) ;
    }
};//end cols

static CallStackCols&
columns ()
{
    static CallStackCols s_cols ;
    return s_cols ;
}

struct CallStack::Priv {
    IDebuggerSafePtr debugger ;
    vector<IDebugger::Frame> frames ;
    Glib::RefPtr<Gtk::ListStore> store ;
    GObjectMMSafePtr widget ;
    bool args_collection_started ;
    sigc::signal<void, int, const IDebugger::Frame&> frame_selected_signal ;

    Priv (IDebuggerSafePtr a_dbg) :
        debugger (a_dbg),
        args_collection_started (false)
    {
        connect_debugger_signals () ;
    }

    void on_debugger_stopped_signal (const UString &a_reason,
                                     bool a_has_frame,
                                     const IDebugger::Frame &a_frame)
    {
        THROW_IF_FAIL (debugger) ;
        debugger->list_frames () ;
    }

    void on_frames_listed_signal (const vector<IDebugger::Frame> &a_stack)
    {
        THROW_IF_FAIL (debugger) ;
        debugger->list_frames_arguments () ;
        set_frame_list (a_stack) ;
    }

    void on_frames_params_listed_signal
            (const map<int, vector<IDebugger::FrameParameter> > &a_frames_params)
    {
        LOG_D ("frames params listed", NMV_DEFAULT_DOMAIN) ;
    }

    void connect_debugger_signals ()
    {
        THROW_IF_FAIL (debugger) ;

        THROW_IF_FAIL (debugger) ;

        debugger->stopped_signal ().connect (sigc::mem_fun
                    (*this, &CallStack::Priv::on_debugger_stopped_signal)) ;
        debugger->frames_listed_signal ().connect (sigc::mem_fun
                    (*this, &CallStack::Priv::on_frames_listed_signal)) ;
        debugger->frames_params_listed_signal ().connect (sigc::mem_fun
                    (*this, &CallStack::Priv::on_frames_params_listed_signal)) ;
    }

    Gtk::Widget* get_widget ()
    {
        if (!widget) {return NULL;}
        return widget.do_dynamic_cast<Gtk::Widget> ().get () ;
    }

    void build_widget ()
    {
        if (widget) {
            return ;
        }
        store = Gtk::ListStore::create (columns ());
        Gtk::TreeView *tree_view = new Gtk::TreeView (store) ;
        THROW_IF_FAIL (tree_view) ;
        widget = tree_view ;
        tree_view->append_column ("line", columns ().location) ;
        tree_view->append_column ("function", columns ().function_name) ;
        tree_view->set_headers_visible (true) ;
        Gtk::TreeViewColumn* column =
                            tree_view->get_column (CallStackCols::FUNCTION_NAME) ;
        THROW_IF_FAIL (column) ;
        column->set_clickable (false) ;
        column->set_reorderable (false) ;

        THROW_IF_FAIL (column = tree_view->get_column (CallStackCols::LOCATION)) ;
        column->set_clickable (false) ;
        column->set_reorderable (false) ;
    }

    void set_frame_list (const vector<IDebugger::Frame> &a_list)
    {
        THROW_IF_FAIL (get_widget ()) ;

        clear_frame_list () ;

        Gtk::TreeModel::iterator store_iter ;
        vector<IDebugger::Frame>::const_iterator frame_iter ;
        for (frame_iter = a_list.begin () ;
             frame_iter != a_list.end () ;
             ++frame_iter) {
            store_iter = store->append () ;
            (*store_iter)[columns ().function_name] = frame_iter->function () ;
            (*store_iter)[columns ().location] =
                            frame_iter->file_name ()
                                + ":"
                                + UString::from_int (frame_iter->line ()) ;
        }
    }

    void clear_frame_list ()
    {
        THROW_IF_FAIL (store) ;
        store->clear () ;
    }
};//end struct CallStack::Priv

CallStack::CallStack (IDebuggerSafePtr &a_debugger)
{
    THROW_IF_FAIL (a_debugger) ;
    m_priv = new Priv (a_debugger) ;
}

CallStack::~CallStack ()
{
    m_priv = NULL ;
}

bool
CallStack::is_empty ()
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frames.empty () ;
}


const vector<IDebugger::Frame>&
CallStack::frames () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frames ;
}

Gtk::Widget&
CallStack::widget () const
{
    THROW_IF_FAIL (m_priv) ;

    if (!m_priv->get_widget ()) {
        m_priv->build_widget () ;
        THROW_IF_FAIL (m_priv->widget) ;
    }
    return *m_priv->get_widget () ;
}

void
CallStack::update_stack ()
{
    //TODO: code this once the widget and the IDebugger engine are ready
}

sigc::signal<void, int, const IDebugger::Frame&>&
CallStack::frame_selected_signal () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frame_selected_signal ;
}
}//end namespace nemiver


