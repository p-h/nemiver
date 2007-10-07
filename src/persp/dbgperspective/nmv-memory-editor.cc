/*******************************************************************************
 *  PROJECT: Nemiver
 *
 *  AUTHOR: Jonathon Jongsma
 *
 *  Copyright (c) 2007 Jonathon Jongsma
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
#include <gtkmm/scrolledwindow.h>
#include <nemiver/common/nmv-exception.h>
#include "nmv-memory-editor.h"

using nemiver::common::SafePtr;

namespace nemiver
{
    struct MemoryEditor::Priv
    {
        Priv ()
        {
            m_textbuffer = Gtk::TextBuffer::create();
            m_textview.reset (new Gtk::TextView (m_textbuffer));
            m_scrolledwindow.reset (new Gtk::ScrolledWindow ());
            m_scrolledwindow->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
            m_scrolledwindow->add (*m_textview);
        }

        mutable sigc::signal<void, size_t, uint8_t> m_signal_value_changed;
        SafePtr<Gtk::TextView> m_textview;
        SafePtr<Gtk::ScrolledWindow> m_scrolledwindow;
        Glib::RefPtr<Gtk::TextBuffer> m_textbuffer;
    };

    MemoryEditor::MemoryEditor () :
        m_priv (new Priv())
    { }

    MemoryEditor::~MemoryEditor ()
    { }

    void MemoryEditor::set_data (size_t start_addr, std::vector<uint8_t> data)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (m_priv && m_priv->m_textbuffer);
        std::ostringstream ostream;
        ostream << std::hex << start_addr << ": ";
        for (std::vector<uint8_t>::const_iterator it = data.begin ();
                it != data.end (); ++it)
        {
            ostream << (int) *it << " ";
        }

        m_priv->m_textbuffer->set_text (ostream.str());
    }

    void MemoryEditor::update_byte (size_t /*addr*/, uint8_t /*value*/)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (m_priv);
    }

    sigc::signal<void, size_t, uint8_t>& MemoryEditor::signal_value_changed () const
    {
        THROW_IF_FAIL (m_priv);
        return m_priv->m_signal_value_changed;
    }

    Gtk::Widget& MemoryEditor::widget ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        THROW_IF_FAIL (m_priv && m_priv->m_scrolledwindow);
        return *m_priv->m_scrolledwindow;
    }

    void MemoryEditor::reset ()
    {
        THROW_IF_FAIL (m_priv && m_priv->m_textbuffer);
        m_priv->m_textbuffer->set_text ("");
    }
}
