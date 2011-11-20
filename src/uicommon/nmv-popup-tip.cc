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
#include "config.h"
#include <gtkmm/label.h>
#include "common/nmv-exception.h"
#include "nmv-popup-tip.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class PopupTip::Priv
{
    Priv ();

public:

    Gtk::Window &window;
    Gtk::Notebook *notebook;
    Gtk::Label  *label;
    int show_position_x;
    int show_position_y;
    int label_index;
    int custom_widget_index;

    Priv (Gtk::Window &a_window) :
        window (a_window),
        label (0),
        show_position_x (0),
        show_position_y (0),
        label_index (-1),
        custom_widget_index (-1)
    {
        window.hide ();
        // Un-comment this to get tooltip specific colors for the window
        // window.set_name ("gtk-tooltips");
        window.set_resizable (false);
        window.set_app_paintable (true);
        window.set_border_width (4);
        notebook = Gtk::manage (new Gtk::Notebook);
        notebook->set_show_tabs (false);
        notebook->show ();
        window.add (*notebook);
        label = Gtk::manage (new Gtk::Label);
        label->set_line_wrap (true);
        label->set_alignment (0.5, 0.5);
        label->show ();
        label_index = notebook->append_page (*label);
        window.add_events (Gdk::LEAVE_NOTIFY_MASK
                           | Gdk::FOCUS_CHANGE_MASK);
        window.signal_leave_notify_event ().connect
            (sigc::mem_fun (*this, &Priv::on_leave_notify_event));
        window.signal_focus_out_event ().connect
            (sigc::mem_fun (*this, &Priv::on_signal_focus_out_event));
    }

    bool
    on_leave_notify_event (GdkEventCrossing *a_event)
    {
        NEMIVER_TRY

        LOG_FUNCTION_SCOPE_NORMAL_DD;

        if (a_event
            && a_event->type == GDK_LEAVE_NOTIFY
            && a_event->detail != GDK_NOTIFY_INFERIOR)
            window.hide ();

        NEMIVER_CATCH

        return false;
    }

    bool
    on_signal_focus_out_event (GdkEventFocus *)
    {
        NEMIVER_TRY

        window.hide ();

        NEMIVER_CATCH

        return false;
    }

};//end PopupTip

PopupTip::PopupTip (const UString &a_text) :
    Gtk::Window (Gtk::WINDOW_POPUP)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    set_type_hint (Gdk::WINDOW_TYPE_HINT_POPUP_MENU );
    m_priv.reset (new PopupTip::Priv (*this));
    if (!a_text.empty ())
        text (a_text);
}

PopupTip::~PopupTip ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
}

void
PopupTip::text (const UString &a_text)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->label);

    if (a_text != "" ) {
        if (a_text.get_number_of_lines () > 1) {
            m_priv->label->set_single_line_mode (false);
        } else {
            m_priv->label->set_single_line_mode (true);
        }
    }
    m_priv->label->set_text (a_text);
    m_priv->notebook->set_current_page (m_priv->label_index);
}

UString
PopupTip::text () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->label);
    return m_priv->label->get_text ();
}

void
PopupTip::set_child (Gtk::Widget &a_widget)
{
    THROW_IF_FAIL (m_priv);
    if (m_priv->custom_widget_index >= 0) {
        m_priv->notebook->remove_page (m_priv->custom_widget_index);
    }
    a_widget.show_all ();
    m_priv->custom_widget_index =
            m_priv->notebook->append_page (a_widget);
    m_priv->notebook->set_current_page (m_priv->custom_widget_index);
}

void
PopupTip::set_show_position (int a_x, int a_y)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    m_priv->show_position_x = a_x;
    m_priv->show_position_y = a_y;
}

void
PopupTip::show ()
{
    THROW_IF_FAIL (m_priv);
    move (m_priv->show_position_x, m_priv->show_position_y);
    Gtk::Window::show ();
}

void
PopupTip::show_all ()
{
    THROW_IF_FAIL (m_priv);
    move (m_priv->show_position_x, m_priv->show_position_y);
    Gtk::Window::show_all ();
}

void
PopupTip::show_at_position (int a_x, int a_y)
{
    set_show_position (a_x, a_y);
    show_all ();
}

NEMIVER_END_NAMESPACE (nemiver)

