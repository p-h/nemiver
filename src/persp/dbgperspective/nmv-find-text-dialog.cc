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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include <gtksourceviewmm/sourceiter.h>
#include "nmv-find-text-dialog.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"
#include "nmv-source-editor.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

using namespace gtksourceview ;

class FindTextDialog::Priv {
    friend class FindTextDialog ;
    Gtk::Dialog &dialog ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;
    bool search_succeeded ;
    Gtk::TextIter match_start ;
    Gtk::TextIter match_end ;

    Priv () ;

public:

    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        dialog (a_dialog),
        glade (a_glade),
        search_succeeded (false)
    {
        a_dialog.set_default_response (Gtk::RESPONSE_OK) ;
    }

    Gtk::Button* get_search_button ()
    {
        Gtk::Button *button =
            ui_utils::get_widget_from_glade<Gtk::Button> (glade,
                                                          "searchbutton") ;
        return button ;
    }

    Gtk::ComboBoxEntry* get_search_text_combo () const
    {
        Gtk::ComboBoxEntry *combo =
            ui_utils::get_widget_from_glade<Gtk::ComboBoxEntry>
                                                    (glade, "searchtextcombo") ;
        return combo ;
    }

    Gtk::CheckButton* get_match_case_check_button () const
    {
        Gtk::CheckButton *button =
            ui_utils::get_widget_from_glade<Gtk::CheckButton>
                                                (glade, "matchcasecheckbutton") ;
        return button ;
    }

    Gtk::CheckButton* get_match_entire_word_check_button () const
    {
        Gtk::CheckButton *button =
            ui_utils::get_widget_from_glade<Gtk::CheckButton>
                                        (glade, "matchentirewordcheckbutton") ;
        return button ;
    }

    Gtk::CheckButton* get_wrap_around_check_button () const
    {
        Gtk::CheckButton *button =
            ui_utils::get_widget_from_glade<Gtk::CheckButton>
                                        (glade, "wraparoundcheckbutton") ;
        return button ;
    }

    Gtk::CheckButton* get_search_backwards_check_button () const
    {
        Gtk::CheckButton *button =
            ui_utils::get_widget_from_glade<Gtk::CheckButton>
                                        (glade, "searchbackwardscheckbutton") ;
        return button ;
    }

    void connect_dialog_signals ()
    {
        Gtk::Button *button = get_search_button () ;
        THROW_IF_FAIL (button) ;
        button->signal_clicked ().connect (sigc::mem_fun
                                    (*this, &Priv::on_search_button_clicked)) ;
    }

    //*******************
    //<signal handlers>
    //*******************
    void on_search_button_clicked ()
    {
        NEMIVER_TRY

        NEMIVER_CATCH
    }
    //*******************
    //</signal handlers>
    //*******************

};//end FindTextDialog

FindTextDialog::FindTextDialog (const UString &a_root_path) :
    Dialog (a_root_path, "findtextdialog.glade", "findtextdialog")
{
    m_priv.reset (new Priv (widget (), glade ())) ;
    THROW_IF_FAIL (m_priv) ;
}

FindTextDialog::~FindTextDialog ()
{
    LOG_D ("destroyed", "destructor-domain") ;
}

bool
FindTextDialog::did_search_succeed () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->search_succeeded ;
}

Gtk::TextIter&
FindTextDialog::get_search_match_start () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->match_start ;
}

Gtk::TextIter&
FindTextDialog::get_search_match_end () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->match_end ;
}

void
FindTextDialog::get_search_string (UString &a_search_str) const
{
    THROW_IF_FAIL (m_priv) ;
    a_search_str = m_priv->get_search_text_combo ()->get_entry ()->get_text () ;
}

void
FindTextDialog::set_search_string (const UString &a_search_str)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->get_search_text_combo ()->get_entry ()->set_text (a_search_str) ;
}

bool
FindTextDialog::get_match_case () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->get_match_case_check_button ()->get_active () ;
}

void
FindTextDialog::set_match_case (bool a_flag)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->get_match_case_check_button ()->set_active (a_flag) ;
}

bool
FindTextDialog::get_match_entire_word () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->get_match_entire_word_check_button ()->get_active () ;
}

void
FindTextDialog::set_match_entire_word (bool a_flag)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->get_match_entire_word_check_button ()->set_active (a_flag) ;
}

bool
FindTextDialog::get_wrap_around () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->get_wrap_around_check_button ()->get_active () ;
}

void
FindTextDialog::set_wrap_around (bool a_flag)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->get_wrap_around_check_button ()->set_active (a_flag) ;
}

bool
FindTextDialog::get_search_backward () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->get_search_backwards_check_button ()->get_active () ;
}

void
FindTextDialog::set_search_backward (bool a_flag)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->get_search_backwards_check_button ()->set_active (a_flag) ;
}


NEMIVER_END_NAMESPACE (nemiver)

