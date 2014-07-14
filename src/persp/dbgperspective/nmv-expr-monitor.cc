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
#include "nmv-expr-monitor.h"
#include <glib/gi18n.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treerowreference.h>
#include "common/nmv-exception.h"
#include "nmv-vars-treeview.h"
#include "nmv-variables-utils.h"
#include "nmv-debugger-utils.h"
#include "nmv-i-workbench.h"
#include "nmv-expr-inspector-dialog.h"

using namespace nemiver::common;
namespace vutils = nemiver::variables_utils2;
namespace dutils = nemiver::debugger_utils;

using namespace dutils;

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct ExprMonitor::Priv
{
    Glib::RefPtr<Gtk::UIManager> ui_manager;
    IDebugger &debugger;
    IPerspective &perspective;
    SafePtr<VarsTreeView> tree_view;
    Glib::RefPtr<Gtk::TreeStore> tree_store;
    SafePtr<Gtk::TreeRowReference> in_scope_exprs_row_ref;
    SafePtr<Gtk::TreeRowReference> out_of_scope_exprs_row_ref;
    Gtk::TreeModel::iterator cur_selected_row;
    IDebugger::VariableList monitored_expressions;
    IDebugger::VariableList changed_in_scope_exprs_at_prev_stop;
    IDebugger::VariableList changed_oo_scope_exprs_at_prev_stop;
    // Variables that went out of scope because the inferior got
    // restarted.
    IDebugger::VariableList killed_expressions;
    map<IDebugger::VariableSafePtr, bool> in_scope_exprs;
    map<IDebugger::VariableSafePtr, bool> revived_exprs;
    vector<Gtk::TreeModel::Path> selected_paths;
    Glib::RefPtr<Gtk::ActionGroup> action_group;
    Gtk::Widget *contextual_menu;
    IDebugger::Frame saved_frame;
    IDebugger::StopReason saved_reason;
    bool saved_has_frame;
    bool initialized;
    bool is_new_frame;
    bool is_up2date;

    Priv (IDebugger &a_debugger,
          IPerspective &a_perspective)
        : debugger (a_debugger),
          perspective (a_perspective),
          contextual_menu (0),
          saved_reason (IDebugger::UNDEFINED_REASON),
          saved_has_frame (false),
          initialized (false),
          is_new_frame (true),
          is_up2date (true)
    {
        // The widget is built lazily when somone requests it from
        // the outside.
    }

    /// Return the widget to visualize the variables managed by the
    /// monitor.  This function lazily builds the widget and
    /// initializes the monitor.
    /// \return the widget to visualize the monitored expressions
    Gtk::Widget&
    get_widget ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        if (!initialized)
            init_widget ();
        THROW_IF_FAIL (initialized && tree_view);
        return *tree_view;
    }

    void
    init_widget ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        if (initialized)
            return;

        // Neither in_scope_exprs_row_ref nor out_of_scope_exprs_row_ref
        // nor tree_view should be non-null before the widget has been
        // initialized.
        THROW_IF_FAIL (!in_scope_exprs_row_ref
                       && !out_of_scope_exprs_row_ref
                       && !tree_view);

        tree_view.reset (VarsTreeView::create ());
        THROW_IF_FAIL (tree_view);

        tree_store = tree_view->get_tree_store ();
        THROW_IF_FAIL (tree_store);

        // *************************************************************
        // Create a row for expressions that are in-scope and a row for
        // expressions that are out-of-scope
        // *************************************************************
        Gtk::TreeModel::iterator it = tree_store->append ();
        (*it)[vutils::get_variable_columns ().name] = _("In scope expressions");
        in_scope_exprs_row_ref.reset
            (new Gtk::TreeRowReference (tree_store,
                                        tree_store->get_path (it)));
        it = tree_store->append ();
        (*it)[vutils::get_variable_columns ().name] = _("Out of scope expressions");
        out_of_scope_exprs_row_ref.reset
            (new Gtk::TreeRowReference (tree_store,
                                        tree_store->get_path (it)));

        THROW_IF_FAIL (in_scope_exprs_row_ref
                       && out_of_scope_exprs_row_ref);

        // And now finish the initialization.
        connect_to_debugger_signal ();
        init_graphical_signals ();
        init_actions ();

        initialized = true;
    }

    /// Re-initialize the widget.
    void
    re_init_widget (bool a_remember_variables)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        if (a_remember_variables) {
            kill_monitored_expressions ();
        } else {
            killed_expressions.clear ();
        }
        monitored_expressions.clear ();
        clear_in_scope_exprs_rows ();
        clear_out_of_scope_exprs_rows ();
        revived_exprs.clear ();
    }

    /// Clear the rows under the "in scope variables" node.
    void
    clear_in_scope_exprs_rows ()
    {
        Gtk::TreeModel::iterator row_it;
        get_in_scope_exprs_row_iterator (row_it);
        Gtk::TreeModel::Children rows = row_it->children ();
        for (row_it = rows.begin (); row_it != rows.end ();)
            row_it = tree_store->erase (*row_it);
    }

    /// Clear the rows under the "out of scope variables" node.
    void
    clear_out_of_scope_exprs_rows ()
    {
        Gtk::TreeModel::iterator row_it;
        get_out_of_scope_exprs_row_iterator (row_it);
        Gtk::TreeModel::Children rows = row_it->children ();
        for (row_it = rows.begin (); row_it != rows.end ();)
            row_it = tree_store->erase (*row_it);
    }

    /// Connect to the graphical
    void
    connect_to_debugger_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        debugger.stopped_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_stopped_signal));
        debugger.inferior_re_run_signal ()
            .connect (sigc::mem_fun
                      (*this,
                       &Priv::on_inferior_re_run_signal));
    }

    /// Connect slot to signals related to graphical stuff.
    void
    init_graphical_signals ()
    {
        THROW_IF_FAIL (tree_view);

        tree_view->signal_row_expanded ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_row_expanded_signal));

        tree_view->signal_draw ().connect_notify
            (sigc::mem_fun (this, &Priv::on_draw_signal));

        // Schedule the button press signal handler to be run before
        // the default handler.
        tree_view->signal_button_press_event ().connect_notify
            (sigc::mem_fun (this, &Priv::on_button_press_signal));

        Glib::RefPtr<Gtk::TreeSelection> selection =
            tree_view->get_selection ();
        selection->set_mode (Gtk::SELECTION_MULTIPLE);

        selection->signal_changed ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_tree_view_selection_changed_signal));
    }

    /// Initialize actions to be triggered whenever the user clicks on
    /// menu items that are specific to variable monitoring.
    void
    init_actions ()
    {
        ui_utils::ActionEntry s_expr_monitor_action_entries [] = {
            {
                "RemoveExpressionsMenuItemAction",
                Gtk::Stock::DELETE,
                _("Remove"),
                _("Remove selected expressions from the monitor"),
                sigc::mem_fun (*this, &Priv::on_remove_expressions_action),
                ui_utils::ActionEntry::DEFAULT,
                "",
                false
            },
            {
                "AddExpressionMenuItemAction",
                Gtk::Stock::ADD,
                _("New..."),
                _("Add a new expression to the monitor"),
                sigc::mem_fun (*this, &Priv::on_add_expression_action),
                ui_utils::ActionEntry::DEFAULT,
                "",
                false
            }
        };
        action_group =
            Gtk::ActionGroup::create ("expr-monitor-action-group");
        action_group->set_sensitive (true);
        int num_actions =
            sizeof (s_expr_monitor_action_entries)
            /
            sizeof (ui_utils::ActionEntry);
        ui_utils::add_action_entries_to_action_group
            (s_expr_monitor_action_entries,
             num_actions,
             action_group);
        get_ui_manager ()->insert_action_group (action_group);
    }

    /// Return true iff the variable in parameter is currently being
    /// monitored.
    ///
    /// \param a_expr the variable to check for.
    bool
    expression_is_monitored (const IDebugger::Variable &a_expr) const
    {
        IDebugger::VariableList::const_iterator it;
        for (it = monitored_expressions.begin ();
             it != monitored_expressions.end ();
             ++it) {
            if (!a_expr.internal_name ().empty ()
                && a_expr.internal_name () == (*it)->internal_name ())
                // Both variables have the same internal name, so they
                // are equal.
                return true;
            else if (!(*it)->needs_unfolding ()
                     && !a_expr.needs_unfolding ()) {
                // Both variables have been unfolded, so we can
                // compare them by value.
                if ((*it)->equals_by_value (a_expr))
                    return true;
            } else {
                if (a_expr.name () == a_expr.name ())
                    return true;
            }
        }
        return false;
    }

    /// Return true iff the variable has been put out of scope because
    /// the inferior was re-started.  Pointer comparison is used for
    /// the test.
    bool
    expression_is_killed (IDebugger::VariableSafePtr a_expr)
    {
        for (IDebugger::VariableList::const_iterator i =
                 killed_expressions.begin ();
             i != killed_expressions.end ();
             ++i) {
            if (*i == a_expr)
                return true;
        }
        return false;
    }

    /// Return true iff a_expr was killed and has been revived.
    bool
    expression_is_revived (IDebugger::VariableSafePtr a_expr)
    {
        return revived_exprs.find (a_expr) != revived_exprs.end ();
    }

    /// Monitor a new expression (or varibale).  In other words, add a
    /// new variable to the monitor.
    ///
    /// \param a_expr the variable to monitor.
    void
    add_expression (const IDebugger::VariableSafePtr a_expr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        LOG_DD ("a_expr: " << a_expr->id ());

        if (!a_expr || expression_is_monitored (*a_expr))
            return;

        monitored_expressions.push_back (a_expr);
        Gtk::TreeModel::iterator root_node;
        if (a_expr->in_scope ())
            get_in_scope_exprs_row_iterator (root_node);
        else
            get_out_of_scope_exprs_row_iterator (root_node);
        THROW_IF_FAIL (root_node);
        vutils::append_a_variable (a_expr,
                                   *tree_view,
                                   root_node,
                                   /*a_truncate_type=*/true);
    }

    /// Add an variable for an expression to the monitor.
    ///
    /// \param a_expr the expression to consider.
    ///
    /// \param a_slot the callback slot to invoke upon creation of the
    /// variable object representing the expression.
    void
    add_expression (const UString &a_expr,
                    const IDebugger::ConstVariableSlot &a_slot)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        debugger.create_variable (a_expr, a_slot);
    }

    /// Monitor a list of new variables.
    ///
    /// \param a_exprs the variables to monitor.
    void
    add_expressions (const IDebugger::VariableList &a_exprs)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        IDebugger::VariableList::const_iterator it = a_exprs.begin ();
        for (; it != a_exprs.end (); ++it)
            add_expression (*it);
    }

    /// Remove a variable from the monitor.
    ///
    /// \param a_expr the variable to remove from the monitor.
    void
    remove_expression (const IDebugger::VariableSafePtr a_expr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        LOG_DD ("a_expr: " << a_expr->id ());

        bool found = false;
        IDebugger::VariableList::iterator it;
        for (it = monitored_expressions.begin ();
             it != monitored_expressions.end (); ++it) {
            if (*it == a_expr
                || (*it)->internal_name () == a_expr->internal_name ()) {
                // Remove the graphical representation of the node,
                // and then remove the node itself from the list of
                // monitored expressions.
                Gtk::TreeModel::iterator parent_row;
                if (a_expr->in_scope ())
                    get_in_scope_exprs_row_iterator (parent_row);
                else
                    get_out_of_scope_exprs_row_iterator (parent_row);
                THROW_IF_FAIL (parent_row);
                THROW_IF_FAIL (vutils::unlink_a_variable_row
                               (a_expr, tree_store, parent_row));
                monitored_expressions.erase (it);
                // We removed an element from the array while
                // iterating on it so the iterator is invalidated.  We
                // must not use it again.
                found = true;
                break;                
            }
        }

        // Remove the expression from the list of killed expressions
        // too.
        for (it = killed_expressions.begin ();
             it != killed_expressions.end ();
             ++it) {
            if (*it == a_expr
                || (*it)->internal_name () == a_expr->internal_name ()) {
                Gtk::TreeModel::iterator parent_row;
                if (a_expr->in_scope ())
                    get_in_scope_exprs_row_iterator (parent_row);
                else
                    get_out_of_scope_exprs_row_iterator (parent_row);
                THROW_IF_FAIL (parent_row);
                THROW_IF_FAIL (vutils::unlink_a_variable_row
                               (a_expr, tree_store, parent_row));
                found = true;
                killed_expressions.erase (it);
                // it is now invalidated, we must not use it
                // again.
                break;
            }
        }

        // Make sure the expression is not stored in the other lists
        // either.
        for (it = changed_in_scope_exprs_at_prev_stop.begin ();
             it != changed_in_scope_exprs_at_prev_stop.end ();
             ++it) {
            if (*it == a_expr
                || (*it)->internal_name () == a_expr->internal_name ()) {
                changed_in_scope_exprs_at_prev_stop.erase (it);
                break;
            }
        }

        for (it = changed_oo_scope_exprs_at_prev_stop.begin ();
             it != changed_oo_scope_exprs_at_prev_stop.end ();
             ++it) {
            if (*it == a_expr
                || (*it)->internal_name () == a_expr->internal_name ()) {
                changed_oo_scope_exprs_at_prev_stop.erase (it);
                break;
            }
        }

        for (map<IDebugger::VariableSafePtr, bool>::iterator i =
                 revived_exprs.begin ();
             i != revived_exprs.end ();
             ++i) {
            if (i->first->internal_name () == a_expr->internal_name ()) {
                revived_exprs.erase (i);
                break;
            }
        }

        if (found) {
            LOG_DD ("variable found and erased");
        } else {
            LOG_DD ("variable was *NOT* found");
        }
    }

    /// Remove a set of variables from the monitor.
    void
    remove_expressions (const IDebugger::VariableList &a_exprs)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        IDebugger::VariableList::const_iterator it = a_exprs.begin ();
        for (; it != a_exprs.end (); ++it)
            remove_expression (*it);
    }

    /// Copy all the expressions being currently monitored, into the
    /// list of killed expressions.  This happens when all the
    /// monitored expressions go out of scope in a way that is not
    /// tracked by the debugging engine anymore; e.g, when the user
    /// restarts the infererior, or when the inferior exits.  In that
    /// case, these expressions must be automatically re-created when
    /// the inferior starts again; the killed expressions then become
    /// "revived" expressions.
    void
    kill_monitored_expressions ()
    {
        for (IDebugger::VariableList::const_iterator i =
                 monitored_expressions.begin ();
             i != monitored_expressions.end ();
             ++i) {
            // Sometimes the debugging backend gets this wrong.
            (*i)->in_scope (false);
            killed_expressions.push_back (*i);
        }
    }

    /// Re-monitor a killed expression.
    /// A killed expression is one that went out of scope because the
    /// inferior was re-started.
    void
    re_monitor_killed_variable (const IDebugger::VariableSafePtr a_expr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        THROW_IF_FAIL (!a_expr->name ().empty ());
        THROW_IF_FAIL (expression_is_killed (a_expr));

        // Update the graphical stuff according to the in-scope-ness
        // propert of a_expr.
        Gtk::TreeModel::iterator var_it, parent_it;
        update_expr_in_scope_or_not (a_expr, var_it, parent_it);

        if (!a_expr->in_scope ())
            add_expression
                (a_expr->name (),
                 sigc::bind (sigc::mem_fun
                             (*this, &Priv::on_killed_var_recreated),
                             a_expr));
    }

    /// Walk the killed expressions and try to re-monitor them.
    /// killed expressions are those that went out of scope because
    /// the inferior was re-started.
    void
    re_monitor_killed_variables ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        for (IDebugger::VariableList::const_iterator it =
                 killed_expressions.begin ();
             it != killed_expressions.end ();
             ++it)
            re_monitor_killed_variable (*it);
    }

    /// Update the graphical representation of the in-scope-ness of a
    /// revived variable.  A re-vived variable is a variable that has
    /// been killed, and that has been re-created.  Look at the
    /// comments of kill_monitored_expressions to understand what a
    /// killed expression is.
    void
    update_revived_exprs_oo_scope_or_not ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        IDebugger::VariableList to_delete;

        for (map<IDebugger::VariableSafePtr, bool>::const_iterator i =
                 revived_exprs.begin ();
             i != revived_exprs.end ();
             ++i) {
            if (!i->first->in_scope ())
                debugger.create_variable
                    (i->first->name (),
                     sigc::bind
                     (sigc::mem_fun
                      (*this, &Priv::on_tentatively_create_revived_expr),
                      i->first));
            else
                to_delete.push_back (i->first);
                
        }

        for (IDebugger::VariableList::iterator i = to_delete.begin ();
             i != to_delete.end ();
             ++i)
            revived_exprs.erase (*i);
    }

    /// Return an iterator on the monitored expressions that are in
    /// scope.
    bool
    get_in_scope_exprs_row_iterator (Gtk::TreeModel::iterator &a_it)
    {
        if (!in_scope_exprs_row_ref)
            return false;
        a_it = tree_store->get_iter (in_scope_exprs_row_ref->get_path ());
        return true;
    }

    /// Return an iterator on the monitored expressions that are out of
    /// scope.
    bool
    get_out_of_scope_exprs_row_iterator (Gtk::TreeModel::iterator &a_it)
    {
        if (!out_of_scope_exprs_row_ref)
            return false;
        a_it = tree_store->get_iter (out_of_scope_exprs_row_ref->get_path ());
        return true;
    }

    /// Ensure that a given expression is only under a given graphical
    /// node.
    ///
    /// This is a sub-routine for update_expr_in_scope_or_not.
    /// \param a_expr the variable to consider
    ///
    /// \param a_first the node under which we want a_expr to be.
    ///
    /// \param a_second the node under which we *don't* want a_expr to
    /// be.
    ///
    /// \param a_expr_it a graphical iterator pointing to where a_expr
    /// was put, finally.
    void
    ensure_expr_under_first_but_not_under_second
    (const IDebugger::VariableSafePtr a_expr,
     Gtk::TreeModel::iterator &a_first,
     Gtk::TreeModel::iterator &a_second,
     Gtk::TreeModel::iterator &a_expr_it)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        // If is under a_second then remove it from there.
        vutils::unlink_a_variable_row (a_expr, tree_store, a_second);
        
        // If a_expr is not in under a_first, add it now.
        Gtk::TreeModel::iterator var_it;
        if (!vutils::find_a_variable (a_expr, a_first, a_expr_it)) {
            LOG_DD ("Adding variable "
                    << a_expr->id ()
                    << " under the first iterator");
            vutils::append_a_variable (a_expr, *tree_view,
                                       a_first, a_expr_it,
                                       /*a_truncate_type=*/true);
        }
    }

    /// Update the graphical state of a_expr so that, it appears under
    /// the right graphical node depending on whether it is in scope
    /// or not.
    void
    update_expr_in_scope_or_not (const IDebugger::VariableSafePtr &a_expr,
                                Gtk::TreeModel::iterator &a_expr_it,
                                Gtk::TreeModel::iterator &a_parent_it)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        LOG_DD ("a_expr: " << a_expr->id ());

        Gtk::TreeModel::iterator in_scope_exprs_it, out_of_scope_exprs_it;

        THROW_IF_FAIL (a_expr->is_morally_root ());

        get_in_scope_exprs_row_iterator (in_scope_exprs_it);
        get_out_of_scope_exprs_row_iterator (out_of_scope_exprs_it);

        if (a_expr->in_scope ()) {
            LOG_DD ("variable " << a_expr->id () << " is in scope");
            in_scope_exprs[a_expr] = true;
            ensure_expr_under_first_but_not_under_second (a_expr,
                                                         in_scope_exprs_it,
                                                         out_of_scope_exprs_it,
                                                         a_expr_it);
            a_parent_it = in_scope_exprs_it;
        } else {
            LOG_DD ("variable " << a_expr->id () << " is not in scope");
            in_scope_exprs.erase (a_expr);
            ensure_expr_under_first_but_not_under_second (a_expr,
                                                         out_of_scope_exprs_it,
                                                         in_scope_exprs_it,
                                                         a_expr_it);
            a_parent_it = out_of_scope_exprs_it;
        }

    }

    /// Clear the internal state we maintain to know what variables
    /// changed at the previous step.
    void
    clear_exprs_changed_at_prev_step ()
    {
        changed_in_scope_exprs_at_prev_stop.clear ();
        changed_oo_scope_exprs_at_prev_stop.clear ();
    }

    /// Graphically update the variables that changed at the previous
    /// step.
    void
    update_exprs_changed_at_prev_step ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        Gtk::TreeModel::iterator in_scope_exprs_it, oo_scope_exprs_it;

        get_in_scope_exprs_row_iterator (in_scope_exprs_it);
        get_out_of_scope_exprs_row_iterator (oo_scope_exprs_it);

        IDebugger::VariableList::const_iterator it;
        for (it = changed_in_scope_exprs_at_prev_stop.begin ();
             it != changed_in_scope_exprs_at_prev_stop.end ();
             ++it) {
            if (!expression_is_killed (*it))
                vutils::update_a_variable (*it, *tree_view,
                                           in_scope_exprs_it,
                                           /*a_truncate_type=*/false,
                                           /*a_handle_highlight=*/true,
                                           /*a_is_new_frame=*/true,
                                           /*a_update_members=*/true);
        }

        for (it = changed_oo_scope_exprs_at_prev_stop.begin ();
             it != changed_oo_scope_exprs_at_prev_stop.end ();
             ++it) {
            if (!expression_is_killed (*it))
                vutils::update_a_variable (*it, *tree_view,
                                           oo_scope_exprs_it,
                                           /*a_truncate_type=*/false,
                                           /*a_handle_highlight=*/true,
                                           /*a_is_new_frame=*/true,
                                           /*a_update_members=*/true);
        }
        clear_exprs_changed_at_prev_step ();
    }

    /// Return true iff we need to graphically update the current
    /// view.  Basically, the widget is not visible, this function
    /// returns false.
    bool
    should_process_now () const
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        THROW_IF_FAIL (tree_view);
        bool is_visible = tree_view->get_is_drawable ();
        LOG_DD ("is visible: " << is_visible);
        return is_visible;
    }

    /// This does what needs to do whenever the widget becomes visible
    /// and we need to update its rendering after the inferior has
    /// stopped.
    ///
    /// \param a_reason the reason why the inferior has stopped.
    ///
    /// \param a_has_frame true if after the stop, we have a frame
    /// handy.
    ///
    /// \param a_frame the frame we have, if a_has_frame is non-null.
    void
    finish_handling_debugger_stopped_event (IDebugger::StopReason a_reason,
                                            bool a_has_frame,
                                            const IDebugger::Frame &a_frame)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        THROW_IF_FAIL (tree_store);

        LOG_DD ("stopped, reason: " << a_reason);
        if (a_reason == IDebugger::EXITED_SIGNALLED
            || a_reason == IDebugger::EXITED_NORMALLY
            || a_reason == IDebugger::EXITED
            || !a_has_frame) {
            kill_monitored_expressions ();
            return;
        }

        is_new_frame = (saved_frame != a_frame);
        saved_frame = a_frame;
        
        // Clear the highlighting from variables that where
        // highlighted during previous step.
        update_exprs_changed_at_prev_step ();

        // Walk the monitored expressions and list those that have
        // changed.
        IDebugger::VariableList::const_iterator it;
        for (it = monitored_expressions.begin ();
             it != monitored_expressions.end ();
             ++it) {
            debugger.list_changed_variables (*it,
                                             sigc::bind
                                             (sigc::mem_fun
                                              (*this, &Priv::on_vars_changed),
                                              *it));
        }

        // Walk the killed expressions and try to re-monitor them.
        // killed expressions are those that went out of scope because
        // the inferior was re-started.
        re_monitor_killed_variables ();

        update_revived_exprs_oo_scope_or_not ();

        NEMIVER_CATCH;
    }

    /// Return the UI manager associated with this variable monitor.
    /// this is for e.g, the contextual menu.
    Glib::RefPtr<Gtk::UIManager>
    get_ui_manager ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        if (!ui_manager)
            ui_manager = Gtk::UIManager::create ();
        return ui_manager;
    }

    /// Return the contextual menu for the variable menu.  Build it,
    /// if it is not already built.
    ///
    /// \return a widget representing the contextual menu.
    Gtk::Widget*
    get_contextual_menu ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        if (!contextual_menu) {
            string absolute_path;
            perspective.build_absolute_resource_path
                (Glib::build_filename ("menus", "exprmonitorpopup.xml"),
                 absolute_path);
            get_ui_manager ()->add_ui_from_file (absolute_path);
            get_ui_manager ()->ensure_update ();
            contextual_menu =
                get_ui_manager ()->get_widget ("/ExprMonitorPopup");
            THROW_IF_FAIL (contextual_menu);
        }
        return contextual_menu;
    }

    /// Return true iff a given path pointing at a row of the variable
    /// monitor is for a row that contains an instance of
    /// IDebugger::Variable.
    /// 
    /// \param a_path the path to check for.
    bool
    path_points_to_variable (const Gtk::TreeModel::Path &a_path) const
    {
        Gtk::TreeModel::iterator it = tree_store->get_iter (a_path);
        if (it->get_value (vutils::get_variable_columns ().variable))
            return true;
        return false;
    }

    /// Return true iff a row containing an instance of
    /// IDebugger::Variable has been selected (clicked) by the user.
    bool
    expression_is_selected () const
    {
        std::vector<Gtk::TreeModel::Path> selected_paths =
            tree_view->get_selection ()->get_selected_rows ();
        std::vector<Gtk::TreeModel::Path>::const_iterator it;
        for (it = selected_paths.begin ();
             it != selected_paths.end ();
             ++it)
            if (path_points_to_variable (*it))
                return true;
        return false;
    }

    /// Update the sensitivity of the items of the contextual menu,
    /// depending on the set of rows that are currently selected.
    void
    update_contextual_menu_sensitivity ()
    {
        Glib::RefPtr<Gtk::Action> remove_expression_action =
            get_ui_manager ()->get_action
            ("/ExprMonitorPopup/RemoveExpressionsMenuItem");
        THROW_IF_FAIL (remove_expression_action);

        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection ();
        THROW_IF_FAIL (selection);

        remove_expression_action->set_sensitive
            (expression_is_selected ());
    }

    /// Pop up the contextual menu of the variable monitor.
    void
    popup_contextual_menu (GdkEventButton *a_event)
    {
        Gtk::Menu *menu = dynamic_cast<Gtk::Menu*> (get_contextual_menu ());
        THROW_IF_FAIL (menu);

        update_contextual_menu_sensitivity ();

        menu->popup (a_event->button, a_event->time);
    }

    // *********************
    // <signal handlers>
    // ********************

    /// Invoked whenever the inferior stops.
    void
    on_stopped_signal (IDebugger::StopReason a_reason,
                       bool a_has_frame,
                       const IDebugger::Frame &a_frame,
                       int /*a_thread_id*/,
                       const string& /*a_bp_num*/,
                       const UString &/*a_cookie*/)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        if (IDebugger::is_exited (a_reason)
            || !a_has_frame)
            return;

        saved_frame = a_frame;
        saved_reason = a_reason;
        saved_has_frame = a_has_frame;

        if (should_process_now ()) {
            finish_handling_debugger_stopped_event (a_reason,
                                                    a_has_frame,
                                                    a_frame);
        } else {
            is_up2date = false;
        }
        NEMIVER_CATCH;
    }

    /// Callback invoked whenever the inferior is re-run.
    void
    on_inferior_re_run_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
    }

    /// Callback invoked whenever a killed expression (made out of scope because
    /// the inferior was restarted), is re-created again, for the
    /// purpose of being monitored again.
    ///
    /// So this function monitors the variable again, and removes it
    /// from the previously killed expressions.
    void
    on_killed_var_recreated (const IDebugger::VariableSafePtr a_new_var,
                             const IDebugger::VariableSafePtr a_killed_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        THROW_IF_FAIL (a_new_var);
        THROW_IF_FAIL (a_killed_var);

        remove_expression (a_killed_var);
        add_expression (a_new_var);
        // Mark a_new_var as a revived one.
        revived_exprs[a_new_var] = true;

        NEMIVER_CATCH;
    }

    void
    on_tentatively_create_revived_expr
    (const IDebugger::VariableSafePtr a_new_expr,
     const IDebugger::VariableSafePtr a_initial_revived_expr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        if (a_new_expr->in_scope ()
            && !a_initial_revived_expr->in_scope ()) {
            remove_expression (a_initial_revived_expr);
            add_expression (a_new_expr);
        }
        // else the expression a_new_expr is just deleted.

        NEMIVER_CATCH;
    }

    /// Invoked whenever a variable changed.
    ///
    /// \a_param the sub variables that actually changed.
    ///
    /// \param the variable which sub-variables changed.
    void
    on_vars_changed (const IDebugger::VariableList &a_sub_vars,
                     const IDebugger::VariableSafePtr a_var_root)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        LOG_DD ("a_var_root: "<< a_var_root->id ());

        NEMIVER_TRY;

        // Is this variable in scope or not? Update the graphical
        // stuff according to that property.
        Gtk::TreeModel::iterator var_it, parent_it;
        update_expr_in_scope_or_not (a_var_root, var_it, parent_it);
        THROW_IF_FAIL (var_it);

        // Walk children of a_var_root and update their graphical
        // representation.
        if (!expression_is_killed (a_var_root)) {
            IDebugger::VariableList::const_iterator v = a_sub_vars.begin ();
            for (; v != a_sub_vars.end (); ++v) {
                LOG_DD ("Going to update variable " << (*v)->id () << ":" << **v);
                vutils::update_a_variable (*v, *tree_view,
                                           parent_it,
                                           /*a_truncate_type=*/false,
                                           /*a_handle_highlight=*/true,
                                           /*a_is_new_frame=*/is_new_frame,
                                           /*a_update_members=*/false);
            }
        }

        // Now, update changed_in_scope_exprs_at_prev_stop and
        // changed_oo_scope_exprs_at_prev_stop variables.
        Gtk::TreeModel::iterator in_scope_exprs_row_it, oo_scope_exprs_row_it;
        get_in_scope_exprs_row_iterator (in_scope_exprs_row_it);
        get_out_of_scope_exprs_row_iterator (oo_scope_exprs_row_it);
        if (parent_it == in_scope_exprs_row_it) {
            changed_in_scope_exprs_at_prev_stop.push_back (a_var_root);
        } else {
            THROW_IF_FAIL (parent_it == oo_scope_exprs_row_it);
            changed_oo_scope_exprs_at_prev_stop.push_back (a_var_root);
        }
        NEMIVER_CATCH;
    }

    /// Invoked whenever a graphical node has been (graphically)
    /// expanded.
    ///
    /// \param a_it an iterator on the row that got expanded.
    ///
    /// \param a_path a path to the row that got expanded.
    void
    on_tree_view_row_expanded_signal (const Gtk::TreeModel::iterator &a_it,
                                      const Gtk::TreeModel::Path &a_path)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        if (!(*a_it)[vutils::get_variable_columns ().needs_unfolding]) {
            return;
        }
        LOG_DD ("A variable needs unfolding");

        IDebugger::VariableSafePtr var =
            (*a_it)[vutils::get_variable_columns ().variable];
        debugger.unfold_variable
            (var,
             sigc::bind  (sigc::mem_fun (*this,
                                         &Priv::on_variable_unfolded_signal),
                          a_path));

        NEMIVER_CATCH;
    }

    /// Invoked when a variable is unfolded.
    ///
    /// Usually the variable is unfolded the first time its graphical
    /// node is expanded.
    ///
    /// \param a_expr the variable we are looking at.
    ///
    /// \param a_expr_node the graphical node for a_expr.
    void
    on_variable_unfolded_signal (const IDebugger::VariableSafePtr a_expr,
                                 const Gtk::TreeModel::Path a_expr_node)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        Gtk::TreeModel::iterator var_it = tree_store->get_iter (a_expr_node);
        vutils::update_unfolded_variable (a_expr,
                                          *tree_view,
                                          var_it,
                                          false /* do not truncate type */);
        tree_view->expand_row (a_expr_node, false);
        NEMIVER_CATCH;
    }

    /// Invoked whenever the current view (widget) is drawn on
    /// screen.  That is, when it becomes visible.
    void
    on_draw_signal (const Cairo::RefPtr<Cairo::Context> &)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY;
        if (!is_up2date) {
            finish_handling_debugger_stopped_event (saved_reason,
                                                    saved_has_frame,
                                                    saved_frame);
            is_up2date = true;
        }
        NEMIVER_CATCH;
    }

    /// Callback function called whenever the user presses button from
    /// either the keyboard or the mousse.
    void
    on_button_press_signal (GdkEventButton *a_event)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        // Right-clicking should pop up a context menu
        if (a_event->type == GDK_BUTTON_PRESS
            && a_event->button == 3)
            popup_contextual_menu (a_event);

        NEMIVER_CATCH;
    }

    /// Callback called whenever the user clicks on a menu item to
    /// remove the set of currently selected variables.
    void
    on_remove_expressions_action ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection ();
        THROW_IF_FAIL (selection);

        std::vector<Gtk::TreeModel::Path> selected_rows =
            selection->get_selected_rows ();

        std::list<IDebugger::VariableSafePtr> delete_list;
        for (std::vector<Gtk::TreeModel::Path>::const_iterator it =
                 selected_rows.begin ();
             it != selected_rows.end ();
             ++it) {
            Gtk::TreeModel::iterator i = tree_store->get_iter (*it);
            IDebugger::VariableSafePtr cur_var =
                (*i)[vutils::get_variable_columns ().variable];
            THROW_IF_FAIL (cur_var);
            delete_list.push_back (cur_var->root ());
        }
        for (std::list<IDebugger::VariableSafePtr>::const_iterator it =
                 delete_list.begin ();
             it != delete_list.end ();
             ++it) {
            remove_expression (*it);
        }

        NEMIVER_CATCH;
    }

    /// Callback function invoked whenever the user clicks on a menu
    /// item to monitor a new expression.
    void
    on_add_expression_action ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        ExprInspectorDialog dialog
            (perspective.get_workbench ().get_root_window(),
             debugger, perspective);

        dialog.expr_monitoring_requested ().connect
            (sigc::mem_fun (*this,
                            &ExprMonitor::Priv::on_expr_monitoring_requested));
        dialog.inspector ().expr_inspected_signal ().connect
            (sigc::bind (sigc::mem_fun (*this,
                                        &ExprMonitor::Priv::on_expr_inspected),
                         &dialog));
        dialog.run ();
    }

    /// Callback function invoked whenever an expression has been been
    /// inspected.  This is usuall triggered when the user clicks on
    /// the the "inspect" button in the ExprInspectorDialog dialog.
    ///
    /// \param a_expr the expression that got recently inspected
    ///
    /// \param a_dialog the dialog this variable monitor belongs to.
    void
    on_expr_inspected (const IDebugger::VariableSafePtr a_expr,
                       ExprInspectorDialog *a_dialog)
    {
        if (expression_is_monitored (*a_expr))
        {
            a_dialog->functionality_mask
                (a_dialog->functionality_mask ()
                 & ~ExprInspectorDialog::FUNCTIONALITY_EXPR_MONITOR_PICKER);
        }
        else
        {
            a_dialog->functionality_mask
                (a_dialog->functionality_mask ()
                 | ExprInspectorDialog::FUNCTIONALITY_EXPR_MONITOR_PICKER);
        }
    }

    /// Callback function invoked whenever the user clicks on the
    /// "monitor variable" button in the dialog launched by
    /// on_add_expression_action above.
    void
    on_expr_monitoring_requested (const IDebugger::VariableSafePtr a_expr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        add_expression (a_expr);
    }

    /// Callback function invoked whenever the user selects or
    /// unselects rows of the variable monitor.
    void
    on_tree_view_selection_changed_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        THROW_IF_FAIL (tree_view);
        THROW_IF_FAIL (tree_store);

        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection ();
        THROW_IF_FAIL (selection);

        selected_paths =
            selection->get_selected_rows ();

        NEMIVER_CATCH;
    }

    // *********************
    // </signal handlers>
    // ********************

}; // end struct ExprMonitor

