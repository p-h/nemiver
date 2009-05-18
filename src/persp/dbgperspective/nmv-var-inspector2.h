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
#ifndef __NMV_VAR_INSPECTOR2_H__
#define __NMV_VAR_INSPECTOR2_H__

#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"

#ifdef WITH_VAROBJS

namespace Gtk {
    class Widget;
}

NEMIVER_BEGIN_NAMESPACE (nemiver)

namespace common {
    class UString;
}

class VarInspector2 : public nemiver::common::Object {
    VarInspector2 (const VarInspector2 &);
    VarInspector2& operator= (const VarInspector2 &);
    VarInspector2 ();
    class Priv;
    SafePtr<Priv> m_priv;

public:
    VarInspector2 (IDebuggerSafePtr a_debugger);
    virtual ~VarInspector2 ();
    Gtk::Widget& widget () const;
    void set_variable (IDebugger::VariableSafePtr a_variable,
                       bool a_expand = false);
    void inspect_variable (const UString &a_variable_name,
                           bool a_expand = false);
    IDebugger::VariableSafePtr get_variable () const;
    void clear ();
};//end class VarInspector2

NEMIVER_END_NAMESPACE (nemiver)

#endif //WITH_VAROBJS

#endif //__NMV_VAR_INSPECTOR2_H__

