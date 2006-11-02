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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NEMIVER_DIALOG_H__
#define __NEMIVER_DIALOG_H__

#include "nmv-object.h"
#include "nmv-safe-ptr-utils.h"

namespace Gtk {
    class Dialog ;
}

namespace Gnome {
    namespace Glade {
        class Xml ;
    }
}

namespace nemiver {

namespace common {
class UString ;
}

using nemiver::common::UString ;
using nemiver::common::SafePtr ;

class Dialog : public common::Object {
    //non copyable
    Dialog (const Dialog&) ;
    Dialog& operator= (const Dialog&) ;

    //force to create on the stack
    void* operator new (size_t) ;

    Dialog () ;
public:

    Dialog (const UString &a_resource_root_path,
            const UString &a_glade_filename,
            const UString &a_widget_name) ;

    virtual ~Dialog () ;

    virtual gint run () ;

protected:
    SafePtr<Gtk::Dialog> m_dialog ;
    Glib::RefPtr<Gnome::Glade::Xml> m_glade ;
};//end class nemiver

}//end namespace nemiver

#endif //__NEMIVER_DIALOG_H__

