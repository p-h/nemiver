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
#ifndef __NMV_GLOBAL_VARS_INSPECTOR_DIALOG_H__
#define __NMV_GLOBAL_VARS_INSPECTOR_DIALOG_H__

#include <list>
#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-dialog.h"
#include "nmv-i-debugger.h"

namespace Gtk {
    class Widget;
}

NEMIVER_BEGIN_NAMESPACE (nemiver)

class IWorkbench;

class NEMIVER_API GlobalVarsInspectorDialog : public Dialog {
    //non copyable
    GlobalVarsInspectorDialog (const GlobalVarsInspectorDialog&);
    GlobalVarsInspectorDialog& operator= (const GlobalVarsInspectorDialog&);

    struct Priv;
    SafePtr<Priv> m_priv;

protected:
    GlobalVarsInspectorDialog ();

public:

    GlobalVarsInspectorDialog (const UString &a_root_path,
                               IDebuggerSafePtr &a_dbg,
                               IWorkbench &a_wb);
    virtual ~GlobalVarsInspectorDialog ();
};//end GlobalVarsInspectorDialog

NEMIVER_END_NAMESPACE (nemiver)
#endif //__NMV_GLOBAL_VARS_INSPECTOR_DIALOG_H__
