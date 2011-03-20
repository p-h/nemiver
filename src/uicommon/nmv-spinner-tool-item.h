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
#ifndef __NMV_SPINNER_TOOL_ITEM_H__
#define __NMV_SPINNER_TOOL_ITEM_H__

#include <gtkmm/toolitem.h>
#include "common/nmv-safe-ptr-utils.h"

namespace Gtk {
    class Spinner;
}

NEMIVER_BEGIN_NAMESPACE (nemiver)

using nemiver::common::SafePtr;

class SpinnerToolItem : public Gtk::ToolItem {
    SafePtr<Gtk::Spinner> m_spinner;

public:
    SpinnerToolItem ();
    virtual ~SpinnerToolItem ();
    void start ();
    void stop ();

protected:
    virtual void on_toolbar_reconfigured ();
};//end class SpinnerToolItem

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_SPINNER_TOOL_ITEM_H__
