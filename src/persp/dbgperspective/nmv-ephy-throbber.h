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
#include "nmv-throbber.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class EphyThrobber : public Throbber {
    struct Priv ;
    SafePtr<Priv> m_priv ;
    EphyThrobber () ;
public:
    virtual ~EphyThrobber ()  ;
    static ThrobberSafePtr create () ;
    void start () ;
    bool is_started () const ;
    void stop () ;
    void toggle_state () ;
    Gtk::ToolItem& get_widget () const ;
};//end class EphyThrobber

NEMIVER_END_NAMESPACE (nemiver)
