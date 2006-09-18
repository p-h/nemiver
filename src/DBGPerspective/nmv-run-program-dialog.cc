//Author: Dodji Seketeli
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
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/stock.h>
#include "nmv-exception.h"
#include "nmv-run-program-dialog.h"
#include "nmv-env.h"
#include "nmv-ustring.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

struct RunProgramDialog::Priv {
    UString root_path ;
    UString program_name ;
    UString arguments ;
    UString working_directory ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;
    SafePtr<Gtk::Dialog> dialog ;

    void load_glade_file ()
    {
        vector<string> path_elems ;
        path_elems.push_back (Glib::locale_from_utf8 (root_path)) ;
        path_elems.push_back ("glade");
        path_elems.push_back ("runprogramdialog.glade");
        string glade_path = Glib::build_filename (path_elems) ;
        if (!Glib::file_test (glade_path, Glib::FILE_TEST_IS_REGULAR)) {
            THROW (UString ("could not find file ") + glade_path) ;
        }
        glade = Gnome::Glade::Xml::create (glade_path) ;
        THROW_IF_FAIL (glade) ;
        dialog = env::get_widget_from_glade<Gtk::Dialog> (glade,
                                                          "runprogramdialog") ;
        dialog->hide () ;
    }

};//end struct RunProgramDialog::Priv

RunProgramDialog::RunProgramDialog ()
{
    m_priv = new RunProgramDialog::Priv ();
    m_priv->load_glade_file () ;
}

RunProgramDialog::RunProgramDialog (const UString &a_root_path)
{
    m_priv = new RunProgramDialog::Priv ();
    m_priv->root_path = a_root_path ;
    m_priv->load_glade_file () ;
    working_directory (Glib::get_current_dir ()) ;
}

RunProgramDialog::~RunProgramDialog ()
{
    m_priv = NULL ;
}

int
RunProgramDialog::run ()
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->dialog->run () ;
}

UString
RunProgramDialog::program_name () const
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *fcb = env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                                "filechooserbutton_program") ;
    return fcb->get_filename () ;
}

void
RunProgramDialog::program_name (const UString &a_name)
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *fcb = env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                                "filechooserbutton_program") ;
    fcb->set_filename (a_name) ;
}

UString
RunProgramDialog::arguments () const
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::Entry *entry = env::get_widget_from_glade<Gtk::Entry> (m_priv->glade,
                                                                "argumentsentry");
    return entry->get_text () ;
}

void
RunProgramDialog::arguments (const UString &a_args)
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::Entry *entry = env::get_widget_from_glade<Gtk::Entry> (m_priv->glade,
                                                                "argumentsentry");
    entry->set_text (a_args) ;
}

UString
RunProgramDialog::working_directory () const
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *fcb =
        env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                "filechooserbutton_workingdir");
    return fcb->get_filename () ;
}

void
RunProgramDialog::working_directory (const UString &a_dir)
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *fcb =
        env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                "filechooserbutton_workingdir");
    fcb->set_filename (a_dir) ;
}

}//end namespace nemiver

