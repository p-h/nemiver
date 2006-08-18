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

struct CallStack::Priv {
    IDebuggerSafePtr debugger ;
    list<IDebugger::Frame> frames ;
    GObjectMMSafePtr widget ;
    sigc::signal<void, int, const IDebugger::Frame&> frame_selected_signal ;

    Priv (IDebuggerSafePtr a_dbg) :
        debugger (a_dbg)
    {}
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

void
CallStack::push_frame (const IDebugger::Frame &a_frame)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->frames.push_front (a_frame) ;
}

bool
CallStack::is_empty ()
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frames.empty () ;
}

IDebugger::Frame&
CallStack::peek_frame () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (!m_priv->frames.empty ()) ;
    return m_priv->frames.front () ;
}

IDebugger::Frame
CallStack::pop_frame ()
{
    THROW_IF_FAIL (m_priv) ;
    IDebugger::Frame frame = m_priv->frames.front () ;
    m_priv->frames.pop_front () ;
    return frame ;
}

list<IDebugger::Frame>&
CallStack::frames () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frames ;
}

Gtk::Widget&
CallStack::widget () const
{
    THROW_IF_FAIL (m_priv && m_priv->widget) ;
    Gtk::Widget* widget = dynamic_cast<Gtk::Widget*> (m_priv->widget.get ()) ;
    THROW_IF_FAIL (widget) ;
    return *widget ;
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

//TODO: write the call stack widget code !

}//end namespace nemiver


