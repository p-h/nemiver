// Dodji Seketeli
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
#include "nmv-call-function-dialog.h"
#include <glib/gi18n.h>
#include <libglademm.h>
#include "common/nmv-exception.h"
#include "common/nmv-env.h"
#include "common/nmv-ustring.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct CallFunctionDialog::Priv {
    Gtk::Entry *call_expr_entry;
    Gtk::Button *ok_button;
    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        call_expr_entry (0),
        ok_button (0)
    {
        a_dialog.set_default_response (Gtk::RESPONSE_OK);
        ok_button =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade, "okbutton");
        THROW_IF_FAIL (ok_button);
        ok_button->set_sensitive (false);

        call_expr_entry =
            ui_utils::get_widget_from_glade<Gtk::Entry> (a_glade,
                                                         "callexpressionentry");
        THROW_IF_FAIL (call_expr_entry);
        call_expr_entry->signal_changed ().connect
            (sigc::mem_fun (*this, &Priv::on_call_expr_entry_changed_signal));
        call_expr_entry->set_activates_default ();
    }

    void on_call_expr_entry_changed_signal ()
    {
        NEMIVER_TRY
        update_ok_button_sensitivity ();
        NEMIVER_CATCH
    }

    void update_ok_button_sensitivity ()
    {
        THROW_IF_FAIL (call_expr_entry);
        THROW_IF_FAIL (ok_button);

        if (call_expr_entry->get_text ().empty ()) {
            ok_button->set_sensitive (false);
        } else {
            ok_button->set_sensitive (true);
        }
    }
};//end struct CallFunctionDialog::Priv

CallFunctionDialog::CallFunctionDialog (const UString &a_root_path):
    Dialog (a_root_path, "callfunctiondialog.glade", "callfunctiondialog")
{
    m_priv.reset (new Priv (widget (), glade ()));
}

CallFunctionDialog::~CallFunctionDialog ()
{
}

UString
CallFunctionDialog::call_expression () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->call_expr_entry);

    return m_priv->call_expr_entry->get_text ();
}

void
CallFunctionDialog::call_expressoin (const UString &a_expr)
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->call_expr_entry);

    m_priv->call_expr_entry->set_text (a_expr);
}

NEMIVER_END_NAMESPACE (nemiver)

