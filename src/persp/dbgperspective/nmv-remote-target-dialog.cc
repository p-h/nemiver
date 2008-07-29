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
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include <glib/gi18n.h>
#include "common/nmv-env.h"
#include "nmv-remote-target-dialog.h"
#include "nmv-ui-utils.h"


NEMIVER_BEGIN_NAMESPACE (nemiver)

using namespace ui_utils;

struct RemoteTargetDialog::Priv {
    friend class RemoteTargetDialog;
    Gtk::Dialog &dialog;
    Glib::RefPtr<Gnome::Glade::Xml> glade;
    mutable UString executable_path;
    mutable UString solib_prefix_path;
    mutable UString server_address;
    mutable UString serial_port_name;
    enum RemoteTargetDialog::ConnectionType connection_type;

    Priv ();

    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        dialog (a_dialog),
        glade (a_glade)
    {
        init_members ();
        init_from_glade ();
    }

    //*******************
    //<signal handlers>
    //*******************
    void on_radio_button_toggled_signal ()
    {
        NEMIVER_TRY

        Gtk::RadioButton *radio =
            get_widget_from_glade<Gtk::RadioButton> (glade, "tcpradiobutton");
        Gtk::Widget *tcp_connection_container =
            get_widget_from_glade<Gtk::Widget> (glade,
                                                "tcpconnectioncontainer");
        Gtk::Widget *serial_connection_container =
            get_widget_from_glade<Gtk::Widget> (glade,
                                                "serialconnectioncontainer");
        if (radio->get_active ()) {
            connection_type = RemoteTargetDialog::TCP_CONNECTION_TYPE;
            tcp_connection_container->set_sensitive (true);
            serial_connection_container->set_sensitive (false);
        } else {
            connection_type = RemoteTargetDialog::SERIAL_CONNECTION_TYPE;
            tcp_connection_container->set_sensitive (false);
            serial_connection_container->set_sensitive (true);
        }

        NEMIVER_CATCH
    }

    void on_selection_changed_signal ()
    {
        NEMIVER_TRY

        Gtk::Button *button =
            get_widget_from_glade<Gtk::Button> (glade, "okbutton");
        if (can_enable_ok_button ()) {
            button->set_sensitive (true);
        } else {
            button->set_sensitive (false);
        }

        NEMIVER_CATCH
    }

    //*******************
    //</signal handlers>
    //*******************

    //*****************
    //<init functions>
    //*****************
    void init_members ()
    {
        connection_type = RemoteTargetDialog::TCP_CONNECTION_TYPE;
    }

