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
#include <bitset>
#include <iomanip>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <glib/gi18n.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include "nmv-ui-utils.h"
#include "nmv-memory-view.h"
#include "nmv-i-debugger.h"

namespace nemiver {

enum MemoryOutputFormat
{
    OUTPUT_FORMAT_HEX,
    OUTPUT_FORMAT_DECIMAL,
    OUTPUT_FORMAT_OCTAL,
    OUTPUT_FORMAT_BINARY
};


class FormatComboBox : public Gtk::ComboBox
{
    public:
        FormatComboBox ()
        {
            m_model = Gtk::ListStore::create (m_cols);
            THROW_IF_FAIL (m_model);
            Gtk::TreeModel::iterator iter = m_model->append ();
            (*iter)[m_cols.name] = _("Hexadecimal");
            (*iter)[m_cols.format] = OUTPUT_FORMAT_HEX;
            iter = m_model->append ();
            (*iter)[m_cols.name] = _("Decimal");
            (*iter)[m_cols.format] = OUTPUT_FORMAT_DECIMAL;
            iter = m_model->append ();
            (*iter)[m_cols.name] = _("Octal");
            (*iter)[m_cols.format] = OUTPUT_FORMAT_OCTAL;
            iter = m_model->append ();
            (*iter)[m_cols.name] = _("Binary");
            (*iter)[m_cols.format] = OUTPUT_FORMAT_BINARY;
            set_model (m_model);
            pack_start (m_cols.name);
            set_active (0);
        }

        MemoryOutputFormat get_format () const
        {
            Gtk::TreeModel::iterator iter = get_active ();
            if (iter)
            {
                return (*iter)[m_cols.format];
            }
            else return OUTPUT_FORMAT_HEX;
        }

    private:
        Glib::RefPtr<Gtk::ListStore> m_model;
        struct FormatModelColumns : public Gtk::TreeModelColumnRecord
        {
            FormatModelColumns () {add (name); add (format);}
            Gtk::TreeModelColumn<Glib::ustring> name;
            Gtk::TreeModelColumn<MemoryOutputFormat> format;
        } m_cols;

};

struct MemoryView::Priv {
public:
    SafePtr<Gtk::Label> m_address_label;
    SafePtr<Gtk::Entry> m_address_entry;
    SafePtr<Gtk::Button> m_jump_button;
    SafePtr<Gtk::HBox> m_hbox;
    SafePtr<Gtk::VBox> m_container;
    SafePtr<Gtk::Label> m_format_label;
    FormatComboBox m_format_combo;
    SafePtr<Gtk::ScrolledWindow> m_scrolledwindow;
    Glib::RefPtr<Gtk::TextBuffer> m_textbuffer;
    SafePtr<Gtk::TextView> m_textview;
    MemoryOutputFormat m_format;
    IDebuggerSafePtr m_debugger;

    Priv (IDebuggerSafePtr& a_debugger) :
        m_address_label (new Gtk::Label (_("Address:"))),
        m_address_entry (new Gtk::Entry ()),
        m_jump_button (new Gtk::Button (_("Show"))),
        m_hbox (new Gtk::HBox ()),
        m_container (new Gtk::VBox ()),
        m_format_label (new Gtk::Label (_("Format:"))),
        m_scrolledwindow (new Gtk::ScrolledWindow ()),
        m_textbuffer (Gtk::TextBuffer::create ()),
        m_textview (new Gtk::TextView (m_textbuffer)),
        m_debugger (a_debugger)
    {
        m_textview->set_wrap_mode (Gtk::WRAP_WORD);
        m_textview->set_left_margin (6);
        m_textview->set_right_margin (6);
        m_scrolledwindow->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        m_scrolledwindow->add (*m_textview);
        m_scrolledwindow->set_shadow_type (Gtk::SHADOW_IN);

        m_hbox->set_spacing (6);
        m_hbox->set_border_width (3);
        m_hbox->pack_start (*m_address_label, Gtk::PACK_SHRINK);
        m_hbox->pack_start (*m_address_entry, Gtk::PACK_SHRINK);
        m_hbox->pack_start (*m_format_label, Gtk::PACK_SHRINK);
        m_hbox->pack_start (m_format_combo, Gtk::PACK_SHRINK);
        m_hbox->pack_start (*m_jump_button, Gtk::PACK_SHRINK);
        m_container->pack_start (*m_hbox, Gtk::PACK_SHRINK);
        m_container->pack_start (*m_scrolledwindow);

        m_format = OUTPUT_FORMAT_HEX;
        connect_signals ();
    }

