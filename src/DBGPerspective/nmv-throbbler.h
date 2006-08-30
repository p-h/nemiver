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
#ifndef __NMV_THROBBLER_H__
#define __NMV_THROBBLER_H__

#include <gtkmm/widget.h>
#include "nmv-object.h"
#include "nmv-safe-ptr-utils.h"

using nemiver::common::Object ;
using nemiver::common::SafePtr ;
using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;

namespace nemiver {
class Throbbler ;
typedef SafePtr<Throbbler, ObjectRef, ObjectUnref> ThrobblerSafePtr ;

class Throbbler : public Object {
    class Priv ;
    SafePtr<Priv> m_priv ;

    Throbbler () ;

public:
    virtual ~Throbbler ()  ;
    static ThrobblerSafeptr create () ;
    void start () ;
    bool is_started () const ;
    void stop () ;
    void toggle_state () ;
    Gtk::Widget& get_widget () const ;
};//end class Throbbler

}//end namespace nemiver
#endif //__NMV_THROBBLER_H__

