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
#ifndef __NMV_MEMORY_EDITOR_H__
#define __NMV_MEMORY_EDITOR_H__

#include <gtkmm/widget.h>
#include <sigc++/signal.h>
#include <nemiver/common/nmv-safe-ptr-utils.h>

namespace nemiver
{
    class MemoryEditor
    {
        public:
            MemoryEditor();
            virtual ~MemoryEditor();
            void set_data (size_t start_addr, std::vector<uint8_t> data);
            void update_byte (size_t addr, uint8_t value);
            void reset ();
            sigc::signal<void, size_t, uint8_t>& signal_value_changed () const;
            Gtk::Widget& widget ();

        private:
            struct Priv;
            nemiver::common::SafePtr<Priv> m_priv;

    };
}

#endif // __NMV_MEMORY_EDITOR_H__
