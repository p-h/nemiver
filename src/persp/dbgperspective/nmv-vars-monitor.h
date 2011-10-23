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
#ifndef __NMV_VARS_MONITOR_H__
#define __NMV_VARS_MONITOR_H__

#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-object.h"
#include "nmv-i-perspective.h"
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

/// \brief A widget that can monitor the state of a given set of
/// variables.
///
/// Each time the debugger stops, this widget updates the state of the
/// variables that have been added to it.  Whenever a variable has
/// gone out of scope [when it hasn't been created by address] the
/// widget is supposed to clearly show it
class NEMIVER_API VarsMonitor : public nemiver::common::Object {
    // Non copyable
    VarsMonitor (const VarsMonitor&);
    VarsMonitor& operator= (const VarsMonitor&);

    struct Priv;
    SafePtr<Priv> m_priv;

 protected:
    VarsMonitor ();

 public:
    VarsMonitor (IDebugger &a_dbg,
                 IPerspective &a_perspective);
    virtual ~VarsMonitor ();
    Gtk::Widget& widget ();
    void add_variable (const IDebugger::VariableSafePtr a_var);
    void add_variables (const IDebugger::VariableList &a_vars);
    void remove_variable (const IDebugger::VariableSafePtr a_var);
    void remove_variables (const IDebugger::VariableList &a_vars);
    void re_init_widget ();
};// end VarsMonitor

NEMIVER_END_NAMESPACE (nemiver)

#endif // __NMV_VARS_MONITOR_H__
