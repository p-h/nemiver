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
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include "nmv-preferences-dialog.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE(nemiver)

struct SourceDirsCols : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> dir ;

    SourceDirsCols ()
    {
        add (dir) ;
    }
};// end SourceDirsColumns

static SourceDirsCols&
source_dirs_cols ()
{
    static SourceDirsCols s_cols ;
    return s_cols ;
}

class PreferencesDialog::Priv {
    Priv () {}

public:
    vector<UString> source_dirs ;
    Glib::RefPtr<Gtk::ListStore> list_store ;
    Gtk::TreeView *tree_view ;
    Gtk::TreeModel::iterator cur_dir_iter ;
    Gtk::Button *remove_dir_button ;

    Priv (Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        tree_view (0),
        remove_dir_button (0)
    {
        init (a_glade) ;
    }

    void init (Glib::RefPtr<Gnome::Glade::Xml> &a_glade)
    {

        list_store = Gtk::ListStore::create (source_dirs_cols ()) ;
        tree_view =
            ui_utils::get_widget_from_glade<Gtk::TreeView> (a_glade,
                                                            "dirstreeview") ;
        tree_view->append_column (_("Source directories"),
                                  source_dirs_cols ().dir) ;

        tree_view->set_headers_visible (false) ;

        tree_view->set_model (list_store) ;

        tree_view->get_selection ()->set_mode (Gtk::SELECTION_SINGLE) ;

        tree_view->get_selection ()->signal_changed ().connect (sigc::mem_fun
                (this, &PreferencesDialog::Priv::on_tree_view_selection_changed));

        Gtk::Button *button =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade, "adddirbutton");
        button->signal_clicked ().connect (sigc::mem_fun
                (this, &PreferencesDialog::Priv::on_add_dir_button_clicked)) ;

        remove_dir_button =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade,
                                                          "suppressdirbutton") ;
        remove_dir_button->signal_clicked ().connect (sigc::mem_fun
            (this, &PreferencesDialog::Priv::on_remove_dir_button_clicked)) ;
        remove_dir_button->set_sensitive (false) ;
    }

    void on_tree_view_selection_changed ()
    {
        THROW_IF_FAIL (tree_view) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        cur_dir_iter = sel->get_selected () ;
        if (cur_dir_iter) {
            remove_dir_button->set_sensitive (true) ;
        } else {
            remove_dir_button->set_sensitive (false) ;
        }
    }

    void on_add_dir_button_clicked ()
    {
        NEMIVER_TRY

        Gtk::FileChooserDialog file_chooser
                                (_("Choose directory"),
                                 Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER) ;

        file_chooser.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL) ;
        file_chooser.add_button (Gtk::Stock::OK, Gtk::RESPONSE_OK) ;
        file_chooser.set_select_multiple (false) ;

        int result = file_chooser.run () ;

        if (result != Gtk::RESPONSE_OK) {LOG_DD ("cancelled") ; return;}

        UString path = file_chooser.get_filename () ;

        if (path == "") {LOG_DD ("Got null dir") ;return;}

        Gtk::TreeModel::iterator iter = list_store->append () ;
        (*iter)[source_dirs_cols ().dir] = path ;
        NEMIVER_CATCH
    }

    void on_remove_dir_button_clicked ()
    {
        NEMIVER_TRY
        if (!cur_dir_iter) {return;}
        list_store->erase (cur_dir_iter) ;
        NEMIVER_CATCH
    }

    void collect_source_dirs ()
    {
        source_dirs.clear () ;
        Gtk::TreeModel::iterator iter;

        for (iter = list_store->children ().begin ();
             iter != list_store->children ().end ();
             ++iter) {
            source_dirs.push_back (UString ((*iter)[source_dirs_cols ().dir])) ;
        }

    }

    void set_source_dirs (const vector<UString> &a_dirs)
    {
        Gtk::TreeModel::iterator row_it ;
        vector<UString>::const_iterator dir_it ;
        for (dir_it = a_dirs.begin () ; dir_it != a_dirs.end () ; ++dir_it) {
            row_it = list_store->append () ;
            (*row_it)[source_dirs_cols ().dir] = *dir_it ;
        }
    }
};//end PreferencesDialog

PreferencesDialog::PreferencesDialog (const UString &a_root_path) :
    Dialog (a_root_path,
            "preferencesdialog.glade",
            "preferencesdialog")
{
    m_priv = new Priv (m_glade) ;
}

PreferencesDialog::~PreferencesDialog ()
{
    THROW_IF_FAIL (m_priv) ;
    m_priv = 0 ;
}

const std::vector<UString>&
PreferencesDialog::source_directories () const
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->collect_source_dirs () ;
    return m_priv->source_dirs ;
}

void
PreferencesDialog::source_directories (const std::vector<UString> &a_dirs)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->source_dirs = a_dirs ;
    m_priv->set_source_dirs (m_priv->source_dirs) ;
}

NEMIVER_END_NAMESPACE
