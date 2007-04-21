//Author: Dodji Seketeli, Jonathon Jongsma
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
#include <map>
#include <glib/gi18n.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treerowreference.h>
#include "common/nmv-exception.h"
#include "nmv-breakpoints-view.h"
#include "nmv-ui-utils.h"
#include "nmv-i-workbench.h"
#include "nmv-i-perspective.h"

namespace nemiver {

struct BPColumns : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<int> id ;
    Gtk::TreeModelColumn<bool> enabled ;
    Gtk::TreeModelColumn<Glib::ustring> filename ;
    Gtk::TreeModelColumn<int> line ;
    Gtk::TreeModelColumn<IDebugger::BreakPoint> breakpoint ;

    BPColumns ()
    {
        add (id) ;
        add (enabled) ;
        add (filename) ;
        add (line) ;
        add (breakpoint) ;
    }
};//end Cols

static BPColumns&
get_bp_columns ()
{
    static BPColumns s_cols ;
    return s_cols ;
}

struct BreakpointsView::Priv {
public:
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::ListStore> list_store ;
    Gtk::Widget *breakpoints_menu;
    sigc::signal<void,
                 const IDebugger::BreakPoint&> go_to_breakpoint_signal;
    Glib::RefPtr<Gtk::ActionGroup> breakpoints_action_group;
    IWorkbench& workbench;
    IPerspective& perspective;
    IDebuggerSafePtr& debugger;

    Priv (IWorkbench& a_workbench,
          IPerspective& a_perspective,
          IDebuggerSafePtr& a_debugger) :
        breakpoints_menu(0),
        workbench(a_workbench),
        perspective(a_perspective),
        debugger(a_debugger)
    {
        init_actions ();
        build_tree_view () ;
        void set_breakpoints
                (const std::map<int, IDebugger::BreakPoint> &a_breakpoints);
        tree_view->signal_button_press_event ().connect_notify
            (sigc::mem_fun (*this,
                            &Priv::on_breakpoints_view_button_press_signal));

        // update breakpoint list when debugger indicates that the list of
        // breakpoints has changed.
        debugger->breakpoint_deleted_signal ().connect (sigc::mem_fun
                (*this, &Priv::on_debugger_breakpoint_deleted_signal)) ;
        debugger->breakpoints_set_signal ().connect (sigc::mem_fun
                (*this, &Priv::on_debugger_breakpoints_set_signal)) ;
    }

    void build_tree_view ()
    {
        if (tree_view) {return;}
        //create a default tree store and a tree view
        list_store = Gtk::ListStore::create (get_bp_columns ()) ;
        tree_view.reset (new Gtk::TreeView (list_store)) ;
        tree_view->get_selection ()->set_mode (Gtk::SELECTION_MULTIPLE) ;

        //create the columns of the tree view
        tree_view->append_column_editable ("", get_bp_columns ().enabled) ;
        tree_view->append_column (_("ID"), get_bp_columns ().id) ;
        tree_view->append_column (_("Filename"), get_bp_columns ().filename) ;
        tree_view->append_column (_("Line"), get_bp_columns ().line) ;
        Gtk::CellRendererToggle *enabled_toggle =
            dynamic_cast<Gtk::CellRendererToggle*>
                                    (tree_view->get_column_cell_renderer(0));
        if (enabled_toggle) {
            enabled_toggle->signal_toggled ().connect (sigc::mem_fun (*this,
                        &BreakpointsView::Priv::on_breakpoint_enable_toggled));
        }
    }

    void on_breakpoint_enable_toggled (const Glib::ustring& path)
    {
        THROW_IF_FAIL (tree_view);
        Gtk::TreeModel::iterator tree_iter =
                                    tree_view->get_model ()->get_iter (path);
        if (tree_iter) {
            if ((*tree_iter)[get_bp_columns ().enabled]) {
                debugger->enable_breakpoint((*tree_iter)[get_bp_columns ().id]);
            } else {
                debugger->disable_breakpoint((*tree_iter)[get_bp_columns ().id]);
            }
        }
    }


