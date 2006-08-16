/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4; -*- */

/*
 *This file is part of the Nemiver Project.
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
#include "nmv-exception.h"
#include "nmv-ui-utils.h"

using namespace nemiver::common ;

namespace nemiver {
namespace ui_utils {

void
add_action_entries_to_action_group (const ActionEntry a_tab[],
                                    int a_num_entries,
                                    Glib::RefPtr<Gtk::ActionGroup> &a_group)
{
    THROW_IF_FAIL (a_group) ;

    for (int i=0; i < a_num_entries ; ++i) {
        Glib::RefPtr<Gtk::Action> action = a_tab[i].to_action () ;
        a_group->add (action,
                      Gtk::AccelKey (a_tab[i].m_accel),
                      a_tab[i].m_activate_slot) ;
    }
}
int
display_info (const UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false, Gtk::MESSAGE_INFO,
                                               Gtk::BUTTONS_OK, true) ;
    dialog.set_default_response (Gtk::RESPONSE_OK) ;
    return dialog.run () ;
}

int
display_warning (const UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false,
                               Gtk::MESSAGE_WARNING,
                               Gtk::BUTTONS_OK, true) ;
    dialog.set_default_response (Gtk::RESPONSE_OK) ;
    return dialog.run () ;
}

int
display_error (const UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false,
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_OK, true) ;
    dialog.set_default_response (Gtk::RESPONSE_OK) ;
    return dialog.run () ;
}

int
ask_yes_no_question (const UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false,
                               Gtk::MESSAGE_QUESTION,
                               Gtk::BUTTONS_YES_NO,
                               true) ;
    dialog.set_default_response (Gtk::RESPONSE_OK) ;
    return dialog.run () ;
}

int
ask_yes_no_cancel_question (const common::UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false,
                               Gtk::MESSAGE_QUESTION,
                               Gtk::BUTTONS_NONE,
                               true) ;

    dialog.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
    dialog.add_button (Gtk::Stock::NO, Gtk::RESPONSE_NO) ;
    dialog.add_button (Gtk::Stock::YES, Gtk::RESPONSE_YES) ;
    dialog.set_default_response (Gtk::RESPONSE_CANCEL) ;
    return dialog.run () ;
}

}//end namespace ui_utils
}//end namespace nemiver

