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
#include <gtkmm/treestore.h>
#include "nmv-var-inspector.h"
#include "nmv-variables-utils.h"
#include "nmv-ui-utils.h"
#include "nmv-exception.h"

using namespace nemiver::ui_utils ;
using namespace nemiver::variables_utils ;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VarInspector::Priv {
    friend class VarInspector ;
    Priv () ;

    bool requested_variable ;
    bool requested_type ;
    IDebugger &debugger ;
    IDebugger::VariableSafePtr variable ;
    SafePtr<Gtk::TreeView> tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_store ;
    Gtk::TreeModel::iterator var_row_it ;
    Gtk::TreeModel::iterator cur_selected_row;

    void build_widget ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        tree_store = Gtk::TreeStore::create (get_variable_columns ()) ;
        tree_view.reset (new Gtk::TreeView (tree_store)) ;
        tree_view->set_headers_clickable (true) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        sel->set_mode (Gtk::SELECTION_SINGLE) ;
        tree_view->append_column (_("variable"),
                                 variables_utils::get_variable_columns ().name) ;
        Gtk::TreeViewColumn * col = tree_view->get_column (0) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        tree_view->append_column (_("value"), get_variable_columns ().value) ;
        col = tree_view->get_column (1) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        col->add_attribute (*col->get_first_cell_renderer (),
                            "foreground-gdk",
                            variables_utils::VariableColumns::FG_COLOR_OFFSET) ;

        tree_view->append_column (_("type"),
                                  get_variable_columns ().type_caption);
        col = tree_view->get_column (2) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
    }

    void connect_to_debugger_signals ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        debugger.variable_value_signal ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_debugger_variable_value_signal)) ;
        debugger.variable_type_signal ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_variable_type_signal)) ;
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection () ;
        THROW_IF_FAIL (selection) ;
        selection->signal_changed ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_tree_view_selection_changed_signal));
        tree_view->signal_row_activated ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_row_activated_signal)) ;
    }

    void re_init_tree_view ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (tree_store) ;
        tree_store->clear () ;
    }

    void set_variable (const IDebugger::VariableSafePtr &a_variable)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        variable = a_variable ;
        re_init_tree_view () ;

        Gtk::TreeModel::iterator parent_iter ;
        append_a_variable (a_variable, parent_iter,
                           tree_store, *tree_view,
                           debugger, false, false,
                           var_row_it) ;

        requested_type = true ;
        debugger.print_variable_type (a_variable->name ()) ;
    }

    void set_variable_type (const UString &a_var_name,
                            const UString &a_var_type)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (var_row_it) ;

        Gtk::TreeModel::iterator it ;
        if ((Glib::ustring) var_row_it->get_value (get_variable_columns ().name)
             == a_var_name) {
            it = var_row_it ;
        } else if (!get_variable_iter_from_qname (a_var_name, var_row_it, it)) {
            LOG_ERROR ("could not get iter for variable: '"
                       << a_var_name << "'") ;
            return ;
        }
        THROW_IF_FAIL (it) ;
        set_a_variable_type_real (it, a_var_type) ;
        NEMIVER_CATCH
    }

    void show_variable_type_in_dialog ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        if (!cur_selected_row) {return;}
        UString type =
            (Glib::ustring) (*cur_selected_row)[get_variable_columns ().type] ;
        UString message ;
        message.printf (_("Variable type is: \n %s"), type.c_str ()) ;

        IDebugger::VariableSafePtr variable =
            (IDebugger::VariableSafePtr)
                cur_selected_row->get_value (get_variable_columns ().variable);
        THROW_IF_FAIL (variable) ;
        //message += "\nDumped for debug: \n" ;
        //variable->to_string (message, false) ;
        ui_utils::display_info (message) ;
    }

    // ******************
    // <signal handlers>
    // ******************


    void on_tree_view_selection_changed_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        THROW_IF_FAIL (tree_view) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        cur_selected_row = sel->get_selected () ;
        if (!cur_selected_row) {return;}
        IDebugger::VariableSafePtr variable =
        (IDebugger::VariableSafePtr)cur_selected_row->get_value
                                            (get_variable_columns ().variable) ;
        if (!variable) {return;}
        UString qname ;
        variable->build_qname (qname) ;
        LOG_DD ("row of variable '" << qname << "'") ;

        NEMIVER_CATCH
    }

    void on_tree_view_row_activated_signal (const Gtk::TreeModel::Path &a_path,
                                            Gtk::TreeViewColumn *a_col)
    {
        NEMIVER_TRY
        THROW_IF_FAIL (tree_store) ;
        Gtk::TreeModel::iterator it = tree_store->get_iter (a_path) ;
        UString type =
            (Glib::ustring) it->get_value (get_variable_columns ().type) ;
        if (type == "") {return;}

        if (a_col != tree_view->get_column (2)) {return;}
        cur_selected_row = it ;
        show_variable_type_in_dialog () ;
        NEMIVER_CATCH
    }

    void on_debugger_variable_value_signal
                                (const UString &a_variable_name,
                                 const IDebugger::VariableSafePtr &a_variable)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        LOG_DD ("variable_name: '" << a_variable_name << "'") ;
        if (!requested_variable) {return;}
        set_variable (a_variable) ;
        requested_variable = false ;
    }

    void on_variable_type_signal (const UString &a_var_name,
                                  const UString &a_type)
    {
        LOG_DD ("variable_name: '" << a_var_name << "'") ;
        LOG_DD ("variable_type: '" << a_type << "'") ;
        if (!requested_type) {return;}
        set_variable_type (a_var_name, a_type) ;
        requested_type = false ;
    }


    // ******************
    // </signal handlers>
    // ******************

public:

    Priv (IDebugger &a_debugger) :
        requested_variable (false),
        requested_type (false),
        debugger (a_debugger)
    {
        build_widget () ;
        re_init_tree_view () ;
        connect_to_debugger_signals () ;
    }
};//end class VarInspector::Priv

VarInspector::VarInspector (IDebugger &a_debugger)
{
    m_priv.reset (new Priv (a_debugger)) ;
}

VarInspector::~VarInspector ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

Gtk::Widget&
VarInspector::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    return *m_priv->tree_view ;
}

void
VarInspector::set_variable (IDebugger::VariableSafePtr &a_variable)
{
    THROW_IF_FAIL (m_priv) ;

    m_priv->set_variable (a_variable) ;
}

void
VarInspector::inspect_variable (const UString &a_variable_name)
{
    if (a_variable_name == "") {return;}
    THROW_IF_FAIL (m_priv) ;
    m_priv->requested_variable = true;
    m_priv->debugger.print_variable_value (a_variable_name) ; ;
}

IDebugger::VariableSafePtr
VarInspector::get_variable () const
{
    THROW_IF_FAIL (m_priv) ;

    return m_priv->variable ;
}

void
VarInspector::clear ()
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->re_init_tree_view () ;
}

NEMIVER_END_NAMESPACE (nemiver)
