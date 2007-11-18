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

#ifdef WITH_VARIABLE_WALKER

#include <glib/gi18n.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include "common/nmv-exception.h"
#include "nmv-var-inspector2.h"
#include "nmv-variables-utils2.h"
#include "nmv-i-var-walker.h"
#include "nmv-ui-utils.h"

namespace uutil = nemiver::ui_utils ;
namespace vutil = nemiver::variables_utils2 ;
namespace cmn = nemiver::common ;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VarInspector2::Priv : public sigc::trackable {
    friend class VarInspector2 ;
    Priv () ;

    bool requested_variable ;
    bool requested_type ;
    IDebuggerSafePtr debugger ;
    IDebugger::VariableSafePtr variable ;
    typedef SafePtr<Gtk::TreeView,
                    uutil::WidgetRef,
                    uutil::WidgetUnref> TreeViewSafePtr ;
    TreeViewSafePtr tree_view ;
    Glib::RefPtr<Gtk::TreeStore> tree_store ;
    Gtk::TreeModel::iterator var_row_it ;
    Gtk::TreeModel::iterator cur_selected_row;
    IVarWalkerSafePtr var_walker ;

    void build_widget ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        tree_store = Gtk::TreeStore::create (vutil::get_variable_columns ()) ;
        tree_view.reset (Gtk::manage (new Gtk::TreeView (tree_store)), true) ;
        tree_view->set_headers_clickable (true) ;
        Glib::RefPtr<Gtk::TreeSelection> sel = tree_view->get_selection () ;
        THROW_IF_FAIL (sel) ;
        sel->set_mode (Gtk::SELECTION_SINGLE) ;
        tree_view->append_column (_("Variable"),
                                  vutil::get_variable_columns ().name) ;
        Gtk::TreeViewColumn * col = tree_view->get_column (0) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        tree_view->append_column (_("Value"), vutil::get_variable_columns ().value) ;
        col = tree_view->get_column (1) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
        col->add_attribute (*col->get_first_cell_renderer (),
                            "foreground-gdk",
                            vutil::VariableColumns::FG_COLOR_OFFSET) ;

        tree_view->append_column (_("Type"),
                                  vutil::get_variable_columns ().type_caption);
        col = tree_view->get_column (2) ;
        THROW_IF_FAIL (col) ;
        col->set_resizable (true) ;
    }

    void connect_to_signals ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection () ;
        THROW_IF_FAIL (selection) ;
        selection->signal_changed ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_tree_view_selection_changed_signal));
        tree_view->signal_row_activated ().connect
            (sigc::mem_fun (*this, &Priv::on_tree_view_row_activated_signal)) ;

        /*
        debugger.variable_value_signal ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_debugger_variable_value_signal)) ;
        debugger.variable_type_signal ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_variable_type_signal)) ;
        debugger.pointed_variable_value_signal ().connect
            (sigc::mem_fun (*this,
                            &Priv::on_pointed_variable_value_signal)) ;

        tree_view->signal_row_expanded ().connect (sigc::mem_fun
            (*this, &Priv::on_tree_view_row_expanded_signal)) ;
        */
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

        THROW_IF_FAIL (tree_view && tree_store) ;
        re_init_tree_view () ;

        Gtk::TreeModel::iterator parent_iter = tree_store->children ().begin ();
        Gtk::TreeModel::iterator var_row ;
        vutil::append_a_variable (a_variable,
                                  *tree_view,
                                  tree_store,
                                  parent_iter,
                                  var_row) ;
        LOG_DD ("set variable" << a_variable->name ()) ;
        if (var_row) {
            tree_view->expand_row (tree_store->get_path (var_row), false) ;
        }
    }

    void show_variable_type_in_dialog ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        if (!cur_selected_row) {return;}
        UString type =
            (Glib::ustring) (*cur_selected_row)[vutil::get_variable_columns ().type] ;
        UString message ;
        message.printf (_("Variable type is: \n %s"), type.c_str ()) ;

        IDebugger::VariableSafePtr variable =
            (IDebugger::VariableSafePtr)
                cur_selected_row->get_value (vutil::get_variable_columns ().variable);
        THROW_IF_FAIL (variable) ;
        //message += "\nDumped for debug: \n" ;
        //variable->to_string (message, false) ;
        ui_utils::display_info (message) ;
    }

    IVarWalkerSafePtr create_var_walker ()
    {
        cmn::DynamicModule::Loader *loader =
            debugger->get_dynamic_module ().get_module_loader ();
        THROW_IF_FAIL (loader) ;
        cmn::DynamicModuleManager *module_manager = loader->get_dynamic_module_manager ();
        THROW_IF_FAIL (module_manager) ;
        IVarWalkerSafePtr result =
            module_manager->load_iface<IVarWalker> ("varwalker", "IVarWalker");
        THROW_IF_FAIL (result) ;
        return result ;
    }

    IVarWalkerSafePtr get_var_walker ()
    {
        if (!var_walker) {
            var_walker = create_var_walker () ;
            THROW_IF_FAIL (var_walker) ;
            var_walker->visited_variable_signal ().connect
                (sigc::mem_fun (this, &VarInspector2::Priv::on_visited_variable_signal)) ;
        }
        return var_walker ;
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
                                    (vutil::get_variable_columns ().variable) ;
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
            (Glib::ustring) it->get_value
                            (vutil::get_variable_columns ().type) ;
        if (type == "") {return;}

        if (a_col != tree_view->get_column (2)) {return;}
        cur_selected_row = it ;
        show_variable_type_in_dialog () ;
        NEMIVER_CATCH
    }

    void on_visited_variable_signal (const IDebugger::VariableSafePtr &a_var)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        set_variable (a_var) ;

        NEMIVER_CATCH
    }

    // ******************
    // </signal handlers>
    // ******************

public:

    Priv (IDebuggerSafePtr &a_debugger) :
          requested_variable (false),
          requested_type (false),
          debugger (a_debugger)
    {
        build_widget () ;
        re_init_tree_view () ;
        connect_to_signals () ;
    }
};//end class VarInspector2::Priv

VarInspector2::VarInspector2 (IDebuggerSafePtr &a_debugger)
{
    m_priv.reset (new Priv (a_debugger)) ;
}

VarInspector2::~VarInspector2 ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

Gtk::Widget&
VarInspector2::widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->tree_view) ;
    return *m_priv->tree_view ;
}

void
VarInspector2::set_variable (IDebugger::VariableSafePtr &a_variable)
{
    THROW_IF_FAIL (m_priv) ;

    m_priv->set_variable (a_variable) ;
}

void
VarInspector2::inspect_variable (const UString &a_variable_name)
{
    if (a_variable_name == "") {return;}
    THROW_IF_FAIL (m_priv) ;
    m_priv->requested_variable = true;
    IVarWalkerSafePtr var_walker = m_priv->get_var_walker () ;
    THROW_IF_FAIL (var_walker) ;
    var_walker->connect (m_priv->debugger, a_variable_name) ;
    var_walker->do_walk_variable () ;
}

IDebugger::VariableSafePtr
VarInspector2::get_variable () const
{
    THROW_IF_FAIL (m_priv) ;

    return m_priv->variable ;
}

void
VarInspector2::clear ()
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->re_init_tree_view () ;
}

NEMIVER_END_NAMESPACE (nemiver)

#endif //WITH_VARIABLE_WALKER

