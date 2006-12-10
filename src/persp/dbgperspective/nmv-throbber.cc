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
#include <gtkmm/toolbutton.h>
#include <gdkmm/pixbufanimation.h>
#include "nmv-exception.h"
#include "nmv-ustring.h"
#include "nmv-throbber.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

class Throbber::Priv {
    Priv () ;

public:
    bool is_started ;
    UString root_path ;
    SafePtr<Gtk::Image> animated_image ;
    SafePtr<Gtk::Image> stopped_image ;
    SafePtr<Gtk::ToolButton> widget ;

    Priv (const UString &a_root_path) :
        is_started (false)
    {
        root_path = a_root_path ;
        widget.reset (new Gtk::ToolButton) ;
        THROW_IF_FAIL (widget) ;
        build_widget () ;
    }

    void build_widget ()
    {
        vector<string> path_elems ;
        path_elems.push_back (Glib::locale_from_utf8 (root_path)) ;
        path_elems.push_back ("icons") ;
        string base_dir = Glib::build_filename (path_elems) ;
        string moving_gif_path = Glib::build_filename (base_dir, "throbber.gif") ;
        if (!Glib::file_test (moving_gif_path, Glib::FILE_TEST_IS_REGULAR)) {
            THROW ("could not find file " + moving_gif_path) ;
        }
        Glib::RefPtr<Gdk::PixbufAnimation> anim =
            Gdk::PixbufAnimation::create_from_file (Glib::locale_to_utf8
                                                                (moving_gif_path)) ;
        THROW_IF_FAIL (anim) ;

        string stopped_img_path = Glib::build_filename (base_dir, "green.png") ;
        if (!Glib::file_test (stopped_img_path, Glib::FILE_TEST_IS_REGULAR)) {
            THROW ("could not find file " + stopped_img_path) ;
        }
        Glib::RefPtr<Gdk::PixbufAnimation> stopped_pixbuf =
            Gdk::PixbufAnimation::create_from_file (Glib::locale_to_utf8
                                                                (stopped_img_path)) ;
        animated_image.reset (new Gtk::Image (anim)) ;
        stopped_image.reset (new Gtk::Image (stopped_pixbuf)) ;
        THROW_IF_FAIL (animated_image) ;
        widget.reset (new Gtk::ToolButton ()) ;
        widget->set_icon_widget (*stopped_image) ;
    }
};//end struct Throbber::Priv

Throbber::~Throbber ()
{
}

Throbber::Throbber ()
{
}

Throbber::Throbber (const UString &a_root_path)
{
    m_priv.reset (new Priv (a_root_path));
}

ThrobberSafePtr
Throbber::create (const UString &a_root_path)
{
    ThrobberSafePtr result (new Throbber (a_root_path)) ;
    return result ;
}

void
Throbber::start ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->widget) ;
    THROW_IF_FAIL (m_priv->animated_image) ;
    m_priv->widget->set_icon_widget (*m_priv->animated_image) ;
    m_priv->is_started = true ;
}

bool
Throbber::is_started () const
{
    return m_priv->is_started ;
}

void
Throbber::stop ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->widget) ;
    THROW_IF_FAIL (m_priv->animated_image) ;
    m_priv->widget->set_icon_widget (*m_priv->stopped_image) ;
    m_priv->is_started = false ;
}

void
Throbber::toggle_state ()
{
    if (is_started ()) {
        stop () ;
    } else {
        start () ;
    }
}

Gtk::ToolItem&
Throbber::get_widget () const
{
    THROW_IF_FAIL (m_priv && m_priv->widget) ;
    return *m_priv->widget ;
}

}
