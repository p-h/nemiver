//Author: Jonathon Jongsma
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
#ifndef __NMV_REGISTERS_VIEW_H__
#define __NMV_REGISTERS_VIEW_H__

#include <list>
#include <gtkmm/widget.h>
#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class IWorkbench;
class IPerspective;

class NEMIVER_API RegistersView : public nemiver::common::Object {
    //non copyable
    RegistersView (const RegistersView&);
    RegistersView& operator= (const RegistersView&);

    struct Priv;
    SafePtr<Priv> m_priv;

public:

    RegistersView (IDebuggerSafePtr& a_debugger);
    virtual ~RegistersView ();
    Gtk::Widget& widget () const;
    void clear ();

};//end RegistersView

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_REGISTERS_VIEW_H__


