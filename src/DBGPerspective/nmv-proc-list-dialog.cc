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

struct ProcListCols : public Gtk::TreeModel::ColumnRecord {
    Gtk::TreeModelColumn<IProcMgr::Process> process ;
    Gtk::TreeModelColumn<unsigned int> pid ;
    Gtk::TreeModelColumn<Glib::ustring> user_name ;
    Gtk::TreeModelColumn<Glib::ustring> proc_args ;

    enum ColsOffset {
        PROCESS=0,
        PID,
        USER_NAME,
        PROC_ARGS
    };

    ProcListCols ()
    {
        add (process) ;
        add (pid) ;
        add (user_name) ;
        add (proc_args) ;
    }
};//end class Gtk::TreeModel

static ProcListCols&
columns ()
{
    static ProcListCols s_columns ;
    return s_columns ;
}

struct ProcListDialog::Priv {
    IProcMgr &proc_mgr ;
    UString root_path ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;
    SafePtr<Gtk::Dialog> dialog ;
    Gtk::TreeView *proclist_view ;
    Glib::RefPtr<Gtk::ListStore> proclist_store ;
    IProcMgr::Process selected_process ;
    bool process_selected ;

    Priv (const UString &a_root_path,
          IProcMgr &a_proc_mgr) :
        proc_mgr (a_proc_mgr),
        root_path (a_root_path),
        process_selected (false)
    {
        init () ;
    }

    void init ()
    {
        load_glade_file () ;
    }

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

    void on_selection_changed_signal ()
    {
        vector<Gtk::TreeModel::Path> paths =
           proclist_view->get_selection ()->get_selected_rows () ;

        if (paths.empty ()) {return;}

        Gtk::TreeModel::iterator row_it = proclist_store->get_iter (paths[0]) ;
        if (row_it == proclist_store->children ().end ()) {return;}
        selected_process = (*row_it)[columns ().process] ;
        process_selected = true ;
    }

    void init_widget ()
    {
        dialog->hide () ;
        proclist_view = env::get_widget_from_glade<Gtk::TreeView>
                                                    (glade, "proclisttreeview") ;
        proclist_store = Gtk::ListStore::create (columns ()) ;
        proclist_view->set_model (proclist_store) ;

        proclist_view->append_column ("PID", columns ().pid) ;
        Gtk::TreeViewColumn *col = proclist_view->get_column (0) ;
        THROW_IF_FAIL (col) ;
        col->set_clickable (true) ;
        col->set_resizable (true) ;
        col->set_sort_column_id (columns ().pid) ;

        proclist_view->append_column ("User Name", columns ().user_name) ;
        col = proclist_view->get_column (1) ;
        THROW_IF_FAIL (col) ;
        col->set_clickable (true) ;
        col->set_resizable (true) ;
        col->set_sort_column_id (columns ().user_name) ;

        proclist_view->append_column ("Proc Args", columns ().proc_args) ;
        col = proclist_view->get_column (2) ;
        THROW_IF_FAIL (col) ;
        col->set_clickable (true) ;
        col->set_resizable (true) ;
        col->set_sort_column_id (columns ().proc_args) ;

        proclist_view->get_selection ()->set_mode (Gtk::SELECTION_SINGLE) ;
        col = proclist_view->get_column (ProcListCols::PID) ;

        proclist_view->get_selection ()->signal_changed ().connect
                                                        (sigc::mem_fun (*this,
                                                         &Priv::on_selection_changed_signal)) ;
    }

    void load_process_list ()
    {
        process_selected = false ;
        Gtk::TreeModel::iterator store_it ;
        list<IProcMgr::Process> process_list = proc_mgr.get_all_process_list ();
        list<IProcMgr::Process>::iterator process_iter;
        list<UString> args ;
        list<UString>::iterator str_iter ;
        UString args_str ;
        proclist_store->clear () ;
        for (process_iter = process_list.begin () ;
             process_iter != process_list.end ();
             ++process_iter) {
            args = process_iter->args () ;
            if (args.empty ()) {continue;}
            store_it = proclist_store->append () ;
            (*store_it)[columns ().pid] = process_iter->pid () ;
            (*store_it)[columns ().user_name] = process_iter->user_name () ;
            args_str = "" ;
            for (str_iter = args.begin () ;
                 str_iter != args.end () ;
                 ++str_iter) {
                args_str += *str_iter + " " ;
            }
            (*store_it)[columns ().proc_args] = args_str ;
            (*store_it)[columns ().process] = *process_iter;
        }
    }
};//end ProcListDialog


ProcListDialog::ProcListDialog (const UString &a_root_path,
                                IProcMgr &a_proc_mgr)
{
    m_priv = new Priv (a_root_path, a_proc_mgr);
}

ProcListDialog::~ProcListDialog ()
{
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
    return m_priv->process_selected;
}

bool
ProcListDialog::get_selected_process (IProcMgr::Process &a_proc)
{
    if (!m_priv->process_selected) {return false;}
    a_proc = m_priv->selected_process ;
    return true;
}

}//end namespace nemiver

