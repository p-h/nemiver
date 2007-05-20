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
#include "common/nmv-exception.h"
#include "nmv-var-inspector-dialog.h"
#include "nmv-var-inspector.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VarInspectorDialog::Priv {
    friend class VarInspectorDialog ;
    Gtk::Entry *var_name_entry ;
    Gtk::Button *inspect_button ;
    SafePtr<VarInspector> var_inspector ;
    Gtk::Dialog &dialog ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;
    IDebugger &debugger ;

    Priv () ;
public:

    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade,
          IDebugger &a_debugger) :
        var_name_entry (0),
        inspect_button (0),
        dialog (a_dialog),
        glade (a_glade),
        debugger (a_debugger)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        build_dialog () ;
        connect_to_widget_signals () ;
    }

    void build_dialog ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        var_name_entry =
            ui_utils::get_widget_from_glade<Gtk::Entry> (glade,
                                                         "variablenameentry") ;
        inspect_button =
            ui_utils::get_widget_from_glade<Gtk::Button> (glade,
                                                          "inspectbutton") ;
        inspect_button->set_sensitive (false) ;

        Gtk::Box *box =
            ui_utils::get_widget_from_glade<Gtk::Box> (glade,
                                                       "inspectorwidgetbox") ;
        var_inspector.reset (new VarInspector (debugger)) ;
        THROW_IF_FAIL (var_inspector) ;
        Gtk::ScrolledWindow *scr = Gtk::manage (new Gtk::ScrolledWindow) ;
        scr->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;
        scr->set_shadow_type (Gtk::SHADOW_IN);
        scr->add (var_inspector->widget ()) ;
        box->pack_start (*scr) ;
        dialog.show_all () ;
    }

    void connect_to_widget_signals ()
    {
        THROW_IF_FAIL (inspect_button) ;
        inspect_button->signal_clicked ().connect (sigc::mem_fun
                (*this, &Priv::on_inspect_button_clicked_signal)) ;
        var_name_entry->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_var_name_changed_signal)) ;
        var_name_entry->signal_activate ().connect (sigc::mem_fun
                (*this, &Priv::on_var_name_activated_signal)) ;
    }

    //************************
    //<signal handlers>
    //*************************
    void on_var_name_activated_signal ()
    {
        NEMIVER_TRY

        THROW_IF_FAIL (var_name_entry) ;
        THROW_IF_FAIL (var_inspector) ;

        UString var_name = var_name_entry->get_text () ;
        if (var_name == "") {return;}
        var_inspector->inspect_variable (var_name) ;

        NEMIVER_CATCH
    }

    void on_inspect_button_clicked_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        THROW_IF_FAIL (var_name_entry) ;
        THROW_IF_FAIL (var_inspector) ;

        UString variable_name = var_name_entry->get_text () ;
        if (variable_name == "") {return;}
        var_inspector->inspect_variable (variable_name) ;

        NEMIVER_CATCH
    }

    void on_var_name_changed_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        NEMIVER_TRY

        THROW_IF_FAIL (var_name_entry) ;
        THROW_IF_FAIL (inspect_button) ;

        UString var_name = var_name_entry->get_text () ;
        if (var_name == "") {
            inspect_button->set_sensitive (false) ;
        } else {
            inspect_button->set_sensitive (true) ;
        }

        NEMIVER_CATCH
    }

    //************************
    //</signal handlers>
    //*************************
};//end class VarInspectorDialog::Priv

VarInspectorDialog::VarInspectorDialog (const UString &a_root_path,
                                        IDebugger &a_debugger) :
    Dialog (a_root_path,
            "varinspectordialog.glade",
            "varinspectordialog")
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    m_priv.reset
        (new VarInspectorDialog::Priv (widget (), glade (), a_debugger)) ;
    THROW_IF_FAIL (m_priv) ;
}

VarInspectorDialog::~VarInspectorDialog ()
{
    LOG_D ("delete", "destructor-domain") ;
}

UString
VarInspectorDialog::variable_name () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->var_name_entry) ;
    return m_priv->var_name_entry->get_text () ;
}

void
VarInspectorDialog::inspect_variable (const UString &a_var_name)
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->var_name_entry) ;
    THROW_IF_FAIL (m_priv->var_inspector) ;

    if (a_var_name != "") {
        m_priv->var_name_entry->set_text (a_var_name) ;
        m_priv->var_inspector->inspect_variable (a_var_name) ;
    }
}

const IDebugger::VariableSafePtr
VarInspectorDialog::variable () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->var_inspector->get_variable () ;
}

NEMIVER_END_NAMESPACE (nemiver)

