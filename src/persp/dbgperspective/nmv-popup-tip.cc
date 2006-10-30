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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include <gtkmm/label.h>
#include "nmv-popup-tip.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class PopupTip::Priv {
    Priv () ;

public:
    Gtk::TextView *text_view ;
    Gtk::ScrolledWindow *scr_win ;
    Glib::RefPtr<Gtk::TextBuffer> text_buffer ;
    sigc::connection expose_event_connection ;
    Gtk::Window &window ;
    int show_position_x ;
    int show_position_y ;

    Priv (const UString &a_message,
          Gtk::Window &a_window) :
        text_view (0),
        scr_win (0),
        window (a_window),
        show_position_x (0),
        show_position_y (0)
    {
        scr_win = Gtk::manage (new Gtk::ScrolledWindow) ;
        THROW_IF_FAIL (scr_win) ;
        scr_win->set_policy (Gtk::POLICY_NEVER, Gtk::POLICY_NEVER) ;
        text_view = Gtk::manage (new Gtk::TextView) ;
        THROW_IF_FAIL (text_view) ;
        scr_win->add (*text_view) ;
        text_buffer = Gtk::TextBuffer::create () ;
        THROW_IF_FAIL (text_buffer) ;
        text_buffer->set_text (a_message) ;
        text_view->set_buffer (text_buffer) ;
        expose_event_connection = window.signal_expose_event ().connect
        (sigc::mem_fun (*this, &Priv::on_expose_event_signal)) ;
        window.set_resizable (false) ;
        window.set_app_paintable (true) ;
        window.set_border_width (4) ;
        window.add (*scr_win) ;
        window.hide () ;
    }

    void paint_window ()
    {
        Gtk::Requisition req = window.size_request () ;
        Glib::RefPtr<Gtk::Style> style = window.get_style () ;
        THROW_IF_FAIL (style) ;
        Gdk::Rectangle zero_rect ;
        style->paint_flat_box (window.get_window (),
                               Gtk::STATE_NORMAL,
                               Gtk::SHADOW_OUT,
                               zero_rect,
                               window,
                               "tooltip",
                               show_position_x,
                               show_position_y,
                               req.width,
                               req.height);
    }

    bool on_expose_event_signal (GdkEventExpose *a_event)
    {
        NEMIVER_TRY
        if (a_event) {}

        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        paint_window () ;
        window.move (show_position_x, show_position_y) ;

        NEMIVER_CATCH
        return false ;
    }
};//end PopupTip

PopupTip::PopupTip (const UString &a_text) :
    Gtk::Window (Gtk::WINDOW_POPUP)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    m_priv = new PopupTip::Priv (a_text, *this);
}

PopupTip::~PopupTip ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    m_priv = NULL ;
}

void
PopupTip::text (const UString &a_text)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->text_buffer) ;
    m_priv->text_buffer->set_text (a_text) ;
}

UString
PopupTip::text () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->text_buffer) ;
    return m_priv->text_buffer->get_text () ;
}

void
PopupTip::set_show_position (int a_x, int a_y)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    m_priv->show_position_x = a_x ;
    m_priv->show_position_y = a_y ;
}

NEMIVER_END_NAMESPACE (nemiver)

