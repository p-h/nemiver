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
#include <glib/gi18n.h>
#include <libglademm.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/stock.h>
#include "nmv-exception.h"
#include "nmv-locate-file-dialog.h"
#include "nmv-env.h"
#include "nmv-ustring.h"
#include "nmv-ui-utils.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {
class LocateFileDialog::Priv {
public:
    Gtk::FileChooserButton *fcbutton_location ;
    Gtk::Label *label_filename;
    Gtk::Button *okbutton ;
    Priv (const Glib::RefPtr<Gnome::Glade::Xml> &a_glade, const UString& a_filename) :
        fcbutton_location (0),
        okbutton (0),
        label_filename(0)
    {

        okbutton =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade, "okbutton") ;
        THROW_IF_FAIL (okbutton) ;
        okbutton->set_sensitive (false) ;

        fcbutton_location =
            ui_utils::get_widget_from_glade<Gtk::FileChooserButton>
                (a_glade, "filechooserbutton_location") ;
        fcbutton_location->signal_selection_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_file_selection_changed_signal)) ;

        label_filename =
            ui_utils::get_widget_from_glade<Gtk::Label> (a_glade, "label_filename") ;
        THROW_IF_FAIL (label_filename) ;
        label_filename->set_text("Cannot find file '<b>" + a_filename + "</b>'.\n" +
                "Please specify the location of this file:");
        label_filename->set_use_markup ();
    }

    void on_file_selection_changed_signal ()
    {
        NEMIVER_TRY

        THROW_IF_FAIL (fcbutton_location) ;

        if (Glib::file_test (fcbutton_location->get_filename (),
                             Glib::FILE_TEST_IS_REGULAR)) {
            okbutton->set_sensitive (true) ;
        } else {
            okbutton->set_sensitive (false) ;
        }
        NEMIVER_CATCH
    }
};//end class LocateFileDialog::Priv

LocateFileDialog::LocateFileDialog (const UString &a_root_path, const UString &a_file) :
    Dialog (a_root_path, "locatefiledialog.glade", "locatefiledialog")
{
    m_priv.reset (new Priv (glade (), a_file)) ;
}

LocateFileDialog::~LocateFileDialog ()
{
}

UString
LocateFileDialog::file_location () const
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->fcbutton_location) ;
    return m_priv->fcbutton_location->get_filename () ;

    NEMIVER_CATCH
}

void
LocateFileDialog::file_location (const UString &a_location)
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv->fcbutton_location) ;
    m_priv->fcbutton_location->set_filename (a_location) ;
    NEMIVER_CATCH
}

}//end namespace nemiver

