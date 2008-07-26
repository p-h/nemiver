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
#include "common/nmv-exception.h"
#include "nmv-ui-utils.h"
#include <glib/gi18n.h>

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
        if (a_tab[i].m_accel != "")
        {
            a_group->add (action,
                    Gtk::AccelKey (a_tab[i].m_accel),
                    a_tab[i].m_activate_slot) ;
        }
        else
        {
            a_group->add (action,
                    a_tab[i].m_activate_slot) ;
        }
    }
}

/// A message dialog that allows the user to chose to not
/// be shown that dialog again
class DontShowAgainMsgDialog : public Gtk::MessageDialog {

    DontShowAgainMsgDialog (const DontShowAgainMsgDialog&);
    DontShowAgainMsgDialog& operator= (const DontShowAgainMsgDialog&);

    Gtk::CheckButton *m_check_button;
public:
    explicit DontShowAgainMsgDialog (const Glib::ustring &a_message,
                                     bool a_propose_dont_ask_again = false,
                                     bool a_use_markup = false,
                                     Gtk::MessageType a_type = Gtk::MESSAGE_INFO,
                                     Gtk::ButtonsType a_buttons = Gtk::BUTTONS_OK,
                                     bool a_modal = false) :
        Gtk::MessageDialog (a_message,
                            a_use_markup,
                            a_type,
                            a_buttons,
                            a_modal),
        m_check_button (0)
    {
        if (a_propose_dont_ask_again)
            pack_dont_ask_me_again_question ();
    }

    void pack_dont_ask_me_again_question ()
    {
        RETURN_IF_FAIL (!m_check_button);

        m_check_button =
            Gtk::manage (new Gtk::CheckButton (_("Do not ask me again")));
        RETURN_IF_FAIL (m_check_button);

        Gtk::Alignment *align =
            Gtk::manage (new Gtk::Alignment);
        RETURN_IF_FAIL (align);

        align->add (*m_check_button);
        RETURN_IF_FAIL (get_vbox ());

        align->show_all ();
        get_vbox ()->pack_end (*align, true, true, 6);
    }

    bool dont_ask_this_again () const
    {
        if (!m_check_button)
            return false;
        return m_check_button->get_active ();
    }
};//end class DontShowAgainMsgDialog

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
                               Gtk::BUTTONS_YES_NO, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
ask_yes_no_question (const UString &a_message,
                     bool a_propose_dont_ask_again,
                     bool &a_dont_ask_this_again)
{
    DontShowAgainMsgDialog dialog (a_message, a_propose_dont_ask_again,
                                   false, Gtk::MESSAGE_QUESTION,
                                   Gtk::BUTTONS_YES_NO, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    int result = dialog.run ();
    a_dont_ask_this_again = dialog.dont_ask_this_again ();
    return result;
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

