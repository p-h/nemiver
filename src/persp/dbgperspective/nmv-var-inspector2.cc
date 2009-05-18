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
#include "config.h"

#include <glib/gi18n.h>
#include <gtkmm/treestore.h>
#include "common/nmv-exception.h"
#include "nmv-var-inspector2.h"
#include "nmv-variables-utils.h"
#include "nmv-i-var-walker.h"
#include "nmv-ui-utils.h"
#include "nmv-vars-treeview.h"

#ifdef WITH_VAROBJS

namespace uutil = nemiver::ui_utils;
namespace vutil = nemiver::variables_utils2;
namespace cmn = nemiver::common;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VarInspector2::Priv : public sigc::trackable {
    friend class VarInspector2;
    Priv ();

    bool requested_variable;
    bool requested_type;
    bool expand_variable;
    IDebuggerSafePtr debugger;
    // Variable that is being inspected
    // at a given point in time
    IDebugger::VariableSafePtr variable;
    VarsTreeViewSafePtr tree_view;
    Glib::RefPtr<Gtk::TreeStore> tree_store;
    Gtk::TreeModel::iterator var_row_it;
    Gtk::TreeModel::iterator cur_selected_row;

    void
    build_widget ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        tree_view = VarsTreeView::create ();
        THROW_IF_FAIL (tree_view);
        tree_store = tree_view->get_tree_store ();
        THROW_IF_FAIL (tree_store);
    }

    void
    connect_to_signals ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        Glib::RefPtr<Gtk::TreeSelection> selection =
                                        tree_view->get_selection ();
        THROW_IF_FAIL (selection);
        selection->signal_changed ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_tree_view_selection_changed_signal));
        tree_view->signal_row_activated ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_row_activated_signal));

        tree_view->signal_row_expanded ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_row_expanded_signal));

        Gtk::CellRenderer *r = tree_view->get_column_cell_renderer
            (VarsTreeView::VARIABLE_VALUE_COLUMN_INDEX);
        THROW_IF_FAIL (r);

        Gtk::CellRendererText *t =
            dynamic_cast<Gtk::CellRendererText*> (r);
        t->signal_edited ().connect (sigc::mem_fun
                                     (*this, &Priv::on_cell_edited_signal));
    }

    void
    re_init_tree_view ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (tree_store);
        tree_store->clear ();
    }

    // If the variable we are inspected was created
    // with a backend counterpart (variable objects for GDB),
    // instruct the backend to delete its variable counterpart.
    void
    delete_variable_if_needed ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        if (variable
            && !variable->internal_name ().empty ()
            && debugger) {
            debugger->delete_variable (variable);
        }
    }

    void
    set_variable (const IDebugger::VariableSafePtr a_variable,
                  bool a_expand)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        THROW_IF_FAIL (tree_view && tree_store);
        re_init_tree_view ();
        delete_variable_if_needed ();

        Gtk::TreeModel::iterator parent_iter =
                                    tree_store->children ().begin ();
        Gtk::TreeModel::iterator var_row;
        vutil::append_a_variable (a_variable,
                                  *tree_view,
                                  tree_store,
                                  parent_iter,
                                  var_row);
        LOG_DD ("set variable" << a_variable->name ());

        // If the variable has children, unfold it so that we can see them.
        if (a_expand
            && var_row
            && (a_variable->members ().size ()
                || a_variable->needs_unfolding ()))
            tree_view->expand_row (tree_store->get_path (var_row), false);
        variable = a_variable;
    }

    void
    show_variable_type_in_dialog ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        if (!cur_selected_row) {return;}
        UString type =
        (Glib::ustring)
                (*cur_selected_row)[vutil::get_variable_columns ().type];
        UString message;
        message.printf (_("Variable type is: \n %s"), type.c_str ());

        IDebugger::VariableSafePtr variable =
            (IDebugger::VariableSafePtr)
                cur_selected_row->get_value
                                (vutil::get_variable_columns ().variable);
        THROW_IF_FAIL (variable);
        // message += "\nDumped for debug: \n";
        // variable->to_string (message, false);
        ui_utils::display_info (message);
    }

    void
    create_variable (const UString &a_name,
                     bool a_expand)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        expand_variable = a_expand;
        debugger->create_variable
            (a_name, sigc::mem_fun
                    (this, &VarInspector2::Priv::on_variable_created_signal));
    }

    // ******************
    // <signal handlers>
    // ******************


    void
    on_tree_view_selection_changed_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY

        THROW_IF_FAIL (tree_view);
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection ();
        THROW_IF_FAIL (sel);
        cur_selected_row = sel->get_selected ();
        if (!cur_selected_row) {return;}
        IDebugger::VariableSafePtr var =
            (IDebugger::VariableSafePtr)cur_selected_row->get_value
                                    (vutil::get_variable_columns ().variable);
        if (!var)
            return;

        variable = var;

        // If the variable should be editable, set the cell of the variable value
        // editable.
        cur_selected_row->set_value
                    (vutil::get_variable_columns ().variable_value_editable,
                     debugger->is_variable_editable (variable));

        // Dump some log about the variable that got selected.
        UString qname;
        variable->build_qname (qname);
        LOG_DD ("row of variable '" << qname << "'");

        NEMIVER_CATCH
    }

    void
    on_tree_view_row_activated_signal (const Gtk::TreeModel::Path &a_path,
                                            Gtk::TreeViewColumn *a_col)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY

        THROW_IF_FAIL (tree_store);
        Gtk::TreeModel::iterator it = tree_store->get_iter (a_path);
        UString type =
            (Glib::ustring) it->get_value
                            (vutil::get_variable_columns ().type);
        if (type == "") {return;}

        if (a_col != tree_view->get_column (2)) {return;}
        cur_selected_row = it;
        show_variable_type_in_dialog ();

        NEMIVER_CATCH
    }

    void
    on_tree_view_row_expanded_signal (const Gtk::TreeModel::iterator &a_row_it,
                                      const Gtk::TreeModel::Path &a_row_path)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY

        if (!(*a_row_it)[vutil::get_variable_columns ().needs_unfolding]) {
            return;
        }
        LOG_DD ("The variable needs unfolding");

        IDebugger::VariableSafePtr var =
            (*a_row_it)[vutil::get_variable_columns ().variable];
        debugger->unfold_variable
        (var, sigc::bind (sigc::mem_fun (*this,
                                         &Priv::on_variable_unfolded_signal),
                          a_row_path));
        LOG_DD ("variable unfolding triggered");

        NEMIVER_CATCH
    }

    void
    on_cell_edited_signal (const Glib::ustring &a_path,
                           const Glib::ustring &a_text)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY

        Gtk::TreeModel::iterator row = tree_store->get_iter (a_path);
        IDebugger::VariableSafePtr var =
            (*row)[vutil::get_variable_columns ().variable];
        THROW_IF_FAIL (var);

        debugger->assign_variable
            (var, a_text,
             sigc::bind (sigc::mem_fun
                                 (*this, &Priv::on_variable_assigned_signal),
                         a_path));

        NEMIVER_CATCH
    }

    void
    on_variable_created_signal (const IDebugger::VariableSafePtr a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY

        set_variable (a_var, expand_variable);

        NEMIVER_CATCH
    }

    void
    on_variable_unfolded_signal (const IDebugger::VariableSafePtr a_var,
                                 const Gtk::TreeModel::Path &a_var_node)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY

        Gtk::TreeModel::iterator var_it = tree_store->get_iter (a_var_node);
        vutil::update_unfolded_variable (a_var, *tree_view, tree_store, var_it);
        tree_view->expand_row (a_var_node, false);

        NEMIVER_CATCH
    }

    void
    on_variable_assigned_signal (const IDebugger::VariableSafePtr a_var,
                                 const UString &a_var_path)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        NEMIVER_TRY

        Gtk::TreeModel::iterator var_row
                                = tree_store->get_iter (a_var_path);
        THROW_IF_FAIL (var_row);
        THROW_IF_FAIL (tree_view);
        vutil::update_a_variable_node (a_var, *tree_view,
                                       var_row, false, false);

        NEMIVER_CATCH
    }

    // ******************
    // </signal handlers>
    // ******************

