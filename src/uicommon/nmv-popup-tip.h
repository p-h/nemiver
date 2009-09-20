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

#ifndef __NMV_POPUP_TIP_H__
#define __NMV_POPUP_TIP_H__

#include <gtkmm/window.h>
#include "common/nmv-ustring.h"
#include "common/nmv-safe-ptr-utils.h"

using nemiver::common::UString;
using nemiver::common::SafePtr;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class PopupTip : public Gtk::Window {
    //non copyable
    PopupTip (const PopupTip&);
    PopupTip& operator= (const PopupTip&);
    class Priv;
    SafePtr<Priv> m_priv;

public:

    PopupTip (const UString &a_text="");
    virtual ~PopupTip ();
    void text (const UString &);
    UString text () const;
    void set_child (Gtk::Widget &a_widget);
    void set_show_position (int a_x, int a_y);
    void show ();
    void show_all ();
    void show_at_position (int a_x, int a_y);
};//end class PopupTip

NEMIVER_END_NAMESPACE (nemiver)
#endif //__NMV_POPUP_TIP_H__

