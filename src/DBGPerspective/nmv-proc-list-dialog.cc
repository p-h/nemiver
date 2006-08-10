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

#include <iostream>
#include <list>
#include <libglademm.h>
#include <gtkmm.h>
#include "nmv-proc-list-dialog.h"
#include "nmv-proc-mgr.h"
#include "nmv-env.h"

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

struct Cols : public Gtk::TreeModel::ColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> pid ;
    Gtk::TreeModelColumn<Glib::ustring> user_name ;
    Gtk::TreeModelColumn<Glib::ustring> proc_name ;
    Gtk::TreeModelColumn<Glib::ustring> proc_args ;

    Cols ()
    {
        add (pid) ;
        add (user_name) ;
        add (proc_name) ;
        add (proc_args) ;
    }
};//end class Gtk::TreeModel

static Cols&
columns ()
{
    static Cols s_columns ;
    return s_columns ;
}

struct ProcListDialog::Priv {
    IProcMgr &proc_mgr ;
    UString root_path ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;
    SafePtr<Gtk::Dialog> dialog ;
    Gtk::TreeView *proclist_view ;

    Priv (const UString &a_root_path,
          IProcMgr &a_proc_mgr) :
        proc_mgr (a_proc_mgr),
        root_path (a_root_path)
    {}

    void load_glade_file ()
    {
        vector<string> path_elems ;
        path_elems.push_back (Glib::locale_from_utf8 (root_path)) ;
        path_elems.push_back ("glade");
        path_elems.push_back ("proclistdialog.glade");
        string glade_path = Glib::build_filename (path_elems) ;
        if (!Glib::file_test (glade_path, Glib::FILE_TEST_IS_REGULAR)) {
            THROW (UString ("could not find file ") + glade_path) ;
        }
        glade = Gnome::Glade::Xml::create (glade_path) ;
        THROW_IF_FAIL (glade) ;
        dialog = env::get_widget_from_glade<Gtk::Dialog> (glade,
                                                          "proclistdialog") ;
        init_widget () ;
    }

    void init_widget ()
    {
        dialog->hide () ;
        proclist_view = env::get_widget_from_glade<Gtk::TreeView>
                                                    (glade, "proclistdialog") ;
        Glib::RefPtr<Gtk::ListStore> list_store = Gtk::ListStore::create (columns ()) ;
        proclist_view->set_model (list_store) ;
        proclist_view->append_column ("PID", columns ().pid) ;
        proclist_view->append_column ("User Name", columns ().user_name) ;
        proclist_view->append_column ("Proc Name", columns ().proc_name) ;
        proclist_view->append_column ("Proc Args", columns ().proc_args) ;
        proclist_view->get_selection ()->set_mode (Gtk::SELECTION_SINGLE) ;
    }

    void load_process_list ()
    {
        list<IProcMgr::Process> process_list = proc_mgr.get_all_process_list ();
        list<IProcMgr::Process>::iterator iter;
        for (iter = process_list.begin () ; iter != process_list.end () ; ++iter) {
            cout << "pid: "  << iter->pid () << "\n";
            cout << "user name: "  << iter->user_name () << "\n";
            cout << "prog: "  << *(iter->args ().begin ()) << "\n";
        }
    }
};//end ProcListDialog


ProcListDialog::ProcListDialog (const UString &a_root_path,
                                IProcMgr &a_proc_mgr)
{
    m_priv = new Priv (a_root_path, a_proc_mgr);
}

gint
ProcListDialog::run ()
{
    m_priv->load_process_list () ;
    return m_priv->dialog->run () ;
}

bool
ProcListDialog::has_selected_process ()
{
    return false ;
}

bool
ProcListDialog::get_selected_process (IProcMgr::Process &a_proc)
{
    return false ;
}

}//end namespace nemiver

