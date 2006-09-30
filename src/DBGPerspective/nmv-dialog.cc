//Author: Jonathon Jongsma
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

#include <vector>
#include <libglademm.h>
#include <gtkmm/dialog.h>
#include "nmv-exception.h"
#include "nmv-dialog.h"
#include "nmv-env.h"
#include "nmv-ustring.h"
#include "nmv-ui-utils.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

Dialog::Dialog (const UString &a_resource_root_path,
                const UString &a_glade_filename,
                const UString &a_widget_name)
{
        vector<string> path_elems ;
        path_elems.push_back (Glib::locale_from_utf8 (a_resource_root_path)) ;
        path_elems.push_back ("glade");
        path_elems.push_back (a_glade_filename);
        string glade_path = Glib::build_filename (path_elems) ;
        if (!Glib::file_test (glade_path, Glib::FILE_TEST_IS_REGULAR)) {
            THROW (UString ("could not find file ") + glade_path) ;
        }
        m_glade = Gnome::Glade::Xml::create (glade_path) ;
        THROW_IF_FAIL (m_glade) ;
        m_dialog = ui_utils::get_widget_from_glade<Gtk::Dialog> (m_glade,
                                                                 a_widget_name) ;
        m_dialog->hide () ;
}

Dialog::~Dialog ()
{
}

int
Dialog::run ()
{
    THROW_IF_FAIL (m_dialog) ;
    return m_dialog->run () ;
}

}//end namespace nemiver

