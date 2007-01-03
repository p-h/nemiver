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
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include "nmv-file-list.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"

namespace nemiver {

struct FileListColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> display_name ;
    Gtk::TreeModelColumn<Glib::ustring> path ;

    FileListColumns ()
    {
        add (display_name) ;
        add (path) ;
    }
};//end Cols

static FileListColumns&
get_file_list_columns ()
{
    static FileListColumns s_cols ;
    return s_cols ;
}

struct FileList::Priv : public sigc::trackable {
public:
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_model ;
    sigc::signal<void,
                 const UString&> file_selected_signal;
    Glib::RefPtr<Gtk::ActionGroup> file_list_action_group;
    IDebuggerSafePtr debugger;

    Priv (IDebuggerSafePtr& a_debugger) :
        debugger(a_debugger)
    {
        build_tree_view () ;
        tree_view->signal_button_press_event ().connect_notify (sigc::mem_fun
                (*this, &Priv::on_file_list_button_press_signal));
        debugger->files_listed_signal ().connect(sigc::mem_fun(*this,
                    &Priv::set_files));
        debugger->list_files();
    }

    void build_tree_view ()
    {
        if (tree_view) {return;}
        //create a default tree store and a tree view
        tree_model = Gtk::TreeStore::create (get_file_list_columns ()) ;
        tree_view.reset (new Gtk::TreeView (tree_model)) ;

        tree_view->set_headers_visible (false);
        //create the columns of the tree view
        tree_view->append_column (_("Filename"), get_file_list_columns ().display_name) ;
        tree_view->get_selection ()->set_mode (Gtk::SELECTION_MULTIPLE);
    }

    struct ComparePathMap :
                    public std::binary_function<const std::pair<UString, Gtk::TreeModel::iterator>,
                                                const UString,
                                                bool> {
        bool operator()(const std::pair<UString, Gtk::TreeModel::iterator>& map,
                const UString& path_component)
        {
            return path_component == map.first;
        }
    };

    void set_files (const std::vector<UString> &a_files)
    {
        // NOTE: This assumes a sorted file list.  If the file list is not
        // sorted, this function will not work as expected

        THROW_IF_FAIL (tree_model) ;
        if (!(tree_model->children ().empty ())) {
            tree_model->clear();
        }

        // a map vector to keep track of what directory levels we have most
        // recently added so that its easier to figure out where new paths need
        // to be added to the tree structure
        typedef std::pair<UString, Gtk::TreeModel::iterator> folder_map_t;
        typedef vector<folder_map_t> folder_map_list_t;
        folder_map_list_t folder_map;

        std::vector<UString>::const_iterator file_iter;
        for (file_iter = a_files.begin (); file_iter != a_files.end ();
                ++file_iter)
        {
            vector<UString> path_components;
            // only add absolute paths to the treeview
            if (Glib::path_is_absolute (*file_iter)) {
                path_components = (*file_iter).split (G_DIR_SEPARATOR_S);
                // find the first point where the last added path and the new
                // path do not match
                pair<folder_map_list_t::iterator, vector<UString>::iterator>
                    mismatch_iter = std::mismatch(folder_map.begin (),
                            folder_map.end (), path_components.begin (),
                            ComparePathMap());
                // ASSUMPTION: there will always be a mismatch found before we
                // run out of components of the new path since all paths in the
                // list should be unique
                THROW_IF_FAIL(mismatch_iter.second != path_components.end ());
                // truncate the folder_map vector at the point of the mismatch
                folder_map.erase (mismatch_iter.first, folder_map.end ());

                // loop through any remaining new path elements and add them to
                // the tree
                for (vector<UString>::iterator iter =
                        mismatch_iter.second; iter !=
                        path_components.end (); ++iter) {
                    Gtk::TreeModel::iterator tree_iter;
                    if (folder_map.empty ()) {
                        // add as children of the root level
                        tree_iter = tree_model->append ();
                    } else {
                        // add as children of the last matched item in the folder map
                        tree_iter = tree_model->append (folder_map.rbegin ()->second->children ());
                    }
                    // build the full path name up to this element
                    (*tree_iter)[get_file_list_columns ().path] = "/" + // add back the root element
                        Glib::build_filename (std::vector<UString>(path_components.begin(),
                                    iter + 1)); // we want to include this iter element in the array, so + 1
                    Glib::ustring display_name =
                        Glib::filename_display_name (Glib::locale_to_utf8 (*iter));
                    (*tree_iter)[get_file_list_columns ().display_name] =
                        display_name.empty () ? "/" : display_name;
                    // store the element in the folder map for use next time
                    // around
                    folder_map.push_back (folder_map_t(*iter, tree_iter));
                }
            }
        }
    }

    void on_file_list_button_press_signal (GdkEventButton *a_event)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        // double-clicking a filename should activate the file-selected action
        if (a_event->type == GDK_2BUTTON_PRESS) {
            if (a_event->button == 1) {
                on_file_selected_action ();
            }
        }

        NEMIVER_CATCH
    }

    list<UString> get_selected_filenames () const
    {
        THROW_IF_FAIL (tree_view)
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection ();
        THROW_IF_FAIL (selection);
        list<Gtk::TreeModel::Path> paths = selection->get_selected_rows ();

        list<UString> filenames;
        for (list<Gtk::TreeModel::Path>::iterator path_iter = paths.begin ();
                path_iter != paths.end (); ++path_iter)
        {
            Gtk::TreeModel::iterator tree_iter = (tree_view->get_model ()->get_iter(*path_iter));
            filenames.push_back (UString((*tree_iter)[get_file_list_columns ().path]));
        }
        return filenames;
    }

    void on_file_selected_action ()
    {
        NEMIVER_TRY
        //file_selected_signal.emit (get_selected_filename ());
        NEMIVER_CATCH
    }

};//end class FileList::Priv

FileList::FileList (IDebuggerSafePtr& a_debugger)
{
    m_priv.reset (new Priv (a_debugger));
}

FileList::~FileList ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

Gtk::Widget&
FileList::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    THROW_IF_FAIL (m_priv->tree_model) ;
    return *m_priv->tree_view ;
}

sigc::signal<void, const UString&>&
FileList::signal_file_selected () const
{
    THROW_IF_FAIL(m_priv);
    return m_priv->file_selected_signal;
}

list<UString>
FileList::get_filenames () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_selected_filenames ();
}

}//end namespace nemiver