    void init_from_glade ()
    {
        Gtk::RadioButton *radio =
            get_widget_from_glade<Gtk::RadioButton> (glade, "tcpradiobutton");
        radio->signal_toggled ().connect (sigc::mem_fun
                (*this, &Priv::on_radio_button_toggled_signal));
        radio->set_active (true);
        on_radio_button_toggled_signal ();//it does not get called otherwise

        Gtk::FileChooser *chooser =
            get_widget_from_glade<Gtk::FileChooser> (glade,
                                                     "execfilechooserbutton");
        chooser->set_show_hidden (true);
        chooser->signal_selection_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_selection_changed_signal));

        chooser =
        get_widget_from_glade<Gtk::FileChooser> (glade,
                                                 "solibprefixchooserbutton");
        chooser->set_show_hidden (true);
        chooser->set_action (Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

        set_solib_prefix_path (common::env::get_system_lib_dir ());

        Gtk::Entry* entry =
                get_widget_from_glade<Gtk::Entry> (glade, "addressentry");
        entry->signal_changed ().connect
                (sigc::mem_fun (*this, &Priv::on_selection_changed_signal));

        entry = get_widget_from_glade<Gtk::Entry> (glade, "portentry");
        entry->signal_changed ().connect
                (sigc::mem_fun (*this, &Priv::on_selection_changed_signal));

        entry = get_widget_from_glade<Gtk::Entry> (glade, "serialentry");
        entry->signal_changed ().connect (sigc::mem_fun
                            (*this, &Priv::on_selection_changed_signal));

        Gtk::Button *button =
            get_widget_from_glade<Gtk::Button> (glade, "okbutton");
        button->set_sensitive (false);
    }

    bool can_enable_ok_button () const
    {
        Gtk::FileChooserButton *chooser =
            get_widget_from_glade<Gtk::FileChooserButton>
                                            (glade, "execfilechooserbutton");
        Gtk::Entry *entry = 0;
        if (chooser->get_filename ().empty ())
            return false;
        if (connection_type == RemoteTargetDialog::TCP_CONNECTION_TYPE) {
            entry = get_widget_from_glade<Gtk::Entry> (glade, "portentry");
            if (entry->get_text ().empty ())
                return false;
        } else if (connection_type ==
                    RemoteTargetDialog::SERIAL_CONNECTION_TYPE) {
            entry = get_widget_from_glade<Gtk::Entry> (glade, "serialentry");
            if (entry->get_text ().empty ())
                return false;
        }
        return true;
    }
    //*****************
    //</init functions>
    //*****************

    const UString& get_executable_path () const
    {
        Gtk::FileChooserButton *chooser =
            get_widget_from_glade<Gtk::FileChooserButton>
                                            (glade, "execfilechooserbutton");
        executable_path = chooser->get_filename ();
        return executable_path;
    }

    void set_executable_path (const UString &a_path)
    {
        Gtk::FileChooserButton *chooser =
            get_widget_from_glade<Gtk::FileChooserButton>
                                            (glade, "execfilechooserbutton");
        chooser->set_filename (a_path);
        executable_path = a_path;
    }

    const UString& get_solib_prefix_path () const
    {
        Gtk::FileChooserButton *chooser =
            get_widget_from_glade<Gtk::FileChooserButton>
                                        (glade, "solibprefixchooserbutton");
        solib_prefix_path = chooser->get_filename ();
        return solib_prefix_path;
    }

    void set_solib_prefix_path (const UString &a_path)
    {
        Gtk::FileChooserButton *chooser =
            get_widget_from_glade<Gtk::FileChooserButton>
                                        (glade, "solibprefixchooserbutton");
        chooser->set_filename (a_path);
        solib_prefix_path = a_path;
    }

    void set_connection_type (RemoteTargetDialog::ConnectionType &a_type)
    {
        Gtk::RadioButton *radio =
            get_widget_from_glade<Gtk::RadioButton> (glade, "tcpradiobutton");
        if (a_type == RemoteTargetDialog::TCP_CONNECTION_TYPE) {
            radio->set_active (true);
        } else {
            radio->set_active (false);
        }
    }

    const UString& get_server_address () const
    {
        Gtk::Entry *entry = get_widget_from_glade<Gtk::Entry>(glade,
                                                              "addressentry");
        server_address = entry->get_text ();
        return server_address;
    }

    void set_server_address (const UString &a_address)
    {
        Gtk::Entry *entry = get_widget_from_glade<Gtk::Entry>(glade,
                                                              "addressentry");
        entry->set_text (a_address);
    }

    int get_server_port () const
    {
        Gtk::Entry *entry = get_widget_from_glade<Gtk::Entry> (glade,
                                                               "portentry");
        return atoi (entry->get_text ().c_str ());
    }

    void set_server_port (int a_port)
    {
        Gtk::Entry *entry = get_widget_from_glade<Gtk::Entry> (glade,
                                                               "portentry");
        entry->set_text (UString::from_int (a_port));
    }

    const UString& get_serial_port_name () const
    {
        Gtk::Entry *entry = get_widget_from_glade<Gtk::Entry> (glade,
                                                               "serialentry");
        serial_port_name = entry->get_text ();
        return serial_port_name;
    }

    void set_serial_port_name (const UString &a_name)
    {
        Gtk::Entry *entry = get_widget_from_glade<Gtk::Entry> (glade,
                                                               "serialentry");
        entry->set_text (a_name);
    }

};//end RemoteTargetDialog::Priv

RemoteTargetDialog::RemoteTargetDialog (const UString &a_root_path) :
    Dialog (a_root_path,
            "remotetargetdialog.glade",
            "remotetargetdialog")
{
    m_priv.reset (new Priv (widget (), glade ()));
    THROW_IF_FAIL (m_priv);
}

RemoteTargetDialog::~RemoteTargetDialog ()
{
    LOG_D ("destroyed", "destructor-domain");
}

const UString&
RemoteTargetDialog::get_executable_path () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_executable_path ();
}

void
RemoteTargetDialog::set_executable_path (const UString &a_path)
{
    THROW_IF_FAIL (m_priv);
    m_priv->set_executable_path (a_path);
}

const UString&
RemoteTargetDialog::get_solib_prefix_path () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_solib_prefix_path ();
}

void
RemoteTargetDialog::set_solib_prefix_path (const UString &a_path)
{
    THROW_IF_FAIL (m_priv);
    m_priv->set_solib_prefix_path (a_path);
}

RemoteTargetDialog::ConnectionType
RemoteTargetDialog::get_connection_type ()
{
    THROW_IF_FAIL (m_priv);
    return m_priv->connection_type;
}

void
RemoteTargetDialog::set_connection_type (ConnectionType a_type)
{
    THROW_IF_FAIL (m_priv);
    m_priv->set_connection_type (a_type);
}

const UString&
RemoteTargetDialog::get_server_address () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_server_address ();
}

void
RemoteTargetDialog::set_server_address (const UString &a_address)
{
    THROW_IF_FAIL (m_priv);
    m_priv->set_server_address (a_address);
}

int
RemoteTargetDialog::get_server_port () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_server_port ();
}

void
RemoteTargetDialog::set_server_port (int a_port)
{
    THROW_IF_FAIL (m_priv);
    m_priv->set_server_port (a_port);
}

const UString&
RemoteTargetDialog::get_serial_port_name () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_serial_port_name ();
}

void
RemoteTargetDialog::set_serial_port_name (const UString &a_serial)
{
    THROW_IF_FAIL (m_priv);
    return m_priv->set_serial_port_name (a_serial);
}

NEMIVER_END_NAMESPACE (nemiver)
