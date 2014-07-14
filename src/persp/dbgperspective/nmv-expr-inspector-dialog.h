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
#ifndef __NMV_EXPR_INSPECTOR_DIALOG_H__
#define __NMV_EXPR_INSPECTOR_DIALOG_H__

#include "nmv-dialog.h"
#include "nmv-i-perspective.h"
#include "nmv-i-debugger.h"
#include "nmv-expr-inspector.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class ExprInspectorDialog : public Dialog {
    //non copyable
    ExprInspectorDialog (const ExprInspectorDialog &);
    ExprInspectorDialog& operator= (const ExprInspectorDialog &);

    //tell me why you would want to extend this.
    ExprInspectorDialog ();

    class Priv;
    SafePtr<Priv> m_priv;

public:

    /// These flags control the fonctionnalities that are enabled to
    /// be used with the current instance of VarInspectorDialog.
    enum FunctionalityFlags {
      FUNCTIONALITY_NONE = 0,
      /// When this bit is set, the inspector allows the user to
      /// inspect expressions.  Thus the "inspect" button is made
      /// clickable.
      FUNCTIONALITY_EXPR_INSPECTOR = 1,
      /// When this bit is set, the inspect allows the user to send
      /// the inspected expression to the expression (or variable)
      /// monitor.
      FUNCTIONALITY_EXPR_MONITOR_PICKER = 1 << 1,
      // This one should be the last one, and should contain all the
      // flags above.
      FUNCTIONALITY_ALL =
      (FUNCTIONALITY_EXPR_INSPECTOR
       | FUNCTIONALITY_EXPR_MONITOR_PICKER)
    };

    ExprInspectorDialog (Gtk::Window &a_parent,
                         IDebugger &a_debugger,
                         IPerspective &a_perspective);
    virtual ~ExprInspectorDialog ();

    UString expression_name () const;
    void inspect_expression (const UString &a_expression_name);
    void inspect_expression (const UString &a_expression_name,
			     const sigc::slot<void,
			     const IDebugger::VariableSafePtr> &);
    const IDebugger::VariableSafePtr expression () const;
    ExprInspector& inspector () const;
    void set_history (const std::list<UString> &);
    void get_history (std::list<UString> &) const;
    void functionality_mask (int functionality_mask);
    unsigned functionality_mask ();

    // <Signals>

    sigc::signal<void, IDebugger::VariableSafePtr>& expr_monitoring_requested ();

    // </Signals>
};//end class ExprInspectorDialog

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_EXPR_INSPECTOR_DIALOG_H__
