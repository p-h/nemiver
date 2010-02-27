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
#ifndef __NEMIVER_UI_UTILS_H__
#define __NEMIVER_UI_UTILS_H__

#include "config.h"
#include <gtkmm.h>
#include <libglademm.h>
#include "common/nmv-env.h"
#include "common/nmv-ustring.h"
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-api-macros.h"


#ifndef NEMIVER_CATCH
#define NEMIVER_CATCH \
} catch (Glib::Exception &e) { \
    LOG_ERROR (std::string ("caught exception: '") + e.what () + "'"); \
    nemiver::ui_utils::display_error (e.what ()); \
} catch (std::exception &e) { \
    LOG_ERROR (std::string ("caught exception: '") + e.what () + "'"); \
    nemiver::ui_utils::display_error (e.what ()); \
} catch (...) { \
    LOG_ERROR ("caught unknown exception"); \
    nemiver::ui_utils::display_error ("An unknown error occured"); \
}
#endif

#ifndef NEMIVER_CATCH_AND_RETURN
#define NEMIVER_CATCH_AND_RETURN(a_value) \
} catch (Glib::Exception &e) { \
    LOG_ERROR (std::string ("caught exception: '") + e.what () + "'"); \
    nemiver::ui_utils::display_error (e.what ()); \
    return a_value; \
} catch (std::exception &e) { \
    LOG_ERROR (std::string ("caught exception: '") + e.what () + "'"); \
    nemiver::ui_utils::display_error (e.what ()); \
    return a_value; \
} catch (...) { \
    LOG_ERROR ("Caught unknown exception"); \
    nemiver::ui_utils::display_error ("An unknown error occured"); \
    return a_value; \
}
#endif

using nemiver::common::UString;

namespace nemiver {
namespace ui_utils {

class ActionEntry {

public:
    enum Type {
        DEFAULT=0,
        TOGGLE
    };

    common::UString m_name;
    Gtk::StockID m_stock_id;
    common::UString m_label;
    common::UString m_tooltip;
    sigc::slot<void> m_activate_slot;
    Type m_type;
    common::UString m_accel;
    bool m_is_important;

    Glib::RefPtr<Gtk::Action> to_action () const
    {
        Glib::RefPtr<Gtk::Action> result;
        switch (m_type) {
            case DEFAULT:
                if (m_stock_id.get_string () != "") {
                    result =
                        Gtk::Action::create (m_name, m_stock_id,
                                             m_label, m_tooltip);
                } else {
                    result =
                        Gtk::Action::create (m_name, m_label, m_tooltip);
                }
                break;
            case TOGGLE:
                if (m_stock_id.get_string () != "") {
                    result =
                        Gtk::ToggleAction::create (m_name, m_stock_id,
                                                   m_label, m_tooltip);
                } else {
                    result =
                        Gtk::ToggleAction::create (m_name,
                                                   m_label,
                                                   m_tooltip);
                }
                break;

            default:
                THROW ("should never reach this point");
        }
#ifdef HAVE_GTKMM_2_16
        if (result)
            result->set_is_important (m_is_important);
#endif
        return result;
    }
};//end class ActionEntry

NEMIVER_API void add_action_entries_to_action_group
                                (const ActionEntry a_tab[],
                                 int a_num_entries,
                                 Glib::RefPtr<Gtk::ActionGroup> &a_group);

NEMIVER_API int display_info (const common::UString &a_message);

NEMIVER_API int display_warning (const common::UString &a_message);

NEMIVER_API int display_error (const common::UString &a_message);

NEMIVER_API int ask_yes_no_question (const common::UString &a_message);

NEMIVER_API int ask_yes_no_question (const common::UString &a_message,
                                     bool a_propose_dont_ask_question,
                                     bool &a_dont_ask_this_again);

NEMIVER_API int ask_yes_no_cancel_question (const common::UString &a_message);

template <class T>
T*
get_widget_from_glade (const Glib::RefPtr<Gnome::Glade::Xml> &a_glade,
                       const UString &a_widget_name)
{
    Gtk::Widget *widget = a_glade->get_widget (a_widget_name);
    if (!widget) {
        THROW ("couldn't find widget '"
               + a_widget_name
               + "' in glade file: " + a_glade->get_filename ().c_str ());
    }
    T *result = dynamic_cast<T*> (widget);
    if (!result) {
        //TODO: we may leak widget here if it is a toplevel widget
        //like Gtk::Window. In this case, we should make sure to delete it
        //before bailing out.
        THROW ("widget " + a_widget_name + " is not of the expected type");
    }
    return result;
}

template <class T>
T*
get_widget_from_glade (const UString &a_glade_file_name,
                       const UString &a_widget_name,
                       const Glib::RefPtr<Gnome::Glade::Xml> &a_glade)
{
    UString path_to_glade_file =
                common::env::build_path_to_glade_file (a_glade_file_name);
    a_glade = Gnome::Glade::Xml::create (path_to_glade_file);
    if (!a_glade) {
        THROW ("Could not create glade from file " + path_to_glade_file);
    }
    return get_widget_from_glade<T> (a_glade, a_widget_name);
}

struct WidgetRef {
    void operator () (Gtk::Widget *a_widget)
    {
        if (a_widget) {
            a_widget->reference ();
        }
    }
};//end struct WidgetRef

struct WidgetUnref {
    void operator () (Gtk::Widget *a_widget)
    {
        if (a_widget) {
            a_widget->unreference ();
        }
    }
};//end struct WidgetUnref
}//end namespace ui_utils
}//end namespace nemiver

#endif// __NEMIVER_UI_UTILS_H__

