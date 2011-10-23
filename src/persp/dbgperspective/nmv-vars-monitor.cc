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
#include "nmv-vars-monitor.h"
#include <glib/gi18n.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treerowreference.h>
#include "common/nmv-exception.h"
#include "nmv-vars-treeview.h"
#include "nmv-variables-utils.h"

using namespace nemiver::common;
namespace vutils = nemiver::variables_utils2;

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct VarsMonitor::Priv
{
    IDebugger &debugger;
    IPerspective &perspective;
    SafePtr<VarsTreeView> tree_view;
    Glib::RefPtr<Gtk::TreeStore> tree_store;
    SafePtr<Gtk::TreeRowReference> in_scope_vars_row_ref;
    SafePtr<Gtk::TreeRowReference> out_of_scope_vars_row_ref;
    Gtk::TreeModel::iterator cur_selected_row;
    bool initialized;
    IDebugger::VariableList monitored_variables;
    map<IDebugger::VariableSafePtr, bool> in_scope_vars;

    Priv (IDebugger &a_debugger,
          IPerspective &a_perspective)
        : debugger (a_debugger),
          perspective (a_perspective),
          initialized (false)
    {
        // The widget is built lazily when somone requests it from
        // the outside.
    }

    /// Return the widget to visualize the variables managed by the
    /// monitor.  This function lazily builds the widget and
    /// initializes the monitor.
    /// \return the widget to visualize the monitored variables
    Gtk::Widget&
    get_widget ()
    {
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

        // Neither in_scope_vars_row_ref nor out_of_scope_vars_row_ref
        // nor tree_view should be non-null before the widget has been
        // initialized.
        THROW_IF_FAIL (!in_scope_vars_row_ref
                       && !out_of_scope_vars_row_ref
                       && !tree_view);

        tree_view.reset (VarsTreeView::create ());
        THROW_IF_FAIL (tree_view);

        tree_store = tree_view->get_tree_store ();
        THROW_IF_FAIL (tree_store);

        // *************************************************************
        // Create a row for variables that are in-scope and a row for
        // variables that are out-of-scope
        // *************************************************************
        Gtk::TreeModel::iterator it = tree_store->append ();
        (*it)[vutils::get_variable_columns ().name] = _("In scope variables");
        in_scope_vars_row_ref.reset
            (new Gtk::TreeRowReference (tree_store,
                                        tree_store->get_path (it)));
        it = tree_store->append ();
        (*it)[vutils::get_variable_columns ().name] = _("Out of scope variables");
        out_of_scope_vars_row_ref.reset
            (new Gtk::TreeRowReference (tree_store,
                                        tree_store->get_path (it)));

        THROW_IF_FAIL (in_scope_vars_row_ref
                       && out_of_scope_vars_row_ref);

        // And now finish the initialization.
        connect_to_debugger_signal ();
        init_graphical_signals ();
        init_actions ();
        re_init_widget ();

        initialized = true;
    }

    void
    re_init_widget ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
    }

    void
    connect_to_debugger_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        debugger.stopped_signal ().connect
            (sigc::mem_fun (*this, &Priv::on_stopped_signal));
    }

    void
    init_graphical_signals ()
    {
    }

    void
    init_actions ()
    {
    }

    void
    add_variable (const IDebugger::VariableSafePtr a_var)
    {
        if (a_var)
            monitored_variables.push_back (a_var);
    }

    void
    add_variables (const IDebugger::VariableList &a_vars)
    {
        IDebugger::VariableList::const_iterator it;
        for (it = a_vars.begin (); it != a_vars.end (); ++it)
            monitored_variables.push_back (*it);
    }

    bool
    get_in_scope_vars_row_iterator (Gtk::TreeModel::iterator &a_it)
    {
        if (!in_scope_vars_row_ref)
            return false;
        a_it = tree_store->get_iter (in_scope_vars_row_ref->get_path ());
        return true;
    }

    bool
    get_out_of_scope_vars_row_iterator (Gtk::TreeModel::iterator &a_it)
    {
        if (!out_of_scope_vars_row_ref)
            return false;
        a_it = tree_store->get_iter (out_of_scope_vars_row_ref->get_path ());
        return true;
    }

    void
    ensure_var_under_first_but_not_under_second
    (const IDebugger::VariableSafePtr &a_var,
     Gtk::TreeModel::iterator &a_first,
     Gtk::TreeModel::iterator &a_second,
     Gtk::TreeModel::iterator &a_var_it)
    {
        // If is under a_second then remove it from there.
        vutils::unlink_a_variable_row (a_var, tree_store, a_second);
        
        // If a_var is not in under a_first, add it now.
        Gtk::TreeModel::iterator var_it;
        if (!vutils::find_a_variable (a_var, a_first, a_var_it))
            vutils::append_a_variable (a_var, *tree_view,
                                       tree_store, a_first, a_var_it,
                                       /*a_truncate_type=*/true);
    }

    void
    update_var_in_scope_or_not (const IDebugger::VariableSafePtr &a_var,
                                Gtk::TreeModel::iterator &a_var_it)
    {
        Gtk::TreeModel::iterator in_scope_vars_it, out_of_scope_vars_it;

        if (!a_var->is_morally_root ())
            return;

        get_in_scope_vars_row_iterator (in_scope_vars_it);
        get_out_of_scope_vars_row_iterator (out_of_scope_vars_it);

        if (a_var->in_scope ()) {
            in_scope_vars[a_var] = true;
            ensure_var_under_first_but_not_under_second (a_var,
                                                         in_scope_vars_it,
                                                         out_of_scope_vars_it,
                                                         a_var_it);
        } else {
            in_scope_vars.erase (a_var);
            ensure_var_under_first_but_not_under_second (a_var,
                                                         out_of_scope_vars_it,
                                                         in_scope_vars_it,
                                                         a_var_it);
        }
    }

    bool
    should_process_now () const
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (tree_view);
        bool is_visible = tree_view->get_is_drawable ();
        LOG_DD ("is visible: " << is_visible);
        return is_visible;
    }

    void
    finish_handling_debugger_stopped_event (IDebugger::StopReason /*a_reason*/,
                                            bool a_has_frame,
                                            const IDebugger::Frame &/*a_frame*/)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        THROW_IF_FAIL (tree_store);

        if (!a_has_frame)
            return;

        //TODO: walk the monitored variables and list those that have
        //changed.
        IDebugger::VariableList::const_iterator it;
        for (it = monitored_variables.begin ();
             it != monitored_variables.end ();
             ++it) {
            debugger.list_changed_variables (*it,
                                             sigc::bind
                                             (sigc::mem_fun
                                              (*this, &Priv::on_vars_changed),
                                              *it));
        }

        NEMIVER_CATCH;
    }

    // *********************
    // <signal handlers>
    // ********************

    void
    on_stopped_signal (IDebugger::StopReason a_reason,
                       bool a_has_frame,
                       const IDebugger::Frame &a_frame,
                       int /*a_thread_id*/,
                       int /*a_bp_num*/,
                       const UString &/*a_cookie*/)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY;

        if (IDebugger::is_exited (a_reason)
            || !a_has_frame)
            return;

        if (should_process_now ())
            finish_handling_debugger_stopped_event (a_reason,
                                                    a_has_frame,
                                                    a_frame);
        NEMIVER_CATCH;
    }

    void
    on_vars_changed (const IDebugger::VariableList &/*a_sub_vars*/,
                     const IDebugger::VariableSafePtr a_var_root)
    {
        NEMIVER_TRY;

        // Is this variable in scope or not? Update the graphical
        // stuff according to that property.
        Gtk::TreeModel::iterator var_it;
        update_var_in_scope_or_not (a_var_root, var_it);
        THROW_IF_FAIL (var_it);

        // TODO: Walk a_sub_vars (children of a_var_root) and update
        // them graphically, using update_a_variable.
        NEMIVER_CATCH;
    }

    // *********************
    // </signal handlers>
    // ********************

}; // end struct VarsMonitor

VarsMonitor::VarsMonitor (IDebugger &a_dbg,
                          IPerspective &a_perspective)
{
    m_priv.reset (new Priv (a_dbg, a_perspective));
}

VarsMonitor::~VarsMonitor ()
{
}

Gtk::Widget&
VarsMonitor::widget ()
{
    THROW_IF_FAIL (m_priv);
    return m_priv->get_widget ();
}

void
VarsMonitor::add_variable (const IDebugger::VariableSafePtr a_var)
{
    m_priv->add_variable (a_var);
}

void
VarsMonitor::add_variables (const IDebugger::VariableList &a_vars)
{
    m_priv->add_variables (a_vars);
}

void
VarsMonitor::remove_variable (const IDebugger::VariableSafePtr /*a_var*/)
{
}

void
VarsMonitor::remove_variables (const std::list<IDebugger::VariableSafePtr> &/*a_vars*/)
{
}

void
VarsMonitor::re_init_widget ()
{
}

NEMIVER_END_NAMESPACE (nemiver)