ExprMonitor::ExprMonitor (IDebugger &a_dbg,
                          IPerspective &a_perspective)
{
    m_priv.reset (new Priv (a_dbg, a_perspective));
}

ExprMonitor::~ExprMonitor ()
{
}

/// Return the widget for this type.
Gtk::Widget&
ExprMonitor::widget ()
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_widget ();
}

/// Monitor a new variable.  IOW, add a new variable to the monitor.
///
/// \param a_expr the new variable to monitor.
void
ExprMonitor::add_expression (const IDebugger::VariableSafePtr a_expr)
{
    m_priv->add_expression (a_expr);
}

/// Monitor a list of new variables.
///
/// \param a_exprs the variables to monitor.
void
ExprMonitor::add_expressions (const IDebugger::VariableList &a_exprs)
{
    m_priv->add_expressions (a_exprs);
}

/// Return true iff the given variable is being monitored by this
/// monitored.
///
/// \param a_expr the variable to check for.
bool
ExprMonitor::expression_is_monitored (const IDebugger::Variable &a_expr) const
{
    return m_priv->expression_is_monitored (a_expr);
}

/// Remove a variable from the monitor.
///
/// \param a_expr the variable to remove from the monitor.
void
ExprMonitor::remove_expression (const IDebugger::VariableSafePtr a_expr)
{
    m_priv->remove_expression (a_expr);
}

/// Remove a list of variables from the monitor.
///
/// \param a_exprs the list of variables to remove.
void
ExprMonitor::remove_expressions (const std::list<IDebugger::VariableSafePtr> &a_exprs)
{
    m_priv->remove_expressions (a_exprs);
}

/// Clear the widget.
void
ExprMonitor::re_init_widget (bool a_remember_variables)
{
    m_priv->re_init_widget (a_remember_variables);
}

NEMIVER_END_NAMESPACE (nemiver)
