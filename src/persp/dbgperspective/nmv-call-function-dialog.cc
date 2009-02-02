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

struct CallExprHistoryCols : public Gtk::TreeModel::ColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> expr;
    CallExprHistoryCols () { add (expr); }
};

static CallExprHistoryCols&
get_call_expr_history_cols ()
{
    static CallExprHistoryCols cols;
    return cols;
}

struct CallFunctionDialog::Priv {
    Gtk::ComboBoxEntry *call_expr_entry;
    Glib::RefPtr<Gtk::ListStore> m_call_expr_history;
    Gtk::Button *ok_button;
    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        call_expr_entry (0),
        ok_button (0)
    {
        a_dialog.set_default_response (Gtk::RESPONSE_OK);

        ok_button =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade,
                                                          "okbutton");
        THROW_IF_FAIL (ok_button);
        ok_button->set_sensitive (false);

        call_expr_entry =
            ui_utils::get_widget_from_glade<Gtk::ComboBoxEntry>
                                            (a_glade, "callexpressionentry");
        THROW_IF_FAIL (call_expr_entry);
        m_call_expr_history=
            Gtk::ListStore::create (get_call_expr_history_cols ());
        call_expr_entry->set_model (m_call_expr_history);
        call_expr_entry->set_text_column (get_call_expr_history_cols ().expr);

        call_expr_entry->signal_changed ().connect
            (sigc::mem_fun (*this, &Priv::on_call_expr_entry_changed_signal));
        call_expr_entry->get_entry ()->set_activates_default ();
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

        if (call_expr_entry->get_entry ()->get_text ().empty ()) {
            ok_button->set_sensitive (false);
        } else {
            ok_button->set_sensitive (true);
        }
    }

    bool exists_in_history (const UString &a_expr) const
    {
        THROW_IF_FAIL (m_call_expr_history);
        Gtk::TreeModel::iterator it;
        for (it = m_call_expr_history->children ().begin ();
             it != m_call_expr_history->children ().end ();
             ++it) {
            if ((*it)[get_call_expr_history_cols ().expr] == a_expr) {
                return true;
            }
        }
        return false;
    }

    void clear_history ()
    {
        m_call_expr_history->clear ();
    }

    void add_to_history (const UString &a_expr,
                         bool a_prepend = true,
                         bool allow_dups = true)
    {
        if (a_expr.empty () // don't append empty expressiosn
            // don't append an expression if it exists already.
            || (!allow_dups && exists_in_history (a_expr)))
            return;

        THROW_IF_FAIL (m_call_expr_history);
        Gtk::TreeModel::iterator it;
        if (a_prepend)
            it = m_call_expr_history->insert
                                    (m_call_expr_history->children ().begin ());
        else
            it = m_call_expr_history->append ();
        (*it)[get_call_expr_history_cols ().expr] = a_expr;
    }

    void get_history (std::list<UString> &a_hist) const
    {
        Gtk::TreeModel::iterator it;
        for (it = m_call_expr_history->children ().begin ();
             it != m_call_expr_history->children ().end ();
             ++it) {
            Glib::ustring elem = (*it)[get_call_expr_history_cols ().expr];
            a_hist.push_back (elem);
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

    return m_priv->call_expr_entry->get_entry ()->get_text ();
}

void
CallFunctionDialog::call_expression (const UString &a_expr)
{
    if (a_expr.empty ())
        return;

    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->call_expr_entry);

    m_priv->call_expr_entry->get_entry ()->set_text (a_expr);
    add_to_history (a_expr);
}

void
CallFunctionDialog::set_history (const std::list<UString> &a_hist)
{
    THROW_IF_FAIL (m_priv);

    m_priv->clear_history ();

    list<UString>::const_iterator it;
    for (it = a_hist.begin (); it != a_hist.end (); ++it) {
        m_priv->add_to_history (*it, false /*append*/);
    }
}

void
CallFunctionDialog::get_history (std::list<UString> &a_hist) const
{
    THROW_IF_FAIL (m_priv);
    m_priv->get_history (a_hist);
}

void
CallFunctionDialog::add_to_history (const UString &a_expr)
{
    THROW_IF_FAIL (m_priv);
    m_priv->add_to_history (a_expr,
                            true /* prepends */,
                            false /* disallow duplicates */);
}
NEMIVER_END_NAMESPACE (nemiver)