public:

    Priv (IDebuggerSafePtr a_debugger) :
          requested_variable (false),
          requested_type (false),
          expand_variable (false),
          debugger (a_debugger)
    {
        build_widget ();
        re_init_tree_view ();
        connect_to_signals ();
    }

    ~Priv ()
    {
        delete_variable_if_needed ();
    }
};//end class VarInspector2::Priv

VarInspector2::VarInspector2 (IDebuggerSafePtr a_debugger)
{
    m_priv.reset (new Priv (a_debugger));
}

VarInspector2::~VarInspector2 ()
{
    LOG_D ("deleted", "destructor-domain");
}

Gtk::Widget&
VarInspector2::widget () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->tree_view);
    return *m_priv->tree_view;
}

void
VarInspector2::set_variable (IDebugger::VariableSafePtr a_variable,
                             bool a_expand)
{
    THROW_IF_FAIL (m_priv);

    m_priv->set_variable (a_variable, a_expand);
}

void
VarInspector2::inspect_variable (const UString &a_variable_name,
                                 bool a_expand)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    if (a_variable_name == "") {return;}
    THROW_IF_FAIL (m_priv);
    m_priv->re_init_tree_view ();
    m_priv->delete_variable_if_needed ();
    m_priv->create_variable (a_variable_name, a_expand);
}

IDebugger::VariableSafePtr
VarInspector2::get_variable () const
{
    THROW_IF_FAIL (m_priv);

    return m_priv->variable;
}

void
VarInspector2::clear ()
{
    THROW_IF_FAIL (m_priv);
    m_priv->re_init_tree_view ();
    m_priv->delete_variable_if_needed ();
}

NEMIVER_END_NAMESPACE (nemiver)

#endif //WITH_VAROBJS

