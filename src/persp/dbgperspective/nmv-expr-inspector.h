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
#ifndef __NMV_EXPR_INSPECTOR2_H__
#define __NMV_EXPR_INSPECTOR2_H__

#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"
#include "nmv-i-perspective.h"

namespace Gtk {
    class Widget;
}

NEMIVER_BEGIN_NAMESPACE (nemiver)

namespace common {
    class UString;
}

class ExprInspector : public nemiver::common::Object {
    ExprInspector (const ExprInspector &);
    ExprInspector& operator= (const ExprInspector &);
    ExprInspector ();
    class Priv;
    SafePtr<Priv> m_priv;

public:
    ExprInspector (IDebugger &a_debugger,
		  IPerspective &a_perspective);
    virtual ~ExprInspector ();
    Gtk::Widget& widget () const;
    void set_expression (IDebugger::VariableSafePtr a_variable,
                       bool a_expand = false,
		       bool a_re_visualize = false);
    void inspect_expression (const UString &a_variable_name,
                           bool a_expand = false);
    void inspect_expression (const UString &a_variable_name,
                           bool a_expand,
			   const sigc::slot<void, 
			              const IDebugger::VariableSafePtr> &a_s);
    IDebugger::VariableSafePtr get_expression () const;
    void enable_contextual_menu (bool a_flag);
    bool is_contextual_menu_enabled () const;
    void clear ();

    // Signals
    sigc::signal<void, const IDebugger::VariableSafePtr>&
      expr_inspected_signal () const;
    sigc::signal<void>& cleared_signal () const;

};//end class ExprInspector

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_EXPR_INSPECTOR2_H__

