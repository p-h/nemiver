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
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/stock.h>
#include "nmv-exception.h"
#include "nmv-run-program-dialog.h"
#include "nmv-env.h"
#include "nmv-ustring.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

RunProgramDialog::RunProgramDialog (const UString &a_root_path) :
    Dialog(a_root_path, "runprogramdialog.glade", "runprogramdialog")
{
    working_directory (Glib::get_current_dir ()) ;
}

RunProgramDialog::~RunProgramDialog ()
{
}

UString
RunProgramDialog::program_name () const
{
    THROW_IF_FAIL (glade) ;
    Gtk::FileChooserButton *fcb = env::get_widget_from_glade<Gtk::FileChooserButton> (glade,
                                                                "filechooserbutton_program") ;
    return fcb->get_filename () ;
}

void
RunProgramDialog::program_name (const UString &a_name)
{
    THROW_IF_FAIL (glade) ;
    Gtk::FileChooserButton *fcb = env::get_widget_from_glade<Gtk::FileChooserButton> (glade,
                                                                "filechooserbutton_program") ;
    fcb->set_filename (a_name) ;
}

UString
RunProgramDialog::arguments () const
{
    THROW_IF_FAIL (glade) ;
    Gtk::Entry *entry = env::get_widget_from_glade<Gtk::Entry> (glade,
                                                                "argumentsentry");
    return entry->get_text () ;
}

void
RunProgramDialog::arguments (const UString &a_args)
{
    THROW_IF_FAIL (glade) ;
    Gtk::Entry *entry = env::get_widget_from_glade<Gtk::Entry> (glade,
                                                                "argumentsentry");
    entry->set_text (a_args) ;
}

UString
RunProgramDialog::working_directory () const
{
    THROW_IF_FAIL (glade) ;
    Gtk::FileChooserButton *fcb =
        env::get_widget_from_glade<Gtk::FileChooserButton> (glade,
                                                "filechooserbutton_workingdir");
    return fcb->get_filename () ;
}

void
RunProgramDialog::working_directory (const UString &a_dir)
{
    THROW_IF_FAIL (glade) ;
    Gtk::FileChooserButton *fcb =
        env::get_widget_from_glade<Gtk::FileChooserButton> (glade,
                                                "filechooserbutton_workingdir");
    fcb->set_filename (a_dir) ;
}

}//end namespace nemiver

