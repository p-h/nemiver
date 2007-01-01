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

#include <glib/gi18n.h>
#include <libglademm.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/filechooserwidget.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include "nmv-exception.h"
#include "nmv-open-file-dialog.h"
#include "nmv-ustring.h"
#include "nmv-ui-utils.h"
#include "nmv-file-list.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {
class OpenFileDialog::Priv {
    public:
    Gtk::VBox* vbox_file_list;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::RadioButton *radio_button_file_list, *radio_button_chooser;
    Gtk::FileChooserWidget file_chooser;
    FileList file_list;
    Gtk::Button *okbutton ;

public:
    Priv (const Glib::RefPtr<Gnome::Glade::Xml> &a_glade, IDebuggerSafePtr&
            a_debugger) :
        vbox_file_list (0),
        radio_button_file_list (0),
        radio_button_chooser (0),
        file_chooser(Gtk::FILE_CHOOSER_ACTION_OPEN),
        file_list(a_debugger),
        okbutton (0)
    {
        file_chooser.set_select_multiple (true);
        okbutton =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade, "okbutton") ;
        THROW_IF_FAIL (okbutton) ;
        vbox_file_list =
            ui_utils::get_widget_from_glade<Gtk::VBox> (a_glade, "vbox_file_list") ;
        THROW_IF_FAIL (vbox_file_list) ;
        radio_button_file_list =
            ui_utils::get_widget_from_glade<Gtk::RadioButton> (a_glade, "radiobutton_target") ;
        THROW_IF_FAIL (radio_button_file_list) ;
        radio_button_file_list->signal_toggled ().connect (sigc::mem_fun
                    (*this, &Priv::on_radio_button_toggled));
        radio_button_chooser =
            ui_utils::get_widget_from_glade<Gtk::RadioButton> (a_glade, "radiobutton_other") ;
        THROW_IF_FAIL (radio_button_chooser) ;
        radio_button_chooser->signal_toggled ().connect (sigc::mem_fun
                    (*this, &Priv::on_radio_button_toggled));

        Gtk::TreeView& treeview = dynamic_cast<Gtk::TreeView&>(file_list.widget ());
        treeview.get_selection ()->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_file_list_selection_changed_signal)) ;

        file_chooser.signal_selection_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_chooser_selection_changed_signal)) ;

        scrolled_window.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        scrolled_window.add(file_list.widget ());

        on_radio_button_toggled ();
    }

    void on_radio_button_toggled()
    {
        NEMIVER_TRY

        THROW_IF_FAIL (vbox_file_list);
        THROW_IF_FAIL (radio_button_file_list);
        THROW_IF_FAIL (radio_button_chooser);

        if (radio_button_file_list->get_active ())
        {
            LOG_DD("Target file list is active");
            // remove existing children of vbox_file_list
            vbox_file_list->children ().clear();
            vbox_file_list->pack_start (scrolled_window);
            scrolled_window.show_all ();
        }
        else if (radio_button_chooser->get_active ())
        {
            LOG_DD("file chooser is active");
            // remove existing children of vbox_file_list
            vbox_file_list->children ().clear();
            vbox_file_list->pack_start (file_chooser);
            file_chooser.show_all ();
        }
        NEMIVER_CATCH
    }

    void on_file_list_selection_changed_signal ()
    {
        NEMIVER_TRY

        THROW_IF_FAIL(okbutton);

        if (!file_list.get_filenames ().empty ()) {
            okbutton->set_sensitive (true) ;
        } else {
            okbutton->set_sensitive (false) ;
        }
        NEMIVER_CATCH
    }

    void on_chooser_selection_changed_signal ()
    {
        NEMIVER_TRY

        THROW_IF_FAIL(okbutton);

        if (!file_chooser.get_filenames ().empty ()) {
            okbutton->set_sensitive (true) ;
        } else {
            okbutton->set_sensitive (false) ;
        }
        NEMIVER_CATCH
    }

    list<UString> get_filenames()
    {
        THROW_IF_FAIL(radio_button_file_list);
        THROW_IF_FAIL(radio_button_chooser);

        if (radio_button_file_list->get_active ())
        {
            return file_list.get_filenames ();
        }
        else if (radio_button_chooser->get_active ())
        {
            return file_chooser.get_filenames ();
        }
        else
        {
            // empty list
            return list<UString>();
        }
    }
};//end class OpenFileDialog::Priv

OpenFileDialog::OpenFileDialog (const UString &a_root_path, IDebuggerSafePtr&
            a_debugger) :
    Dialog (a_root_path, "openfiledialog.glade", "dialog_open_source_file")
{
    m_priv.reset (new Priv (glade (), a_debugger)) ;
}

OpenFileDialog::~OpenFileDialog ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

list<UString>
OpenFileDialog::get_filenames () const
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;

    NEMIVER_CATCH
    return m_priv->get_filenames () ;
}

}//end namespace nemiver

