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
#include <glib/gi18n.h>
#include "nmv-proc-list-dialog.h"
#include "nmv-env.h"
#include "nmv-ui-utils.h"

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

class ProcListDialog::Priv {
public:
    IProcMgr &proc_mgr ;
    Gtk::Button *okbutton ;
    Gtk::TreeView *proclist_view ;
    Glib::RefPtr<Gtk::ListStore> proclist_store ;
    IProcMgr::Process selected_process ;
    bool process_selected ;

    Priv (const Glib::RefPtr<Gnome::Glade::Xml> &a_glade,
          IProcMgr &a_proc_mgr) :
        proc_mgr (a_proc_mgr),
        okbutton (0),
        proclist_view (0),
        process_selected (false)
    {
        okbutton =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade, "okbutton") ;
        THROW_IF_FAIL (okbutton) ;
        okbutton->set_sensitive (false) ;

        proclist_view =
            ui_utils::get_widget_from_glade<Gtk::TreeView> (a_glade,
                                                            "proclisttreeview");
        THROW_IF_FAIL (proclist_view) ;
        proclist_store = Gtk::ListStore::create (columns ()) ;
        proclist_view->set_model (proclist_store) ;
        proclist_view->set_search_column (ProcListCols::PROC_ARGS) ;
        proclist_view->set_enable_search (true) ;

        proclist_view->append_column ("PID", columns ().pid) ;
        Gtk::TreeViewColumn *col = proclist_view->get_column (0) ;
        THROW_IF_FAIL (col) ;
        col->set_clickable (true) ;
        col->set_resizable (true) ;
        col->set_sort_column_id (columns ().pid) ;

        proclist_view->append_column (_("User Name"), columns ().user_name) ;
        col = proclist_view->get_column (1) ;
        THROW_IF_FAIL (col) ;
        col->set_clickable (true) ;
        col->set_resizable (true) ;
        col->set_sort_column_id (columns ().user_name) ;

        proclist_view->append_column (_("Proc Args"), columns ().proc_args) ;
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
        proclist_view->signal_row_activated ().connect (sigc::mem_fun
                (*this, &Priv::on_row_activated_signal)) ;
    }

    void on_selection_changed_signal ()
    {
        NEMIVER_TRY

        vector<Gtk::TreeModel::Path> paths =
            proclist_view->get_selection ()->get_selected_rows () ;

        if (paths.empty ()) {return;}

        Gtk::TreeModel::iterator row_it = proclist_store->get_iter (paths[0]) ;
        if (row_it == proclist_store->children ().end ()) {return;}
        selected_process = (*row_it)[columns ().process] ;
        process_selected = true ;

        THROW_IF_FAIL (okbutton) ;
        okbutton->set_sensitive (true) ;

        NEMIVER_CATCH
    }

    void on_row_activated_signal (const Gtk::TreeModel::Path &a_path,
                                  Gtk::TreeViewColumn *a_col)
    {
        if (a_col) {}

        LOG_FUNCTION_SCOPE_NORMAL_DD

        NEMIVER_TRY

        THROW_IF_FAIL (okbutton) ;

        Gtk::TreeModel::iterator row_it = proclist_store->get_iter (a_path) ;
        if (!row_it) {return;}
        selected_process = (*row_it)[columns ().process] ;
        process_selected = true ;
        okbutton->clicked () ;

        NEMIVER_CATCH
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
};//end class ProcListDialog::Priv


ProcListDialog::ProcListDialog (const UString &a_root_path,
                                IProcMgr &a_proc_mgr) :
    Dialog(a_root_path, "proclistdialog.glade", "proclistdialog")
{
    m_priv.reset (new Priv (glade (), a_proc_mgr));
    widget ().hide () ;
}

gint ProcListDialog::run ()
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->load_process_list();
    return Dialog::run();
}

ProcListDialog::~ProcListDialog ()
{
}

bool
ProcListDialog::has_selected_process ()
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->process_selected;
}

bool
ProcListDialog::get_selected_process (IProcMgr::Process &a_proc)
{
    THROW_IF_FAIL (m_priv) ;
    if (!m_priv->process_selected) {return false;}
    a_proc = m_priv->selected_process ;
    return true;
}

}//end namespace nemiver

