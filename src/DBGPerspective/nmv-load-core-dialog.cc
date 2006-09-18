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

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

struct LoadCoreDialog::Priv {
    UString root_path ;
    UString program_name ;
    UString arguments ;
    UString core_file ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;
    SafePtr<Gtk::Dialog> dialog ;

    void load_glade_file ()
    {
        vector<string> path_elems ;
        path_elems.push_back (Glib::locale_from_utf8 (root_path)) ;
        path_elems.push_back ("glade");
        path_elems.push_back ("loadcoredialog.glade");
        string glade_path = Glib::build_filename (path_elems) ;
        if (!Glib::file_test (glade_path, Glib::FILE_TEST_IS_REGULAR)) {
            THROW (UString ("could not find file ") + glade_path) ;
        }
        glade = Gnome::Glade::Xml::create (glade_path) ;
        THROW_IF_FAIL (glade) ;
        dialog = env::get_widget_from_glade<Gtk::Dialog> (glade,
                                                          "loadcoredialog") ;
        dialog->hide () ;
    }

};//end struct LoadCoreDialog::Priv

LoadCoreDialog::LoadCoreDialog ()
{
    m_priv = new LoadCoreDialog::Priv ();
    m_priv->load_glade_file () ;
}

LoadCoreDialog::LoadCoreDialog (const UString &a_root_path)
{
    m_priv = new LoadCoreDialog::Priv ();
    m_priv->root_path = a_root_path ;
    m_priv->load_glade_file () ;
    core_file (Glib::get_current_dir ()) ;
}

LoadCoreDialog::~LoadCoreDialog ()
{
    m_priv = NULL ;
}

int
LoadCoreDialog::run ()
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->dialog->run () ;
}

UString
LoadCoreDialog::program_name () const
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *chooser = env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                                "filechooserbutton_executable") ;
    return chooser->get_filename () ;
}

void
LoadCoreDialog::program_name (const UString &a_name)
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *chooser = env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                                "filechooserbutton_executable") ;
    chooser->set_filename (a_name) ;
}

UString
LoadCoreDialog::core_file () const
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *chooser =
        env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                "filechooserbutton_corefile");
    return chooser->get_filename () ;
}

void
LoadCoreDialog::core_file (const UString &a_dir)
{
    THROW_IF_FAIL (m_priv) ;
    Gtk::FileChooserButton *chooser =
        env::get_widget_from_glade<Gtk::FileChooserButton> (m_priv->glade,
                                                "filechooserbutton_corefile");
    chooser->set_filename (a_dir) ;
}

}//end namespace nemiver

