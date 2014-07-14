// Author: Dodji Seketeli

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
#ifndef NMV_WATCHPOINT_DIALOG_H__
#define NMV_WATCHPOINT_DIALOG_H__

#include "common/nmv-safe-ptr-utils.h"
#include "nmv-dialog.h"
#include "nmv-i-debugger.h"
#include "nmv-i-perspective.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

namespace common {
class UString;
}

class WatchpointDialog : public Dialog
{
    struct Priv;
    SafePtr<Priv> m_priv;
    WatchpointDialog ();
    WatchpointDialog (WatchpointDialog &);
    WatchpointDialog& operator= (WatchpointDialog &);

public:

    // The mode of the Watchpoint.
    // a WRITE_MODE means the watchpoint triggers when the watched location
    // is read, and READ_MODE means the watchpoint trigger when the
    // watches location is written to. A watchpoint mode can be READ and WRITE;
    // in that case, it's called an access watchpoint.
    enum Mode
    {
        UNDEFINED_MODE = 0,
        WRITE_MODE = 1,
        READ_MODE = 1 << 1
    };

    WatchpointDialog (Gtk::Window &a_parent,
                      const UString &a_resource_root_path,
                      IDebugger &a_debugger,
                      IPerspective &a_perspective);
    virtual ~WatchpointDialog ();

    const UString expression () const;
    void expression (const UString &);

    Mode mode () const;
    void mode (Mode);
};// end class WatchpointDialog

WatchpointDialog::Mode operator| (WatchpointDialog::Mode,
                                  WatchpointDialog::Mode);
WatchpointDialog::Mode operator& (WatchpointDialog::Mode,
                                  WatchpointDialog::Mode);
WatchpointDialog::Mode operator~ (WatchpointDialog::Mode);
WatchpointDialog::Mode& operator|= (WatchpointDialog::Mode&,
                                   WatchpointDialog::Mode);
WatchpointDialog::Mode& operator&= (WatchpointDialog::Mode&,
                                    WatchpointDialog::Mode);
NEMIVER_END_NAMESPACE (nemiver)

#endif // NMV_WATCHPOINT_DIALOG_H__
