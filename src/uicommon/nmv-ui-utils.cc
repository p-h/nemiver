/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4; -*- */

// Author: Dodji Seketeli
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
#include <glib/gi18n.h>
#include "common/nmv-exception.h"
#include "nmv-ui-utils.h"
#include "nmv-locate-file-dialog.h"


using namespace nemiver::common;

namespace nemiver {
namespace ui_utils {

void
add_action_entries_to_action_group (const ActionEntry a_tab[],
                                    int a_num_entries,
                                    Glib::RefPtr<Gtk::ActionGroup> &a_group)
{
    THROW_IF_FAIL (a_group);

    for (int i=0; i < a_num_entries; ++i) {
        Glib::RefPtr<Gtk::Action> action = a_tab[i].to_action ();
        if (a_tab[i].m_accel != "") {
            a_group->add (action,
                          Gtk::AccelKey (a_tab[i].m_accel),
                          a_tab[i].m_activate_slot);
        } else {
            a_group->add (action, a_tab[i].m_activate_slot);
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
                                     Gtk::MessageType a_type
                                                         = Gtk::MESSAGE_INFO,
                                     Gtk::ButtonsType a_buttons
                                                         = Gtk::BUTTONS_OK,
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
                                               Gtk::BUTTONS_OK, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
display_warning (const UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false,
                               Gtk::MESSAGE_WARNING,
                               Gtk::BUTTONS_OK, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
display_error (const UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false,
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_OK, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
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
                               true);

    dialog.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button (Gtk::Stock::NO, Gtk::RESPONSE_NO);
    dialog.add_button (Gtk::Stock::YES, Gtk::RESPONSE_YES);
    dialog.set_default_response (Gtk::RESPONSE_CANCEL);
    return dialog.run ();
}

bool
ask_user_to_select_file (const UString &a_file_name,
                         const UString &a_default_dir,
                         UString &a_selected_file_path)
{
    LocateFileDialog dialog ("", a_file_name);
    // start looking in the default directory
    dialog.file_location (a_default_dir);
    int result = dialog.run ();
    if (result == Gtk::RESPONSE_OK) {
        UString file_path = dialog.file_location ();
        if (!Glib::file_test (file_path, Glib::FILE_TEST_IS_REGULAR)
            || (Glib::path_get_basename (a_file_name)
                != Glib::path_get_basename (file_path)))
            return false;
        UString parent_dir =
            Glib::filename_to_utf8 (Glib::path_get_dirname
                                                (dialog.file_location ()));
        if (!Glib::file_test (parent_dir, Glib::FILE_TEST_IS_DIR))
            return false;

        a_selected_file_path = file_path;
        return true;
    }
    return false;
}

bool
find_absolute_path_or_ask_user (const UString& a_file_path,
                                const UString &a_prog_path,
                                const UString &a_cwd,
                                list<UString> &a_session_dirs,
                                const list<UString> &a_global_dirs,
                                map<UString, bool> &a_ignore_paths,
                                bool a_ignore_if_not_found,
                                UString& a_absolute_path)
{
    if (!env::find_file_absolute_or_relative (a_file_path, a_prog_path,
                                              a_cwd, a_session_dirs, a_global_dirs,
                                              a_absolute_path)) {
        if (a_ignore_paths.find (a_file_path)
            != a_ignore_paths.end ())
            // We didn't find a_file_path but as we were previously
            // requested to *not* ask the user to locate it, just
            // pretend we didn't find the file.
            return false;
        if (ask_user_to_select_file (a_file_path, a_cwd, a_absolute_path)) {
            UString parent_dir =
                Glib::filename_to_utf8 (Glib::path_get_dirname
                                        (a_absolute_path));
            a_session_dirs.push_back (parent_dir);
        } else {
            if (a_ignore_if_not_found)
                // Don't ask the user to locate a_file_path next time.
                a_ignore_paths[a_file_path] = true;
            return false;
        }
    }
    return true;
}

/// Given a file path P and a line number N , reads the line N from P
/// and return it iff the function returns true. This is useful
/// e.g. when forging a mixed source/assembly source view, and we want
/// to display a source line N from a file P.
///
/// \param a_file_path the file path to consider. Not necessarily
/// absolute. If the file is not found, it will be searched for very hard
/// in many places.
/// \param a_prog_path the path of to the current program. This can be
/// useful to search for a_file_path in case we have to search for it
/// very hard.
/// \param a_cwd the current working directory. This can be useful to
/// search for a_file_path.
/// \param a_sess_dirs a list of directories where to search for if
/// all other places we searched for failed.
/// \param a_glob_dirs a list of directories where to seach for if
/// anything else failed so far.
/// \param a_ignore_paths a list of file paths to not search for. If
/// a_file_path is present in these, we will not search for it.
/// \param a_ignore_if_not_found if true and if we haven't found
/// a_file_path so far, then add it to a_ignore_paths.
/// \param a_line_number the line number to consider
/// \param a_line the string containing the resulting line read, if
/// and only if the function returned true.
/// \return true upon successful completion, false otherwise.
bool
find_file_and_read_line (const UString &a_file_path,
                         const UString &a_prog_path,
                         const UString &a_cwd,
                         list<UString> &a_sess_dirs,
                         const list<UString> &a_glob_dirs,
                         map<UString, bool> &a_ignore_paths,
                         bool a_ignore_if_not_found,
                         int a_line_number,
                         string &a_line)
{
    if (a_file_path.empty ())
        return false;

    UString path;
    if (!find_absolute_path_or_ask_user (a_file_path, a_prog_path,
                                         a_cwd, a_sess_dirs, a_glob_dirs,
                                         a_ignore_paths, a_ignore_if_not_found,
                                         path))
        return false;

    return env::read_file_line (path, a_line_number, a_line);
}

}//end namespace ui_utils
}//end namespace nemiver

