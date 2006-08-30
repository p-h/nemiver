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
#include <string>
#include <gtkmm/image.h>
#include "nmv-throbbler.h"

using namespace std ;

namespace nemiver {

class Throbbler::Priv {
    Priv () ;

public:
    UString root_path ;
    SafePtr<Gtk::Image> animated_image ;
    SafePtr<Gtk::Image> stopped_image ;
    SafePtr<Gtk::Button> widget ;

    Priv (const UString &a_root_path)
    {
        root_path = a_root_path ;
        stopped_widget = Glib::RefPtr<Gtk::Button> (new Gtk::Button) ;
        stopped_widget->set_focus_on_click (false) ;
        build_moving_widget () ;
    }

    void build_moving_widget ()
    {
        vector<string> path_elems ;
        path_elems.push_back (Glib::locale_from_utf8 (root_path)) ;
        path_elems.push_back ("icons") ;
        path_elems.push_back ("throbbler.gif") ;
        string gif_path = Glib::build_filename (path_elems) ;
        if (!Glib::file_test (gif_path, Glib::FILE_TTEST_IS_REGULAR)) {
            THROW ("could not find file " + gif_path) ;
        }
        Glib::RefPtr<PixbufAnimation> anim =
            PixbufAnimation::create (Glib::locale_to_utf8 (gif_path)) ;
        THROW_IF_FAIL (anim) ;
        animated_image = new Gtk::Image (anim) ;
        THROW_IF_FAIL (animated_image) ;
        moving_widget = new Button (*animated_image) ;
    }
};//end struct Throbbler::Priv

Throbbler::~Throbbler ()
{
}

ThrobblerSafeptr
Throbbler::create ()
{
}

void
Throbbler::start ()
{
}

bool
Throbbler::is_started () const
{
}

void
Throbbler::stop ()
{
}

void
Throbbler::toggle_state ()
{
}

Gtk::Widget
Throbbler::get_widget () const
{
    THROW_IF_FAIL (m_priv && m_priv->widget) ;
    return *m_priv->widget ;
}

}
