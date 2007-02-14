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
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */

#include <vector>
#include <libglademm.h>
#include <gtkmm/dialog.h>
#include "common/nmv-exception.h"
#include "common/nmv-env.h"
#include "common/nmv-ustring.h"
#include "nmv-ui-utils.h"
#include "nmv-dialog.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {
class Dialog::Priv {
    Priv () ;
public:

    SafePtr<Gtk::Dialog> dialog ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;

    Priv (const UString &a_resource_root_path,
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
        glade = Gnome::Glade::Xml::create (glade_path) ;
        THROW_IF_FAIL (glade) ;
        dialog.reset
            (ui_utils::get_widget_from_glade<Gtk::Dialog> (glade,
                                                           a_widget_name)) ;
        THROW_IF_FAIL (dialog) ;
        dialog->hide () ;
    }
};//end struct Dialog::Priv

Dialog::Dialog (const UString &a_resource_root_path,
                const UString &a_glade_filename,
                const UString &a_widget_name)
{
    m_priv.reset (new Priv (a_resource_root_path,
                            a_glade_filename,
                            a_widget_name)) ;
}

Gtk::Dialog&
Dialog::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->dialog) ;
    return *m_priv->dialog ;
}

const Glib::RefPtr<Gnome::Glade::Xml>
Dialog::glade () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->glade) ;
    return m_priv->glade ;
}

Dialog::~Dialog ()
{
}

int
Dialog::run ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->dialog) ;
    return m_priv->dialog->run () ;
}

void
Dialog::hide ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->dialog) ;
    return m_priv->dialog->hide () ;
}

}//end namespace nemiver

