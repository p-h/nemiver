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
#ifndef __NMV_VAR_INSPECTOR_DIALOG_H__
#define __NMV_VAR_INSPECTOR_DIALOG_H__

#include "nmv-dialog.h"
#include "nmv-i-perspective.h"
#include "nmv-i-debugger.h"
#include "nmv-var-inspector.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VarInspectorDialog : public Dialog {
    //non copyable
    VarInspectorDialog (const VarInspectorDialog &);
    VarInspectorDialog& operator= (const VarInspectorDialog &);

    //tell me why you would want to extend this.
    VarInspectorDialog ();

    class Priv;
    SafePtr<Priv> m_priv;

public:
    VarInspectorDialog (IDebugger &a_debugger,
                        IPerspective &a_perspective);
    virtual ~VarInspectorDialog ();

    UString variable_name () const;
    void inspect_variable (const UString &a_variable_name);
    void inspect_variable (const UString &a_variable_name,
			   const sigc::slot<void, 
			                    const IDebugger::VariableSafePtr> &);
    const IDebugger::VariableSafePtr variable () const;
    VarInspector& inspector () const;
    void set_history (const std::list<UString> &);
    void get_history (std::list<UString> &) const;
};//end class VarInspectorDialog

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_VAR_INSPECTOR_DIALOG_H__
