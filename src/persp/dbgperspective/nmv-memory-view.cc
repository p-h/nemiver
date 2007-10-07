/*******************************************************************************
 *  PROJECT: Nemiver
 *
 *  AUTHOR: Jonathon Jongsma
 *  See COPYRIGHT file copyright information.
 *
 *  License:
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the
 *    Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *    Boston, MA  02111-1307  USA
 *
 *******************************************************************************/
#include <gtkmm/textview.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include "nmv-ui-utils.h"
#include "nmv-memory-view.h"
#include "nmv-i-debugger.h"

namespace nemiver {

struct MemoryView::Priv {
public:
    Glib::RefPtr<Gnome::Glade::Xml> m_glade;
    Gtk::TextView* m_textview;
    Gtk::Entry* m_address_entry;
    Gtk::Button* m_jump_button;
    SafePtr<Gtk::VBox> m_container;
    IDebuggerSafePtr m_debugger;

    Priv (const UString& a_resource_root_path, IDebuggerSafePtr& a_debugger) :
        m_debugger (a_debugger)
    {
        std::vector<string> path_elems ;
        path_elems.push_back (Glib::locale_from_utf8 (a_resource_root_path)) ;
        path_elems.push_back ("glade");
        path_elems.push_back ("memoryviewwidget.glade");
        std::string glade_path = Glib::build_filename (path_elems) ;
        if (!Glib::file_test (glade_path, Glib::FILE_TEST_IS_REGULAR)) {
            THROW (UString ("could not find file ") + glade_path) ;
        }
        m_glade = Gnome::Glade::Xml::create (glade_path) ;
        THROW_IF_FAIL (m_glade) ;
        Gtk::Window* win =  ui_utils::get_widget_from_glade<Gtk::Window> (m_glade,
                "unused_window") ;
        THROW_IF_FAIL (win) ;
        m_container.reset (ui_utils::get_widget_from_glade<Gtk::VBox> (m_glade,
                "memoryviewwidget")) ;
        THROW_IF_FAIL (m_container) ;
        static_cast<Gtk::Container*>(win)->remove (static_cast<Gtk::Widget&>(*m_container));

        m_textview = ui_utils::get_widget_from_glade<Gtk::TextView> (m_glade,
                "memory_textview") ;
        THROW_IF_FAIL (m_textview) ;
        m_address_entry = ui_utils::get_widget_from_glade<Gtk::Entry> (m_glade,
                "address_entry") ;
        THROW_IF_FAIL (m_address_entry) ;
        m_jump_button = ui_utils::get_widget_from_glade<Gtk::Button> (m_glade,
                "jump_button") ;
        THROW_IF_FAIL (m_jump_button) ;

        connect_signals ();
    }

    void connect_signals ()
    {
        THROW_IF_FAIL (m_jump_button && m_debugger);
        m_debugger->state_changed_signal ().connect (sigc::mem_fun (this,
                    &Priv::on_debugger_state_changed));
        m_debugger->stopped_signal ().connect (sigc::mem_fun (this,
                    &Priv::on_debugger_stopped));
        m_jump_button->signal_clicked ().connect (sigc::mem_fun (this,
                    &Priv::on_jump_button_clicked));
        m_debugger->read_memory_signal ().connect (sigc::mem_fun (this,
                    &Priv::on_memory_read_response));
    }

    void on_debugger_state_changed (IDebugger::State a_state)
    {
        NEMIVER_TRY
        THROW_IF_FAIL (m_address_entry);
        switch (a_state)
        {
            case IDebugger::READY:
                set_widget_sensitive (true);
                break;
            default:
                set_widget_sensitive (false);
        }
        NEMIVER_CATCH
    }

    void on_debugger_stopped (const UString& /*a_reason*/,
                              bool /*a_has_frame*/,
                              const IDebugger::Frame& /*a_frame*/,
                              int /*a_thread_id*/,
                              const UString& /*a_cookie*/)
    {
        NEMIVER_TRY
        UString addr = m_address_entry->get_text ();
        if (validate_address (addr))
        {
            m_debugger->read_memory (addr, 100);
        }
        NEMIVER_CATCH
    }

    void on_jump_button_clicked ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY
        THROW_IF_FAIL (m_address_entry && m_debugger);
        UString addr = m_address_entry->get_text ();
        if (validate_address (addr))
        {
            m_debugger->read_memory (addr, 100);
        }
        NEMIVER_CATCH
    }

    bool validate_address (const UString& addr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        // FIXME: implement validation
        if (!addr.empty ())
        {
            return true;
        }
        return false;
    }

    void set_widget_sensitive (bool enable = true)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (m_address_entry && m_jump_button);
        m_address_entry->set_sensitive (enable);
        m_jump_button->set_sensitive (enable);
    }

    void on_memory_read_response (const UString& a_addr,
            std::vector<UString> a_values, const UString& a_cookie)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY
        THROW_IF_FAIL (m_address_entry && m_textview);
        if (a_cookie.empty ()) {}
        m_address_entry->set_text (a_addr);
        UString val_str = UString::join (a_values);
        m_textview->get_buffer ()->set_text (val_str);
        NEMIVER_CATCH
    }
};

MemoryView::MemoryView (const UString& a_resource_root_path, IDebuggerSafePtr&
        a_debugger) :
    m_priv (new Priv(a_resource_root_path, a_debugger))
{
}

MemoryView::~MemoryView ()
{}

Gtk::Widget&
MemoryView::widget () const
{
    THROW_IF_FAIL (m_priv && m_priv->m_container);
    return *m_priv->m_container;
}

void
MemoryView::clear ()
{
    THROW_IF_FAIL (m_priv && m_priv->m_textview);
    m_priv->m_textview->get_buffer ()->set_text ("");
}

} // namespace nemiver
