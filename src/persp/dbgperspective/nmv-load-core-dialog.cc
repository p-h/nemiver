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
#include "nmv-load-core-dialog.h"
#include "nmv-env.h"
#include "nmv-ustring.h"
#include "nmv-ui-utils.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {
class LoadCoreDialog::Priv {
    public:
    Gtk::FileChooserButton *fcbutton_core_file ;
    Gtk::FileChooserButton *fcbutton_executable;
    Gtk::Button *okbutton ;

public:
    Priv (const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        fcbutton_core_file (0),
        fcbutton_executable (0),
        okbutton (0)
    {

        okbutton =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade, "okbutton") ;
        THROW_IF_FAIL (okbutton) ;
        okbutton->set_sensitive (false) ;

        fcbutton_executable =
            ui_utils::get_widget_from_glade<Gtk::FileChooserButton>
                (a_glade, "filechooserbutton_executable") ;
        fcbutton_executable->signal_selection_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_file_selection_changed_signal)) ;

        fcbutton_core_file =
            ui_utils::get_widget_from_glade<Gtk::FileChooserButton>
                (a_glade, "filechooserbutton_corefile") ;
        fcbutton_core_file->signal_selection_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_file_selection_changed_signal)) ;
    }

    void on_file_selection_changed_signal ()
    {
        NEMIVER_TRY

        THROW_IF_FAIL (fcbutton_executable) ;
        THROW_IF_FAIL (fcbutton_core_file) ;

        if (Glib::file_test (fcbutton_executable->get_filename (),
                             Glib::FILE_TEST_IS_EXECUTABLE)) {
            if (Glib::file_test (fcbutton_core_file->get_filename (),
                                 Glib::FILE_TEST_IS_REGULAR)) {
                okbutton->set_sensitive (true) ;
            } else {
                okbutton->set_sensitive (false) ;
            }
        } else {
            okbutton->set_sensitive (false) ;
        }
        NEMIVER_CATCH
    }
};//end class LoadCoreDialog::Priv

LoadCoreDialog::LoadCoreDialog (const UString &a_root_path) :
    Dialog (a_root_path, "loadcoredialog.glade", "loadcoredialog")
{
    m_priv.reset (new Priv (glade ())) ;
}

LoadCoreDialog::~LoadCoreDialog ()
{
}

UString
LoadCoreDialog::program_name () const
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->fcbutton_executable) ;

    NEMIVER_CATCH
    return m_priv->fcbutton_executable->get_filename () ;
}

void
LoadCoreDialog::program_name (const UString &a_name)
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->fcbutton_executable) ;
    m_priv->fcbutton_executable->set_filename (a_name) ;

    NEMIVER_CATCH
}

UString
LoadCoreDialog::core_file () const
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->fcbutton_core_file) ;

    NEMIVER_CATCH
    return m_priv->fcbutton_core_file->get_filename () ;
}

void
LoadCoreDialog::core_file (const UString &a_dir)
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->fcbutton_core_file) ;
    m_priv->fcbutton_core_file->set_filename (a_dir) ;

    NEMIVER_CATCH
}

}//end namespace nemiver

