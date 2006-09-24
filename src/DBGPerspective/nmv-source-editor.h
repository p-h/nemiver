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
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NEMIVER_SOURCE_EDITOR_H__
#define __NEMIVER_SOURCE_EDITOR_H__

#include <list>
#include <gtkmm/box.h>
#include <gtksourceviewmm/sourceview.h>
#include "nmv-safe-ptr-utils.h"
#include "nmv-ustring.h"

using gtksourceview::SourceView ;
using gtksourceview::SourceBuffer ;
using Gtk::VBox ;
using nemiver::common::SafePtr ;
using nemiver::common::UString ;
using std::list ;

namespace nemiver {

class SourceEditor : public  VBox {
    //non copyable
    SourceEditor (const SourceEditor&) ;
    SourceEditor& operator= (const SourceEditor&) ;
    struct Priv ;
    SafePtr<Priv> m_priv ;

    void init () ;
    SourceEditor () ;

public:

    SourceEditor (const UString &a_root_dir,
                  Glib::RefPtr<SourceBuffer> &a_buf) ;
    virtual ~SourceEditor () ;
    SourceView& source_view () ;
    gint current_line () ;
    void current_line (gint &a_line) ;
    gint current_column () ;
    void current_column (gint &a_col) ;
    void move_where_marker_to_line (int a_line) ;
    void unset_where_marker () ;
    void set_visual_breakpoint_at_line (int a_line) ;
    void remove_visual_breakpoint_from_line (int a_line) ;
    bool is_visual_breakpoint_set_at_line (int a_line) ;
    void scroll_to_line (int a_line) ;
    void set_path (const UString &a_path) ;
    UString get_path () const ;

    /// \name signals
    /// @{
    sigc::signal<void, int>& marker_region_got_clicked_signal () ;
    /// @}
};//end class SourceEditor

}//end namespace nemiver

#endif //__NEMIVER_DBG_SOURCE_EDITOR_H__