    void set_breakpoints
        (const std::map<int, IDebugger::BreakPoint> &a_breakpoints)
    {
        if (a_breakpoints.empty ()) {
            return;
        }

        if (list_store->children ().empty ()) {
            // there are no breakpoints in the list yet, so no need to do any
            // searching for things to update, just add them all directly
            add_breakpoints(a_breakpoints);
        } else {
            //find breakpoints that need adding or updating
            std::map<int, IDebugger::BreakPoint>::const_iterator breakmap_iter;
            for (breakmap_iter = a_breakpoints.begin ();
                 breakmap_iter != a_breakpoints.end ();
                 ++breakmap_iter) {
                Gtk::TreeModel::iterator tree_iter =
                    find_breakpoint_in_model(breakmap_iter->second);
                if (tree_iter) {
                    update_breakpoint(tree_iter, breakmap_iter->second);
                } else {
                    append_breakpoint(breakmap_iter->second);
                }
            }
        }
    }

    void add_breakpoints
                (const std::map<int, IDebugger::BreakPoint> &a_breakpoints)
    {
        THROW_IF_FAIL (list_store) ;

        std::map<int, IDebugger::BreakPoint>::const_iterator break_iter;
        for (break_iter = a_breakpoints.begin ();
                break_iter != a_breakpoints.end ();
                ++break_iter) {
            append_breakpoint(break_iter->second);
        }
    }

    bool breakpoint_list_has_id (const std::map<int, IDebugger::BreakPoint>
            &a_breakpoints, int a_id)
    {
        std::map<int, IDebugger::BreakPoint>::const_iterator breakmap_iter ;
        for (breakmap_iter = a_breakpoints.begin ();
                breakmap_iter != a_breakpoints.end (); ++ breakmap_iter) {
            if (a_id == breakmap_iter->second.number ()) {
                return true;
            }
        }
        return false;
    }

    Gtk::TreeModel::iterator find_breakpoint_in_model
                                (const IDebugger::BreakPoint &a_breakpoint)
    {
        THROW_IF_FAIL (list_store) ;

        Gtk::TreeModel::iterator iter;
        for (iter = list_store->children ().begin ();
             iter != list_store->children ().end ();
             ++iter) {
            if ((*iter)[get_bp_columns ().id] == a_breakpoint.number ()) {
                return iter;
            }
        }
        // Breakpoint not found in model, return an invalid iter
        return Gtk::TreeModel::iterator();
    }

    void update_breakpoint(Gtk::TreeModel::iterator& a_iter,
            const IDebugger::BreakPoint &a_breakpoint)
    {
        // we should only be 'updating' a breakpoint in the list if they have
        // the same ID
        THROW_IF_FAIL ((*a_iter)[get_bp_columns ().id]
                       == a_breakpoint.number ());
        (*a_iter)[get_bp_columns ().breakpoint] = a_breakpoint;
        (*a_iter)[get_bp_columns ().enabled] = a_breakpoint.enabled () ;
        (*a_iter)[get_bp_columns ().filename] = a_breakpoint.file_name () ;
        (*a_iter)[get_bp_columns ().line] = a_breakpoint.line () ;
    }

    Gtk::TreeModel::iterator append_breakpoint
                                    (const IDebugger::BreakPoint &a_breakpoint)
    {
        Gtk::TreeModel::iterator tree_iter = list_store->append();
        (*tree_iter)[get_bp_columns ().id] = a_breakpoint.number ();
        (*tree_iter)[get_bp_columns ().breakpoint] = a_breakpoint;
        (*tree_iter)[get_bp_columns ().enabled] = a_breakpoint.enabled ();
        (*tree_iter)[get_bp_columns ().filename] = a_breakpoint.file_name ();
        (*tree_iter)[get_bp_columns ().line] = a_breakpoint.line ();

        return tree_iter;
    }