    void connect_signals ()
    {
        THROW_IF_FAIL (m_debugger);
        m_debugger->state_changed_signal ().connect (sigc::mem_fun (this,
                    &Priv::on_debugger_state_changed));
        m_debugger->stopped_signal ().connect (sigc::mem_fun (this,
                    &Priv::on_debugger_stopped));
        m_debugger->read_memory_signal ().connect (sigc::mem_fun (this,
                    &Priv::on_memory_read_response));
        THROW_IF_FAIL (m_jump_button);
        m_jump_button->signal_clicked ().connect (sigc::mem_fun (this,
                    &Priv::do_memory_read));
        m_format_combo.signal_changed ().connect (sigc::mem_fun (this,
                    &Priv::do_memory_read));
        THROW_IF_FAIL (m_address_entry);
        m_address_entry->signal_activate ().connect (sigc::mem_fun (this,
                    &Priv::do_memory_read));
    }

    void on_debugger_state_changed (IDebugger::State a_state)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
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

    void do_memory_read ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY
        THROW_IF_FAIL (m_debugger);
        size_t addr = get_address ();
        if (validate_address (addr))
        {
            m_debugger->read_memory (addr, 200);
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
        do_memory_read ();
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

    bool validate_address (size_t a_addr)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        // FIXME: implement validation
        if (a_addr)
        {
            return true;
        }
        return false;
    }

    void set_widgets_sensitive (bool a_enable = true)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (m_address_entry && m_jump_button);
        m_address_entry->set_sensitive (a_enable);
        m_jump_button->set_sensitive (a_enable);
    }

    void on_memory_read_response (size_t a_addr,
            std::vector<uint8_t> a_values, const UString& /*a_cookie*/)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY
        THROW_IF_FAIL (m_address_entry);
        ostringstream addr;
        addr << std::showbase << std::hex << a_addr;
        m_address_entry->set_text (addr.str ());
        set_data (a_addr, a_values);
        NEMIVER_CATCH
    }

    void set_data (size_t a_start_addr, std::vector<uint8_t> a_data)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (m_textbuffer);
        std::ostringstream ostream;
        ostream << std::showbase << std::hex << a_start_addr << ":"
            << std::noshowbase << std::endl;
        for (std::vector<uint8_t>::const_iterator it = a_data.begin ();
                it != a_data.end (); ++it)
        {
            switch (m_format_combo.get_format ())
            {
                case OUTPUT_FORMAT_BINARY:
                    // thanks to Nicolai Josuttis for the tip:
                    // http://www.josuttis.com/libbook/cont/bitset2.cpp.html
                    ostream <<
                        std::bitset<std::numeric_limits<uint8_t>::digits>(*it);
                    break;
                case OUTPUT_FORMAT_DECIMAL:
                    ostream << std::setw(3) << std::dec << (int) *it;
                    break;
                case OUTPUT_FORMAT_OCTAL:
                    ostream << std::setw(4) << std::showbase << std::oct
                        << (int) *it;
                    break;
                case OUTPUT_FORMAT_HEX:
                default:
                    ostream << std::setw(4) << std::showbase << std::hex
                        << (int) *it;
            }
            ostream << " ";
        }

        m_textbuffer->set_text (ostream.str());
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
    THROW_IF_FAIL (m_priv && m_priv->m_textbuffer && m_priv->m_address_entry);
    m_priv->m_textbuffer->set_text ("");
    m_priv->m_address_entry->set_text ("");
}

void
MemoryView::modify_font (const Pango::FontDescription& a_font_desc)
{
    THROW_IF_FAIL (m_priv && m_priv->m_textview);
    m_priv->m_textview->modify_font (a_font_desc);
}

} // namespace nemiver
