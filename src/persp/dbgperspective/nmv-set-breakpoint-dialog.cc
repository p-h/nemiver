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
#include <glib/gi18n.h>
#include <libglademm.h>
#include <gtkmm/dialog.h>
#include "common/nmv-exception.h"
#include "common/nmv-env.h"
#include "common/nmv-ustring.h"
#include "nmv-set-breakpoint-dialog.h"
#include "nmv-ui-utils.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {
class SetBreakpointDialog::Priv {
    public:
    Gtk::Entry *entry_filename ;
    Gtk::Entry *entry_line;
    Gtk::Entry *entry_function;
    Gtk::RadioButton *radio_source_location;
    Gtk::RadioButton *radio_function_name;
    Gtk::Button *okbutton ;

public:
    Priv (const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        entry_filename (0),
        entry_line (0),
        entry_function (0),
        radio_source_location (0),
        radio_function_name (0),
        okbutton (0)
    {

        okbutton =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade, "okbutton") ;
        THROW_IF_FAIL (okbutton) ;
        okbutton->set_sensitive (false) ;

        entry_filename =
            ui_utils::get_widget_from_glade<Gtk::Entry>
                (a_glade, "entry_filename") ;
        entry_filename->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_text_changed_signal)) ;

        entry_line =
            ui_utils::get_widget_from_glade<Gtk::Entry>
                (a_glade, "entry_line") ;
        entry_line->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_text_changed_signal)) ;

        entry_function =
            ui_utils::get_widget_from_glade<Gtk::Entry>
                (a_glade, "entry_function") ;
        entry_function->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_text_changed_signal)) ;

        radio_source_location =
            ui_utils::get_widget_from_glade<Gtk::RadioButton>
                (a_glade, "radiobutton_source_location") ;
        radio_source_location->signal_clicked ().connect (sigc::mem_fun
                (*this, &Priv::on_radiobutton_changed)) ;

        radio_function_name =
            ui_utils::get_widget_from_glade<Gtk::RadioButton>
                (a_glade, "radiobutton_function_name") ;
        radio_function_name->signal_clicked ().connect (sigc::mem_fun
                (*this, &Priv::on_radiobutton_changed)) ;

        // set the 'source location' mode active by default
        mode (MODE_SOURCE_LOCATION);
    }

    void update_ok_button_sensitivity ()
    {
        THROW_IF_FAIL (entry_filename) ;
        THROW_IF_FAIL (entry_line) ;

        if (mode () == MODE_SOURCE_LOCATION) {
            // make sure there's something in both entries
            if (!entry_filename->get_text ().empty () &&
                    !entry_line->get_text ().empty () &&
                    // make sure the line number field is a valid number
                    atoi(entry_line->get_text ().c_str ())) {
                okbutton->set_sensitive (true) ;
            } else {
                okbutton->set_sensitive (false) ;
            }
        } else {
            if (!entry_function->get_text ().empty ()) {
                okbutton->set_sensitive (true) ;
            } else {
                okbutton->set_sensitive (false) ;
            }
        }
    }

    void on_text_changed_signal ()
    {
        NEMIVER_TRY
        update_ok_button_sensitivity ();
        NEMIVER_CATCH
    }

    void on_radiobutton_changed ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        THROW_IF_FAIL (entry_filename) ;
        THROW_IF_FAIL (entry_line) ;
        THROW_IF_FAIL (entry_function) ;

        if (mode () == MODE_SOURCE_LOCATION) {
            LOG_DD ("Setting Sensitivity for SOURCE_LOCATION");
            entry_function->set_sensitive (false);
            entry_filename->set_sensitive (true);
            entry_line->set_sensitive (true);
        } else {
            LOG_DD ("Setting Sensitivity for FUNCTION_NAME");
            entry_function->set_sensitive (true);
            entry_filename->set_sensitive (false);
            entry_line->set_sensitive (false);
        }
        update_ok_button_sensitivity ();
        NEMIVER_CATCH
    }

    void mode (SetBreakpointDialog::Mode a_mode)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        THROW_IF_FAIL (radio_source_location) ;
        THROW_IF_FAIL (radio_function_name) ;

        switch (a_mode)
        {
            case MODE_SOURCE_LOCATION:
                LOG_DD ("Changing Mode to SOURCE_LOCATION");
                radio_source_location->set_active ();
                break;
            case MODE_FUNCTION_NAME:
                LOG_DD ("Changing Mode to FUNCTION_NAME");
                radio_function_name->set_active ();
                break;
            default:
                g_assert_not_reached ();
        }

        NEMIVER_CATCH
    }

    SetBreakpointDialog::Mode mode () const
    {
        NEMIVER_TRY

        THROW_IF_FAIL (radio_source_location) ;
        THROW_IF_FAIL (radio_function_name) ;

        NEMIVER_CATCH

        if (radio_source_location->get_active ()) {
            return MODE_SOURCE_LOCATION;
        } else {
            return MODE_FUNCTION_NAME;
        }
    }
};//end class SetBreakpointDialog::Priv

SetBreakpointDialog::SetBreakpointDialog (const UString &a_root_path) :
    Dialog (a_root_path, "setbreakpoint.glade", "setbreakpointdialog")
{
    m_priv.reset (new Priv (glade ())) ;
}

SetBreakpointDialog::~SetBreakpointDialog ()
{
}

UString
SetBreakpointDialog::file_name () const
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->entry_filename) ;

    NEMIVER_CATCH
    return m_priv->entry_filename->get_text () ;
}

void
SetBreakpointDialog::file_name (const UString &a_name)
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->entry_filename) ;
    m_priv->entry_filename->set_text (a_name) ;

    NEMIVER_CATCH
}

int
SetBreakpointDialog::line_number () const
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->entry_line) ;

    NEMIVER_CATCH
    return atoi (m_priv->entry_line->get_text ().c_str ()) ;
}

void
SetBreakpointDialog::line_number (int a_line)
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->entry_line) ;
    m_priv->entry_line->set_text (UString::from_int(a_line)) ;

    NEMIVER_CATCH
}

UString
SetBreakpointDialog::function () const
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->entry_function) ;

    NEMIVER_CATCH
    return m_priv->entry_function->get_text () ;
}

void
SetBreakpointDialog::function (const UString &a_name)
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->entry_function) ;
    m_priv->entry_function->set_text (a_name) ;

    NEMIVER_CATCH
}

SetBreakpointDialog::Mode
SetBreakpointDialog::mode () const
{
    NEMIVER_TRY
    THROW_IF_FAIL (m_priv) ;
    NEMIVER_CATCH

    return m_priv->mode ();
}


void
SetBreakpointDialog::mode (Mode a_mode)
{
    NEMIVER_TRY

    THROW_IF_FAIL (m_priv) ;
    m_priv->mode (a_mode);

    NEMIVER_CATCH
}

}//end namespace nemiver