    void on_debugger_breakpoints_set_signal
                            (const map<int, IDebugger::BreakPoint> &a_breaks,
                             const UString &a_cookie)
    {
        NEMIVER_TRY
        if (a_cookie.empty ()) {}
        set_breakpoints (a_breaks);
        NEMIVER_CATCH
    }

    void on_debugger_breakpoint_deleted_signal
            (const IDebugger::BreakPoint &a_break, int a_break_number,
             const UString &a_cookie)
    {
        if (a_break.number () || a_cookie.empty()) {}
        NEMIVER_TRY
        list<Gtk::TreeModel::iterator> iters_to_erase ;
        for (Gtk::TreeModel::iterator iter = list_store->children ().begin ();
                iter != list_store->children ().end ();
                ++iter) {
            if ((*iter)[get_bp_columns ().id] == a_break_number) {
                iters_to_erase.push_back (iter) ;
                break;
            }
        }
        for (list<Gtk::TreeModel::iterator>::iterator it =
                iters_to_erase.begin ();
             it != iters_to_erase.end ();
             ++it) {
            list_store->erase (*it) ;
        }
        NEMIVER_CATCH
    }

    Gtk::Widget* load_menu (UString a_filename, UString a_widget_name)
    {
        NEMIVER_TRY
        string relative_path = Glib::build_filename ("menus", a_filename) ;
        string absolute_path ;
        THROW_IF_FAIL (perspective.build_absolute_resource_path
                (Glib::locale_to_utf8 (relative_path), absolute_path)) ;

        workbench.get_ui_manager ()->add_ui_from_file
                                        (Glib::locale_to_utf8 (absolute_path)) ;
        NEMIVER_CATCH
        return workbench.get_ui_manager ()->get_widget (a_widget_name);
    }

    Gtk::Widget* get_breakpoints_menu ()
    {
        if (!breakpoints_menu) {
            breakpoints_menu = load_menu ("breakpointspopup.xml",
                                          "/BreakpointsPopup");
            THROW_IF_FAIL (breakpoints_menu);
        }
        return breakpoints_menu;
    }

    void popup_breakpoints_view_menu (GdkEventButton *a_event)
    {
        THROW_IF_FAIL (a_event) ;
        THROW_IF_FAIL (tree_view) ;
        Gtk::Menu *menu = dynamic_cast<Gtk::Menu*> (get_breakpoints_menu ()) ;
        THROW_IF_FAIL (menu) ;
        // only pop up a menu if a row exists at that position
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* p_column = NULL;
        int cell_x=0, cell_y=0;
        if (tree_view->get_path_at_pos
                (static_cast<int>(a_event->x),
                 static_cast<int>(a_event->y),
                 path, p_column, cell_x, cell_y)) {
            if (tree_view->get_selection ()->count_selected_rows () > 1) {
                Glib::RefPtr<Gtk::Action> action =
                    workbench.get_ui_manager ()->get_action
                            ("/BreakpointsPopup/GoToSourceBreakpointMenuItem");
                if (action) {
                    action->set_sensitive (false) ;
                } else {
                    LOG_ERROR
                        ("Could not get action "
                         "/BreakpointsPopup/GoToSourceBreakpointMenuItem") ;
                }
            } else {
                Glib::RefPtr<Gtk::Action> action =
                    workbench.get_ui_manager ()->get_action
                            ("/BreakpointsPopup/GoToSourceBreakpointMenuItem");
                if (action) {
                    action->set_sensitive (true) ;
                } else {
                    LOG_ERROR
                        ("Could not get action "
                         "/BreakpointsPopup/GoToSourceBreakpointMenuItem") ;
                }
            }
            menu->popup (a_event->button, a_event->time) ;
        }
    }

    void on_breakpoints_view_button_press_signal (GdkEventButton *a_event)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        // double-clicking a breakpoint item should go to the source location for
        // the breakpoint
        if (a_event->type == GDK_2BUTTON_PRESS) {
            if (a_event->button == 1) {
                on_breakpoint_go_to_source_action ();
            }
        }

