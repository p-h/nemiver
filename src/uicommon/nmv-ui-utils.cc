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
#include "config.h"
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

    DontShowAgainMsgDialog (Gtk::Window &a_parent_window,
                            const DontShowAgainMsgDialog&);
    DontShowAgainMsgDialog& operator= (const DontShowAgainMsgDialog&);

    Gtk::CheckButton *m_check_button;
public:
    explicit DontShowAgainMsgDialog (Gtk::Window &a_parent_window,
                                     const Glib::ustring &a_message,
                                     bool a_propose_dont_ask_again = false,
                                     bool a_use_markup = false,
                                     Gtk::MessageType a_type
                                                         = Gtk::MESSAGE_INFO,
                                     Gtk::ButtonsType a_buttons
                                                         = Gtk::BUTTONS_OK,
                                     bool a_modal = false) :
        Gtk::MessageDialog (a_parent_window,
                            a_message,
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
display_info (Gtk::Window &a_parent_window,
              const UString &a_message)
{
    Gtk::MessageDialog dialog (a_parent_window,
                               a_message, false,
                               Gtk::MESSAGE_INFO,
                               Gtk::BUTTONS_OK, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
display_warning (Gtk::Window &a_parent_window,
                 const UString &a_message)
{
    Gtk::MessageDialog dialog (a_parent_window,
                               a_message, false,
                               Gtk::MESSAGE_WARNING,
                               Gtk::BUTTONS_OK, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
display_error (Gtk::Window &a_parent_window,
               const UString &a_message)
{
    Gtk::MessageDialog dialog (a_parent_window,
                               a_message, false,
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_OK, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
display_error_not_transient (const UString &a_message)
{
    Gtk::MessageDialog dialog (a_message, false,
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_OK, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
ask_yes_no_question (Gtk::Window &a_parent_window,
                     const UString &a_message)
{
    Gtk::MessageDialog dialog (a_parent_window,
                               a_message, false,
                               Gtk::MESSAGE_QUESTION,
                               Gtk::BUTTONS_YES_NO, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    return dialog.run ();
}

int
ask_yes_no_question (Gtk::Window &a_parent_window,
                     const UString &a_message,
                     bool a_propose_dont_ask_again,
                     bool &a_dont_ask_this_again)
{
    DontShowAgainMsgDialog dialog (a_parent_window,
                                   a_message, a_propose_dont_ask_again,
                                   false, Gtk::MESSAGE_QUESTION,
                                   Gtk::BUTTONS_YES_NO, true);
    dialog.set_default_response (Gtk::RESPONSE_OK);
    int result = dialog.run ();
    a_dont_ask_this_again = dialog.dont_ask_this_again ();
    return result;
}

int
ask_yes_no_cancel_question (Gtk::Window &a_parent_window,
                            const common::UString &a_message)
{
    Gtk::MessageDialog dialog (a_parent_window,
                               a_message, false,
                               Gtk::MESSAGE_QUESTION,
                               Gtk::BUTTONS_NONE,
                               true);

    dialog.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button (Gtk::Stock::NO, Gtk::RESPONSE_NO);
    dialog.add_button (Gtk::Stock::YES, Gtk::RESPONSE_YES);
    dialog.set_default_response (Gtk::RESPONSE_CANCEL);
    return dialog.run ();
}

/// Use a dialog to interactively ask the user to select a file.
///
/// \param a_parent the parent window used by the (transient) dialog.
///
/// \param a_file_name the name of the file to ask the user help us
/// look for.
///
/// \param a_default_dir the default directory from where to the user
/// is proposed to start the search from.
///
/// \param a_selected_file_path the resulting absolute file path as
/// selected by the user.  This is set iff the function returns true.
///
/// \return true iff the user actually selected a file.
bool
ask_user_to_select_file (Gtk::Window &a_parent,
                         const UString &a_file_name,
                         const UString &a_default_dir,
                         UString &a_selected_file_path)
{
    LocateFileDialog dialog ("", a_file_name, a_parent);
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

/// Find a given file from a set of directories.
///
/// If the file is not found, then graphically ask the user to find it
/// instread.
///
/// \param a_parent_window the parent window of the dialog used to ask
/// the user where to find the file, should the need arise.
///
/// \param a_file_name the file name to find.
///
/// \param a_where_to_look the list of directories where to look the
/// file from.
///
/// \param a_session_dirs if the file was found, add its parent
/// directory to this list.
///
/// \param a_ignore_paths If a file not found by this function has its
/// name in this map, then do not ask the user to look for it.  Also,
/// if a_ignore_if_not_found is true and if the file wasn't found
/// this time, the a_file_name is added to this map (a_ignore_paths).
///
/// \param a_absolute_path the absolute path of the file found.
///
/// \return true iff the file was found.
bool
find_file_or_ask_user (Gtk::Window &a_parent_window,
                       const UString& a_file_name,
                       const list<UString> &a_where_to_look,
                       list<UString> &a_session_dirs,
                       map<UString, bool> &a_ignore_paths,
                       bool a_ignore_if_not_found,
                       UString& a_absolute_path)
{
    if (!env::find_file (a_file_name, a_where_to_look, a_absolute_path)) {
        if (a_ignore_paths.find (a_file_name)
            != a_ignore_paths.end ())
            // We didn't find a_file_name but as we were previously
            // requested to *not* ask the user to locate it, just
            // pretend we didn't find the file.
            return false;
        if (ask_user_to_select_file (a_parent_window,
                                     a_file_name,
                                     a_where_to_look.front (),
                                     a_absolute_path)) {
            UString parent_dir =
                Glib::filename_to_utf8 (Glib::path_get_dirname
                                        (a_absolute_path));
            a_session_dirs.push_back (parent_dir);
        } else {
            if (a_ignore_if_not_found)
                // Don't ask the user to locate a_file_path next time.
                a_ignore_paths[a_file_name] = true;
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
/// \param a_parent_window the parent window used by the Dialog widget
/// that we use.
/// \param a_file_path the file path to consider. Not necessarily
/// absolute. If the file is not found, it will be searched for very hard
/// in many places.
/// \param a_where_to_look the list of directories where to look for files.
/// \param a_sess_dirs if the user helps to find the a file,
/// the directory given by the user will be added to this list.
/// \param a_ignore_paths a list of file paths to not search for. If
/// a_file_path is present in these, we will not search for it.
/// \param a_ignore_if_not_found if true and if we haven't found
/// a_file_path so far, then add it to a_ignore_paths.
/// \param a_line_number the line number to consider
/// \param a_line the string containing the resulting line read, if
/// and only if the function returned true.
/// \return true upon successful completion, false otherwise.
bool
find_file_and_read_line (Gtk::Window &a_parent_window,
                         const UString &a_file_path,
                         const list<UString> &a_where_to_look,
                         list<UString> &a_sess_dirs,
                         map<UString, bool> &a_ignore_paths,
                         int a_line_number,
                         string &a_line)
{
    if (a_file_path.empty ())
        return false;

    UString path;
    if (!find_file_or_ask_user (a_parent_window,
                                a_file_path,
                                a_where_to_look,
                                a_sess_dirs,
                                a_ignore_paths,
                                true, path))
        return false;

    return env::read_file_line (path, a_line_number, a_line);
}

}//end namespace ui_utils
}//end namespace nemiver

