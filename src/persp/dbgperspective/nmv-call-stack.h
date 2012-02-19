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

#ifndef __NMV_CALL_STACK_H__
#define __NMV_CALL_STACK_H__

#include <list>
#include <gtkmm/widget.h>
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-object.h"
#include "nmv-i-debugger.h"

using namespace std;
using nemiver::common::SafePtr;
using nemiver::common::Object;
using nemiver::IDebugger;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class IWorkbench;
class IPerspective;

class NEMIVER_API CallStack : public Object {
    //non copyable

    CallStack (const CallStack &);
    CallStack& operator= (const CallStack &);

    struct Priv;
    SafePtr<Priv> m_priv;

protected:
    CallStack ();

public:

    CallStack (IDebuggerSafePtr &a_debugger, IWorkbench& a_workbench,
            IPerspective& a_perspective);
    virtual ~CallStack ();
    bool is_empty ();
    const vector<IDebugger::Frame>& frames () const;
    IDebugger::Frame& current_frame () const;
    void update_stack (bool select_top_most = false);
    void clear ();
    Gtk::Widget& widget () const;
    sigc::signal<void,
                 int,
                 const IDebugger::Frame&>& frame_selected_signal () const;
};//end class CallStack

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_CALL_STACK_H__

