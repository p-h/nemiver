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
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/stock.h>
#include "common/nmv-exception.h"
#include "common/nmv-env.h"
#include "common/nmv-ustring.h"
#include "nmv-saved-sessions-dialog.h"
#include "nmv-ui-utils.h"

using namespace std;
using namespace nemiver::common;

namespace nemiver {

struct SessionModelColumns : public Gtk::TreeModel::ColumnRecord
{
    // I tried using UString here, but it didn't want to compile... jmj
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<gint64> id;
    Gtk::TreeModelColumn<ISessMgr::Session> session;
    SessionModelColumns() { add (name); add (id); add (session); }
};

class SavedSessionsDialog::Priv {
public:
    SafePtr<Gtk::TreeView> treeview_sessions;
    Gtk::Button *okbutton;
    SessionModelColumns session_columns;
    Glib::RefPtr<Gtk::ListStore> model;
    Gtk::Dialog &dialog;
    Glib::RefPtr<Gnome::Glade::Xml> glade;

private:
    Priv ();

public:
    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        okbutton (0),
        model(Gtk::ListStore::create (session_columns)),
        dialog (a_dialog),
        glade (a_glade)
    {
    }

    void init (ISessMgr *a_session_manager)
    {
        okbutton =
            ui_utils::get_widget_from_glade<Gtk::Button> (glade, "okbutton1");
        treeview_sessions.reset
            (ui_utils::get_widget_from_glade<Gtk::TreeView>
                                            (glade, "treeview_sessions"));
        okbutton->set_sensitive (false);
        THROW_IF_FAIL (a_session_manager);
        list<ISessMgr::Session> sessions = a_session_manager->sessions ();
        THROW_IF_FAIL (model);
        for (list<ISessMgr::Session>::iterator iter = sessions.begin();
                iter != sessions.end(); ++iter)
        {
            Gtk::TreeModel::iterator treeiter = model->append ();
            (*treeiter)[session_columns.id] = iter->session_id ();
            (*treeiter)[session_columns.name] =
                                        iter->properties ()["sessionname"];
            (*treeiter)[session_columns.session] = *iter;
        }

        THROW_IF_FAIL (treeview_sessions);
        treeview_sessions->set_model (model);
        treeview_sessions->append_column (_("ID"), session_columns.id);
        treeview_sessions->append_column (_("Session"), session_columns.name);

        // update the sensitivity of the OK
        // button when the selection is changed
        treeview_sessions->get_selection ()->signal_changed ().connect
            (sigc::mem_fun(*this,
                           &SavedSessionsDialog::Priv::on_selection_changed));

        treeview_sessions->signal_row_activated ().connect
            (sigc::mem_fun(*this,
                           &SavedSessionsDialog::Priv::on_row_activated));
    }

    void on_selection_changed ()
    {
        THROW_IF_FAIL (okbutton);
        okbutton->set_sensitive
            (treeview_sessions->get_selection ()->count_selected_rows ());
    }

    void on_row_activated (const Gtk::TreeModel::Path& a_path,
                           Gtk::TreeViewColumn* a_col)
    {
        if (a_path.get_depth () || a_col) {}
        dialog.activate_default();
    }
}; //end struct SavedSessionsDialog::Priv

SavedSessionsDialog::SavedSessionsDialog (const UString &a_root_path,
                                          ISessMgr *a_session_manager) :
    Dialog(a_root_path, "savedsessionsdialog.glade", "savedsessionsdialog")
{
    m_priv.reset (new Priv (widget (), glade ()));
    THROW_IF_FAIL (m_priv);
    m_priv->init (a_session_manager);
}

SavedSessionsDialog::~SavedSessionsDialog ()
{
}

ISessMgr::Session
SavedSessionsDialog::session () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->treeview_sessions);

    Glib::RefPtr<Gtk::TreeSelection> selection =
        m_priv->treeview_sessions->get_selection ();
    Gtk::TreeModel::iterator iter = selection->get_selected ();
    if (iter)
    {
        return (*iter)[m_priv->session_columns.session];
    }
    // return an 'invalid' session if there is no selection
    return ISessMgr::Session();
}

}//end namespace nemiver