        // right-clicking should pop up a context menu
        else if (a_event->type == GDK_BUTTON_PRESS) {
            if (a_event->button == 3) {
                popup_breakpoints_view_menu (a_event) ;
            }
        }

        NEMIVER_CATCH
    }

    void on_breakpoint_go_to_source_action ()
    {
        THROW_IF_FAIL(tree_view)
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection ();
        vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows ();
        if (paths.empty ())
            return ;
        Gtk::TreeModel::iterator tree_iter = list_store->get_iter (paths[0]) ;
        if (tree_iter) {
            go_to_breakpoint_signal.emit
                            ((*tree_iter)[get_bp_columns ().breakpoint]);
        }
    }

    void on_breakpoint_delete_action ()
    {
        THROW_IF_FAIL (tree_view)
        THROW_IF_FAIL (list_store) ;

        Glib::RefPtr<Gtk::TreeSelection> selection =
                                                tree_view->get_selection ();
        vector<Gtk::TreeModel::Path> paths = selection->get_selected_rows ();
        vector<Gtk::TreeModel::Path>::const_iterator it ;
        Gtk::TreeModel::iterator tree_iter ;
        for (it=paths.begin () ; it != paths.end (); ++it) {
            tree_iter = list_store->get_iter (*it) ;
            if (tree_iter) {
                debugger->delete_breakpoint
                                ((*tree_iter)[get_bp_columns ().id]);
            }
        }
    }

    void init_actions()
    {
        static ui_utils::ActionEntry s_breakpoints_action_entries [] = {
            {
                "DeleteBreakpointMenuItemAction",
                Gtk::Stock::DELETE,
                _("_Delete"),
                _("Remove this breakpoint"),
                sigc::mem_fun (*this, &Priv::on_breakpoint_delete_action),
                ui_utils::ActionEntry::DEFAULT,
                ""
            },
            {
                "GoToSourceBreakpointMenuItemAction",
                Gtk::Stock::JUMP_TO,
                _("_Go to Source"),
                _("Find this breakpoint in the source editor"),
                sigc::mem_fun (*this,
                        &Priv::on_breakpoint_go_to_source_action),
                ui_utils::ActionEntry::DEFAULT,
                ""
            }
        };

        breakpoints_action_group =
            Gtk::ActionGroup::create ("breakpoints-action-group") ;
        breakpoints_action_group->set_sensitive (true) ;

        int num_actions =
            sizeof (s_breakpoints_action_entries)/sizeof (ui_utils::ActionEntry) ;

        ui_utils::add_action_entries_to_action_group
                        (s_breakpoints_action_entries, num_actions,
                         breakpoints_action_group) ;

        workbench.get_ui_manager ()->insert_action_group
                                                (breakpoints_action_group);
    }

    void re_init ()
    {
        debugger->list_breakpoints ();
    }

};//end class BreakpointsView::Priv

BreakpointsView::BreakpointsView (IWorkbench& a_workbench,
        IPerspective& a_perspective, IDebuggerSafePtr& a_debugger)
{
    m_priv.reset (new Priv (a_workbench, a_perspective, a_debugger));
}

BreakpointsView::~BreakpointsView ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

Gtk::Widget&
BreakpointsView::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    THROW_IF_FAIL (m_priv->list_store) ;
    return *m_priv->tree_view ;
}

void
BreakpointsView::set_breakpoints
                (const std::map<int, IDebugger::BreakPoint> &a_breakpoints)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->set_breakpoints (a_breakpoints) ;
}

void
BreakpointsView::clear ()
{
    THROW_IF_FAIL (m_priv) ;
    if (m_priv->list_store) {
        m_priv->list_store->clear () ;
    }
}

void
BreakpointsView::re_init ()
{
    THROW_IF_FAIL (m_priv) ;
    clear () ;
    m_priv->re_init ();
}

sigc::signal<void, const IDebugger::BreakPoint&>&
BreakpointsView::go_to_breakpoint_signal () const
{
    THROW_IF_FAIL(m_priv);
    return m_priv->go_to_breakpoint_signal;
}

}//end namespace nemiver

