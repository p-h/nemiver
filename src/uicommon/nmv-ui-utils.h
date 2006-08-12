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
#ifndef __NEMIVER_UI_UTILS_H__
#define __NEMIVER_UI_UTILS_H__

#include <gtkmm.h>
#include "nmv-env.h"
#include "nmv-ustring.h"
#include "nmv-api-macros.h"


#ifndef NEMIVER_CATCH
#define NEMIVER_CATCH \
} catch (Glib::Exception &e) { \
    nemiver::ui_utils::display_error (e.what ()) ; \
} catch (std::exception &e) { \
    nemiver::ui_utils::display_error (e.what ()) ; \
} catch (...) { \
    nemiver::ui_utils::display_error ("An unknown error occured") ; \
}
#endif

#ifndef NEMIVER_CATCH_AND_RETURN
#define NEMIVER_CATCH_AND_RETURN(a_value) \
} catch (Glib::Exception &e) { \
    nemiver::ui_utils::display_error (e.what ()) ; \
    return a_value ; \
} catch (std::exception &e) { \
    nemiver::ui_utils::display_error (e.what ()) ; \
    return a_value ; \
} catch (...) { \
    nemiver::ui_utils::display_error ("An unknown error occured") ; \
    return a_value ; \
}
#endif

namespace nemiver {
namespace ui_utils {

struct ActionEntry {
    common::UString m_name ;
    Gtk::StockID m_stock_id ;
    common::UString m_label ;
    common::UString m_tooltip ;
    sigc::slot<void> m_activate_slot;

public:

    Glib::RefPtr<Gtk::Action> to_action () const
    {
        Glib::RefPtr<Gtk::Action> result ;
        if (m_stock_id.get_string () != "") {
            result = Gtk::Action::create (m_name, m_stock_id, m_label, m_tooltip);
        } else {
            result = Gtk::Action::create (m_name, m_label, m_tooltip);
        }
        return result ;
    }

};//end class ActionEntry

NEMIVER_API void add_action_entries_to_action_group
                                (const ActionEntry a_tab[],
                                 int a_num_entries,
                                 Glib::RefPtr<Gtk::ActionGroup> &a_group) ;

NEMIVER_API int display_info (const common::UString &a_message) ;

NEMIVER_API int display_warning (const common::UString &a_message) ;

NEMIVER_API int display_error (const common::UString &a_message) ;

NEMIVER_API int ask_yes_no_question (const common::UString &a_message) ;

NEMIVER_API int ask_yes_no_cancel_question (const common::UString &a_message) ;

}//end namespace ui_utils
}//end namespace nemiver

#endif// __NEMIVER_UI_UTILS_H__

