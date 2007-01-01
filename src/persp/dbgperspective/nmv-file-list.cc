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
#include <gtkmm/liststore.h>
#include "nmv-file-list.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"
#include "nmv-i-workbench.h"
#include "nmv-i-perspective.h"

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

struct FileList::Priv {
public:
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::ListStore> list_store ;
    Gtk::Widget *file_list_menu;
    sigc::signal<void,
                 const UString&> file_selected_signal;
    Glib::RefPtr<Gtk::ActionGroup> file_list_action_group;
    IWorkbench& workbench;
    IPerspective& perspective;
    IDebuggerSafePtr debugger;
    sigc::connection list_files_connection;

    Priv (IDebuggerSafePtr& a_debugger, IWorkbench& a_workbench, IPerspective& a_perspective) :
        file_list_menu(0),
        workbench(a_workbench),
        perspective(a_perspective),
        debugger(a_debugger)
    {
        init_actions ();
        build_tree_view () ;
        tree_view->signal_button_press_event ().connect_notify (sigc::mem_fun
                (*this, &Priv::on_file_list_button_press_signal));
        debugger->files_listed_signal ().connect(sigc::mem_fun(*this,
                    &Priv::set_files));
        list_files_connection = debugger->stopped_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_debugger_stopped_signal)) ;
    }

    void on_debugger_stopped_signal (const UString &a_reason,
                                     bool a_has_frame,
                                     const IDebugger::Frame &a_frame,
                                     int a_thread_id)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY
        LOG_DD ("stopped, reason: " << a_reason) ;

        if (a_has_frame || a_frame.line () || a_thread_id) {}

        if (a_reason == "exited-signaled"
            || a_reason == "exited-normally"
            || a_reason == "exited") {
            return ;
        }

        THROW_IF_FAIL (debugger) ;
        debugger->list_files();
        // only need to list files once, so disconnect this handler after we've
        // handled it once
        list_files_connection.disconnect ();

        NEMIVER_CATCH
    }

    void build_tree_view ()
    {
        if (tree_view) {return;}
        //create a default tree store and a tree view
        list_store = Gtk::ListStore::create (get_file_list_columns ()) ;
        tree_view.reset (new Gtk::TreeView (list_store)) ;

        //create the columns of the tree view
        tree_view->append_column (_("Filename"), get_file_list_columns ().display_name) ;
    }

    void set_files (const std::vector<UString> &a_files)
    {
        THROW_IF_FAIL (list_store) ;
        list_store->clear();
        std::vector<UString>::const_iterator file_iter;
        for (file_iter = a_files.begin (); file_iter != a_files.end ();
                ++file_iter)
        {
            Gtk::TreeModel::iterator tree_iter = list_store->append();
            (*tree_iter)[get_file_list_columns ().path] = *file_iter;
            (*tree_iter)[get_file_list_columns ().display_name] =
                Glib::filename_display_name (Glib::locale_to_utf8
                        (*file_iter));
        }
    }

    Gtk::Widget* load_menu (UString a_filename, UString a_widget_name)
    {
        NEMIVER_TRY
        string relative_path = Glib::build_filename ("menus", a_filename) ;
        string absolute_path ;
        THROW_IF_FAIL (perspective.build_absolute_resource_path
                (Glib::locale_to_utf8 (relative_path), absolute_path)) ;

        workbench.get_ui_manager ()->add_ui_from_file (Glib::locale_to_utf8 (absolute_path)) ;

        NEMIVER_CATCH
        return workbench.get_ui_manager ()->get_widget (a_widget_name);
    }

    Gtk::Widget* get_file_list_menu ()
    {
        if (!file_list_menu) {
            file_list_menu = load_menu ("filelistpopup.xml",
                    "/FileListPopup");
            THROW_IF_FAIL (file_list_menu);
        }
        return file_list_menu;
    }

    void popup_file_list_menu (GdkEventButton *a_event)
    {
        THROW_IF_FAIL (a_event) ;
        THROW_IF_FAIL (tree_view) ;
        Gtk::Menu *menu = dynamic_cast<Gtk::Menu*> (get_file_list_menu ()) ;
        THROW_IF_FAIL (menu) ;
        // only pop up a menu if a row exists at that position
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* p_column = NULL;
        int cell_x=0, cell_y=0;
        if (tree_view->get_path_at_pos(static_cast<int>(a_event->x),
                    static_cast<int>(a_event->y), path, p_column, cell_x,
                    cell_y)) {
            menu->popup (a_event->button, a_event->time) ;
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

        // right-clicking should pop up a context menu
        else if (a_event->type == GDK_BUTTON_PRESS) {
            if (a_event->button == 3) {
                popup_file_list_menu (a_event) ;
            }
        }

        NEMIVER_CATCH
    }

    void on_file_selected_action ()
    {
        NEMIVER_TRY
        THROW_IF_FAIL(tree_view)
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection ();
        Gtk::TreeModel::iterator tree_iter = selection->get_selected();
        if (tree_iter) {
            file_selected_signal.emit (UString((*tree_iter)[get_file_list_columns ().path]));
        }
        NEMIVER_CATCH
    }

    void init_actions()
    {
        static ui_utils::ActionEntry s_file_list_action_entries [] = {
            {
                "FileSelectedMenuItemAction",
                Gtk::Stock::OPEN,
                _("_Open File"),
                _("Open this file in the source editor"),
                sigc::mem_fun (*this, &Priv::on_file_selected_action),
                ui_utils::ActionEntry::DEFAULT,
                ""
            }
        };

        file_list_action_group =
            Gtk::ActionGroup::create ("file-list-action-group") ;
        file_list_action_group->set_sensitive (true) ;

        int num_actions =
            sizeof (s_file_list_action_entries)/sizeof (ui_utils::ActionEntry) ;

        ui_utils::add_action_entries_to_action_group
            (s_file_list_action_entries, num_actions,
             file_list_action_group) ;

        workbench.get_ui_manager ()->insert_action_group (file_list_action_group);
    }

};//end class FileList::Priv

FileList::FileList (IDebuggerSafePtr& a_debugger, IWorkbench& a_workbench,
        IPerspective& a_perspective)
{
    m_priv.reset (new Priv (a_debugger, a_workbench, a_perspective));
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
    THROW_IF_FAIL (m_priv->list_store) ;
    return *m_priv->tree_view ;
}

sigc::signal<void, const UString&>&
FileList::signal_file_selected () const
{
    THROW_IF_FAIL(m_priv);
    return m_priv->file_selected_signal;
}

}//end namespace nemiver

