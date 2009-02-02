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
#include <gtkmm/liststore.h>
#include "common/nmv-exception.h"
#include "nmv-var-inspector-dialog.h"
#include "nmv-var-inspector.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VariableHistoryStoreColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring> varname;
    VariableHistoryStoreColumns() { add (varname); }
};

class VarInspectorDialog::Priv {
    friend class VarInspectorDialog;
    Gtk::ComboBoxEntry *var_name_entry;
    Glib::RefPtr<Gtk::ListStore> m_variable_history_store;
    VariableHistoryStoreColumns m_variable_store_columns;
    Gtk::Button *inspect_button;
    SafePtr<VarInspector2> var_inspector;
    Gtk::Dialog &dialog;
    Glib::RefPtr<Gnome::Glade::Xml> glade;
    IDebuggerSafePtr debugger;

    Priv ();
public:

    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade,
          IDebuggerSafePtr a_debugger) :
        var_name_entry (0),
        inspect_button (0),
        dialog (a_dialog),
        glade (a_glade),
        debugger (a_debugger)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        build_dialog ();
        connect_to_widget_signals ();
    }

    void build_dialog ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        var_name_entry =
            ui_utils::get_widget_from_glade<Gtk::ComboBoxEntry> (glade,
                                                         "variablenameentry");
        m_variable_history_store =
            Gtk::ListStore::create (m_variable_store_columns);
        var_name_entry->set_model (m_variable_history_store);
        var_name_entry->set_text_column (m_variable_store_columns.varname);

        inspect_button =
            ui_utils::get_widget_from_glade<Gtk::Button> (glade,
                                                          "inspectbutton");
        inspect_button->set_sensitive (false);

        Gtk::Box *box =
            ui_utils::get_widget_from_glade<Gtk::Box> (glade,
                                                       "inspectorwidgetbox");
        var_inspector.reset (new VarInspector2 (debugger));
        THROW_IF_FAIL (var_inspector);
        Gtk::ScrolledWindow *scr = Gtk::manage (new Gtk::ScrolledWindow);
        scr->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        scr->set_shadow_type (Gtk::SHADOW_IN);
        scr->add (var_inspector->widget ());
        box->pack_start (*scr);
        dialog.show_all ();
    }

    void connect_to_widget_signals ()
    {
        THROW_IF_FAIL (inspect_button);
        THROW_IF_FAIL (var_name_entry);
        inspect_button->signal_clicked ().connect (sigc::mem_fun
                (*this, &Priv::do_inspect_variable));
        var_name_entry->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_var_name_changed_signal));
        var_name_entry->get_entry()->signal_activate ().connect (sigc::mem_fun
                (*this, &Priv::do_inspect_variable));
    }

    //************************
    //<signal handlers>
    //*************************
    void do_inspect_variable ()
    {
        NEMIVER_TRY

        THROW_IF_FAIL (var_name_entry);

        UString var_name = var_name_entry->get_entry ()->get_text ();
        if (var_name == "") {return;}
        inspect_variable (var_name);

        NEMIVER_CATCH
    }

    void inspect_variable (const UString& a_varname)
    {
        THROW_IF_FAIL (var_inspector);
        THROW_IF_FAIL (m_variable_history_store);

        var_inspector->inspect_variable (a_varname);

        // now update the history in the entry combobox
        bool found = false;
        // first check if this variable is already in the history
        Gtk::TreeModel::iterator tree_iter;
        for (tree_iter = m_variable_history_store->children ().begin ();
             tree_iter != m_variable_history_store->children ().end ();
             ++tree_iter) {
            if (a_varname == (*tree_iter)[m_variable_store_columns.varname]) {
                found = true;
                break;
            }
        }
        if (!found) {
            // if it's not already in the list, add it.
            Gtk::TreeModel::iterator new_iter = m_variable_history_store->append ();
            (*new_iter)[m_variable_store_columns.varname] = a_varname;
        }
    }

    void on_var_name_changed_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY

        THROW_IF_FAIL (var_name_entry);
        THROW_IF_FAIL (inspect_button);

        UString var_name = var_name_entry->get_entry ()->get_text ();
        if (var_name == "") {
            inspect_button->set_sensitive (false);
        } else {
            inspect_button->set_sensitive (true);
        }

        // this handler is called when any text is changed in the entry or when
        // an item is selected from the combobox.  We don't want to inspect any
        // text that is typed into the entry, but we do want to inspect when
        // they choose an item from the dropdown list
        if (var_name_entry->get_active ()) {
            inspect_variable(var_name);
        }

        NEMIVER_CATCH
    }

    //************************
    //</signal handlers>
    //*************************
};//end class VarInspectorDialog::Priv

VarInspectorDialog::VarInspectorDialog (const UString &a_root_path,
                                        IDebuggerSafePtr &a_debugger) :
    Dialog (a_root_path,
            "varinspectordialog.glade",
            "varinspectordialog")
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    m_priv.reset
        (new VarInspectorDialog::Priv (widget (), glade (), a_debugger));
    THROW_IF_FAIL (m_priv);
}

VarInspectorDialog::~VarInspectorDialog ()
{
    LOG_D ("delete", "destructor-domain");
}

UString
VarInspectorDialog::variable_name () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->var_name_entry);
    return m_priv->var_name_entry->get_entry ()->get_text ();
}

void
VarInspectorDialog::inspect_variable (const UString &a_var_name)
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->var_name_entry);
    THROW_IF_FAIL (m_priv->var_inspector);

    if (a_var_name != "") {
        m_priv->var_name_entry->get_entry ()->set_text (a_var_name);
        m_priv->inspect_variable (a_var_name);
    }
}

const IDebugger::VariableSafePtr
VarInspectorDialog::variable () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->var_inspector->get_variable ();
}

NEMIVER_END_NAMESPACE (nemiver)

