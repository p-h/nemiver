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
#ifndef __NMV_MEMORY_VIEW_H__
#define __NMV_MEMORY_VIEW_H__

#include <gtkmm/widget.h>
#include <pangomm/fontdescription.h>
#include <libglademm.h>
#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"

using nemiver::common::SafePtr;
using nemiver::common::UString;

namespace nemiver {

class NEMIVER_API MemoryView : public nemiver::common::Object {
    // non-copyable
    MemoryView (const MemoryView&);
    MemoryView& operator= (const MemoryView&);

    struct Priv;
    SafePtr<Priv> m_priv;

    public:
    MemoryView (IDebuggerSafePtr& a_debugger);
    virtual ~MemoryView ();
    Gtk::Widget& widget () const;
    void clear ();
    void modify_font (const Pango::FontDescription& a_font_desc);
};

}   // namespace nemiver
#endif // __NMV_MEMORY_VIEW_H__
