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
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-ustring.h"

using gtksourceview::SourceView;
using gtksourceview::SourceBuffer;
using Gtk::VBox;
using nemiver::common::SafePtr;
using nemiver::common::UString;
using std::list;

namespace nemiver {

extern const char* BREAKPOINT_ENABLED_CATEGORY;
extern const char* BREAKPOINT_DISABLED_CATEGORY;
extern const char* WHERE_CATEGORY;

extern const char* WHERE_MARK;


class SourceEditor : public  VBox {
    //non copyable
    SourceEditor (const SourceEditor&);
    SourceEditor& operator= (const SourceEditor&);
    struct Priv;
    SafePtr<Priv> m_priv;

    void init ();
    SourceEditor ();

public:

    SourceEditor (const UString &a_root_dir,
                  Glib::RefPtr<SourceBuffer> &a_buf);
    virtual ~SourceEditor ();
    SourceView& source_view () const;
    gint current_line () const;
    void current_line (gint &a_line);
    gint current_column () const;
    void current_column (gint &a_col);
    void move_where_marker_to_line (int a_line, bool a_do_scroll=true);
    void unset_where_marker ();
    void set_visual_breakpoint_at_line (int a_line, bool enabled=true);
    void remove_visual_breakpoint_from_line (int a_line);
    bool is_visual_breakpoint_set_at_line (int a_line) const;
    void scroll_to_line (int a_line);
    void scroll_to_iter (Gtk::TextIter &a_iter);
    void set_path (const UString &a_path);
    void get_path (UString &a_path) const;
    void get_file_name (UString &a_file_name);
    bool get_word_at_position (int a_x,
                               int a_y,
                               UString &a_word,
                               Gdk::Rectangle &a_start_rect,
                               Gdk::Rectangle &a_end_rect) const;

    bool do_search (const UString &a_str,
                    Gtk::TextIter &a_start,
                    Gtk::TextIter &a_end,
                    bool a_match_case=false,
                    bool a_match_entire_word=false,
                    bool a_search_backwards=false,
                    bool a_clear_selection=false);

    /// \name signals
    /// @{
    sigc::signal<void,
                 int/*line clicked*/,
                 bool/*dialog requested*/>&
                                 marker_region_got_clicked_signal () const;
    sigc::signal<void, const Gtk::TextBuffer::iterator&>&
                                 insertion_changed_signal () const;
    /// @}
};//end class SourceEditor

}//end namespace nemiver

#endif //__NEMIVER_DBG_SOURCE_EDITOR_H__

