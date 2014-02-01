//Author: Jonathon Jongsma
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
#ifndef __NMV_HEX_EDITOR_H__
#define __NMV_HEX_EDITOR_H__

#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-ustring.h"
#include <uicommon/nmv-hex-document.h>
#include <gtkhex.h>  // for GROUP_* defines

namespace Gtk {
    class Widget;
}
using nemiver::common::Object;
using nemiver::common::SafePtr;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::UString;

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (Hex)

class Editor;
typedef SafePtr<Editor, ObjectRef, ObjectUnref> EditorSafePtr;

class Editor : public Object {
    class Priv;
    SafePtr<Priv> m_priv;

protected:
    Editor (const DocumentSafePtr &a_document);

public:
    virtual ~Editor ();
    static EditorSafePtr create (const DocumentSafePtr &a_document);
    void set_cursor (int);
    void set_cursor_xy (int, int);
    void set_nibble (int);

    guint get_cursor ();
    guchar get_byte (guint);

    void set_group_type (guint);

    void set_starting_offset (int a_starting_offset);
    void show_offsets (bool show=true);
    void set_font (const Pango::FontDescription& a_desc);

    void set_insert_mode (bool);

    void set_geometry (int cpl, int vis_lines);
    void get_geometry (int& cpl, int& vis_lines) const;

    void copy_to_clipboard ();
    void cut_to_clipboard ();
    void paste_from_clipboard ();

    void set_selection (int start, int end);
    bool get_selection (int& start, int& end);
    void clear_selection ();
    void delete_selection ();

    virtual Gtk::Container& get_widget () const;
};//end class Editor

NEMIVER_END_NAMESPACE (Hex)
NEMIVER_END_NAMESPACE (nemiver)
#endif //__NMV_HEX_EDITOR_H__

