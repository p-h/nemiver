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
#ifndef __NMV_THREAD_LIST_H__
#define __NMV_THREAD_LIST_H__

#include <list>
#include <gtkmm/widget.h>
#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"

using nemiver::common::Object;
using nemiver::common::SafePtr;
using nemiver::IDebuggerSafePtr;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class NEMIVER_API ThreadList : public Object {
    //non copyable
    ThreadList (const ThreadList &);
    ThreadList& operator=  (const ThreadList &);

    struct Priv;
    SafePtr<Priv> m_priv;

protected:
    ThreadList ();

public:

    ThreadList (IDebuggerSafePtr &);
    virtual ~ThreadList ();
    const std::list<int>& thread_ids () const;
    int current_thread_id () const;
    Gtk::Widget& widget () const;
    void clear ();
    sigc::signal<void, int>& thread_selected_signal () const;
};//end class ThreadList

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_THREAD_LIST_H__
