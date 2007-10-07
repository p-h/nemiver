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
#include <sstream>
#include <gtkmm/textview.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <glib/gi18n.h>
#include "nmv-ui-utils.h"
#include "nmv-memory-view.h"
#include "nmv-memory-editor.h"
#include "nmv-i-debugger.h"

namespace nemiver {

struct MemoryView::Priv {
public:
    SafePtr<MemoryEditor> m_editor;
    SafePtr<Gtk::Label> m_address_label;
    SafePtr<Gtk::Entry> m_address_entry;
    SafePtr<Gtk::Button> m_jump_button;
    SafePtr<Gtk::HBox> m_hbox;
    SafePtr<Gtk::VBox> m_container;
    IDebuggerSafePtr m_debugger;

    Priv (IDebuggerSafePtr& a_debugger) :
        m_editor (new MemoryEditor()),
        m_address_label (new Gtk::Label(_("Address:"))),
        m_address_entry (new Gtk::Entry()),
        m_jump_button (new Gtk::Button(_("Show"))),
        m_hbox (new Gtk::HBox()),
        m_container (new Gtk::VBox()),
        m_debugger (a_debugger)
    {
        m_hbox->pack_start (*m_address_label, Gtk::PACK_SHRINK);
        m_hbox->pack_start (*m_address_entry, Gtk::PACK_SHRINK);
        m_hbox->pack_start (*m_jump_button, Gtk::PACK_SHRINK);
        m_container->pack_start (*m_hbox, Gtk::PACK_SHRINK);
        m_container->pack_start (m_editor->widget ());

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
                set_widgets_sensitive (true);
                break;
            default:
                set_widgets_sensitive (false);
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
        size_t addr = get_address ();
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
        THROW_IF_FAIL (m_debugger);
        size_t addr = get_address ();
        LOG_DD ("got address: " << UString::from_int(addr));
        if (validate_address (addr))
        {
            m_debugger->read_memory (addr, 100);
        }
        NEMIVER_CATCH
    }

    size_t get_address ()
    {
        THROW_IF_FAIL (m_address_entry);
        std::istringstream istream (m_address_entry->get_text ());
        size_t addr;
        istream >> std::hex >> addr;
        return addr;
    }

    bool validate_address (size_t addr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        // FIXME: implement validation
        if (addr)
        {
            return true;
        }
        return false;
    }

    void set_widgets_sensitive (bool enable = true)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (m_address_entry && m_jump_button);
        m_address_entry->set_sensitive (enable);
        m_jump_button->set_sensitive (enable);
    }

    void on_memory_read_response (size_t a_addr,
            std::vector<uint8_t> a_values, const UString& /*a_cookie*/)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY
        THROW_IF_FAIL (m_address_entry && m_editor);
        ostringstream addr;
        addr << "0x" << std::hex << a_addr;
        m_address_entry->set_text (addr.str ());
        m_editor->set_data (a_addr, a_values);
        NEMIVER_CATCH
    }
};

MemoryView::MemoryView (IDebuggerSafePtr& a_debugger) :
    m_priv (new Priv(a_debugger))
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
    THROW_IF_FAIL (m_priv && m_priv->m_editor);
    m_priv->m_editor->reset ();
}

} // namespace nemiver
