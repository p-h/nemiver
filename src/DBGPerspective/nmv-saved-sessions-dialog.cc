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
#include "nmv-saved-sessions-dialog.h"
#include "nmv-env.h"
#include "nmv-ustring.h"
#include "nmv-ui-utils.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

SavedSessionsDialog::SavedSessionsDialog (const UString &a_root_path, ISessMgr *a_sesssion_manager) :
    Dialog(a_root_path, "savedsessionsdialog.glade", "savedsessionsdialog"),
    m_model(Gtk::ListStore::create(m_session_columns))
{
    THROW_IF_FAIL (glade) ;
    THROW_IF_FAIL(a_sesssion_manager != NULL);
    list<ISessMgr::Session> sessions = a_sesssion_manager->sessions ();
    m_treeview_sessions = ui_utils::get_widget_from_glade<Gtk::TreeView> (glade,
            "treeview_sessions") ;
    m_treeview_sessions->set_model (m_model);
    m_treeview_sessions->append_column (_("Session"), m_session_columns.name);
    //m_treeview_sessions->append_column (_("id"), m_session_columns.id);
    for (list<ISessMgr::Session>::iterator iter = sessions.begin();
            iter != sessions.end(); ++iter)
    {
        Gtk::TreeModel::iterator treeiter = m_model->append ();
        (*treeiter)[m_session_columns.id] = iter->session_id ();
        (*treeiter)[m_session_columns.name] = iter->properties ()["sessionname"];
        (*treeiter)[m_session_columns.session] = *iter;
    }
}

SavedSessionsDialog::~SavedSessionsDialog ()
{
}

ISessMgr::Session
SavedSessionsDialog::session () const
{
    Glib::RefPtr<Gtk::TreeSelection> selection = m_treeview_sessions->get_selection ();
    Gtk::TreeModel::iterator iter = selection->get_selected ();
    return (*iter)[m_session_columns.session];
}

}//end namespace nemiver

