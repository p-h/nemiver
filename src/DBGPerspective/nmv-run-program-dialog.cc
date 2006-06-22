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
#include <libglademm.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
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

    void on_browse_program_button_clicked ()
    {
        Gtk::FileChooserDialog file_chooser ("Choose a program",
                                             Gtk::FILE_CHOOSER_ACTION_OPEN) ;
        file_chooser.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
        file_chooser.add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
        file_chooser.set_select_multiple (false) ;

        int result = file_chooser.run () ;

        if (result != Gtk::RESPONSE_OK) {return;}

        Gtk::Entry *entry =
            env::get_widget_from_glade<Gtk::Entry> (glade, "programentry") ;
        entry->set_text (file_chooser.get_filename ()) ;
    }

    void on_browse_dir_button_clicked ()
    {
        Gtk::FileChooserDialog file_chooser
                                    ("Open file",
                                     Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) ;
        file_chooser.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
        file_chooser.add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
        file_chooser.set_select_multiple (false) ;

        int result = file_chooser.run () ;

        if (result != Gtk::RESPONSE_OK) {return;}

        Gtk::Entry *entry =
            env::get_widget_from_glade<Gtk::Entry> (glade, "workingdirentry") ;
        entry->set_text (file_chooser.get_filename ()) ;
    }

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
        init_widget () ;
    }

    void init_widget ()
    {
        dialog->hide () ;

        Gtk::Button *button =
            env::get_widget_from_glade<Gtk::Button>
                                            (glade, "browseprogrambutton") ;
        button->signal_clicked ().connect
            (sigc::mem_fun
                (*this,
                 &RunProgramDialog::Priv::on_browse_program_button_clicked)) ;

        button = env::get_widget_from_glade<Gtk::Button> (glade,
                                                          "browsedirbutton") ;
        button->signal_clicked ().connect
            (sigc::mem_fun
                (*this,
                 &RunProgramDialog::Priv::on_browse_dir_button_clicked)) ;
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
    Gtk::Entry *entry = env::get_widget_from_glade<Gtk::Entry> (m_priv->glade,
                                                                "programentry") ;
    return entry->get_text () ;
}

void
RunProgramDialog::program_name (const UString &a_name)
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::Entry *entry = env::get_widget_from_glade<Gtk::Entry> (m_priv->glade,
                                                                "programentry") ;
    entry->set_text (a_name) ;
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
    Gtk::Entry *entry =
        env::get_widget_from_glade<Gtk::Entry> (m_priv->glade,
                                                "workingdirentry");
    return entry->get_text () ;
}

void
RunProgramDialog::working_directory (const UString &a_dir)
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::Entry *entry =
        env::get_widget_from_glade<Gtk::Entry> (m_priv->glade,
                                                "workingdirentry");
    entry->set_text (a_dir) ;
}

}//end namespace nemiver

