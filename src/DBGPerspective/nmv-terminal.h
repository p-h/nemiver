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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */

#ifndef NMV_TERMINAL
#define NMV_TERMINAL

#include "nmv-safe-ptr-utils.h"
#include "nmv-env.h"
#include "nmv-ustring.h"
#include "nmv-object.h"

using nemiver::common::UString ;
using nemiver::common::Object ;
using nemiver::common::SafePtr ;

namespace Gtk {class Widget;}

NEMIVER_BEGIN_NAMESPACE(nemiver)

class Terminal : public Object {
    //non copyable
    Terminal (const Terminal &) ;
    Terminal& operator= (const Terminal &) ;

    struct Priv ;
    SafePtr<Priv> m_priv ;

public:

    Terminal () ;
    ~Terminal () ;
    Gtk::Widget& widget () const ;
    UString slave_pts_name () const ;
};//end class Terminal

NEMIVER_END_NAMESPACE(nemiver)
#endif //NMV_TERMINAL
