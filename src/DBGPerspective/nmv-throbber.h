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
#ifndef __NMV_THROBBER_H__
#define __NMV_THROBBER_H__

#include <gtkmm/widget.h>
#include "nmv-object.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-ustring.h"

using nemiver::common::Object ;
using nemiver::common::SafePtr ;
using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;
using nemiver::common::UString ;

NEMIVER_BEGIN_NAMESPACE (nemiver)
class Throbber ;
typedef SafePtr<Throbber, ObjectRef, ObjectUnref> ThrobberSafePtr ;

class Throbber : public Object {
    class Priv ;
    SafePtr<Priv> m_priv ;

    Throbber () ;
    Throbber (const UString &a_root_path) ;

public:
    virtual ~Throbber ()  ;
    static ThrobberSafePtr create (const UString &a_root_path) ;
    void start () ;
    bool is_started () const ;
    void stop () ;
    void toggle_state () ;
    Gtk::Widget& get_widget () const ;
};//end class Throbber

NEMIVER_END_NAMESPACE (nemiver)
#endif //__NMV_THROBBER_H__

