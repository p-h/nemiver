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
#include "config.h"
#include <map>
#include <glib/gi18n.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#ifdef WITH_SOURCEVIEWMM2
#include <gtksourceviewmm/sourcemark.h>
#else
#include <gtksourceviewmm/sourcemarker.h>
#endif // WITH_SOURCEVIEWMM2
#include <gtksourceviewmm/sourceiter.h>
#include "common/nmv-exception.h"
#include "common/nmv-sequence.h"
#include "common/nmv-ustring.h"
#include "nmv-source-editor.h"

using namespace std ;
using namespace nemiver::common;
#ifdef WITH_SOURCEVIEWMM2
using gtksourceview::SourceMark;
#else
using gtksourceview::SourceMarker;
#endif // WITH_SOURCEVIEWMM2
using gtksourceview::SourceIter;
using gtksourceview::SearchFlags;

namespace nemiver {

const char* BREAKPOINT_ENABLED_CATEGORY = "breakpoint-enabled-category";
const char* BREAKPOINT_DISABLED_CATEGORY = "breakpoint-disabled-category";
const char* WHERE_CATEGORY = "line-pointer-category";

const char* WHERE_MARK = "where-marker";

class SourceView : public gtksourceview::SourceView {

    sigc::signal<void, int> m_marker_region_got_clicked_signal ;

public:
    SourceView (Glib::RefPtr<SourceBuffer> &a_buf) :
        gtksourceview::SourceView (a_buf)
    {
        init_font ();
        enable_events () ;
    }

    SourceView () :
        gtksourceview::SourceView ()
    {
        init_font ();
    }

    void init_font ()
    {
        Pango::FontDescription font("monospace");
        modify_font(font);
    }

    void enable_events ()
    {
        add_events (Gdk::LEAVE_NOTIFY_MASK
                    |Gdk::BUTTON_PRESS_MASK
                    |Gdk::KEY_PRESS_MASK) ;
    }

    void do_custom_button_press_event_handling (GdkEventButton *a_event)
    {
        THROW_IF_FAIL (a_event) ;

        if (a_event->type == GDK_BUTTON_PRESS && a_event->button != 1) {
            return ;
        }
        Glib::RefPtr<Gdk::Window> markers_window =
                                        get_window (Gtk::TEXT_WINDOW_LEFT) ;
        THROW_IF_FAIL (markers_window) ;

        if (markers_window.operator->()->gobj () != a_event->window) {
            LOG_DD ("didn't clicked in markers region") ;
            return ;
        }
        LOG_DD ("got clicked in markers region !") ;
        Gtk::TextBuffer::iterator iter ;
        int line_top=0, x=0, y=0 ;

        window_to_buffer_coords (Gtk::TEXT_WINDOW_LEFT,
                                 (int)a_event->x, (int)a_event->y, x, y) ;

        get_line_at_y (iter, (int) y, line_top) ;

        THROW_IF_FAIL (iter) ;

        LOG_DD ("got clicked on line: " << iter.get_line ()) ;
        marker_region_got_clicked_signal ().emit (iter.get_line ()) ; ;
    }

    bool on_button_press_event (GdkEventButton *a_event)
    {
        if (a_event->type == GDK_BUTTON_PRESS && a_event->button == 3) {
            return false ;
        } else {
            Gtk::Widget::on_button_press_event (a_event) ;
            do_custom_button_press_event_handling (a_event) ;
            return false ;
        }
    }

    bool on_key_press_event (GdkEventKey *a_event)
    {
        Gtk::Widget::on_key_press_event (a_event) ;
        return false ;
    }

    sigc::signal<void, int>& marker_region_got_clicked_signal ()
    {
        return m_marker_region_got_clicked_signal ;
    }

};//end class Sourceview

struct SourceEditor::Priv {
    Sequence sequence ;
#ifdef WITH_SOURCEVIEWMM2
    std::map<int, Glib::RefPtr<gtksourceview::SourceMark> > markers ;
#else
    std::map<int, Glib::RefPtr<gtksourceview::SourceMarker> > markers ;
#endif  // WITH_SOURCEVIEWMM2
    UString root_dir ;
    gint current_column ;
    gint current_line ;
    nemiver::SourceView *source_view ;
    Gtk::HBox *status_box ;
    Gtk::Label *line_col_label ;
    Gtk::Label *line_count;
    sigc::signal<void, gint, gint> signal_insertion_moved ;
    sigc::signal<void, int> marker_region_got_clicked_signal ;
    sigc::signal<void, const Gtk::TextBuffer::iterator&>
                                                    insertion_changed_signal;
    UString path ;

    //**************
    //<signal slots>
    //**************
    void on_marker_region_got_clicked (int a_line)
    {
        marker_region_got_clicked_signal.emit (a_line) ;
    }

    void on_mark_set_signal (const Gtk::TextBuffer::iterator &a_iter,
                             const Glib::RefPtr<Gtk::TextBuffer::Mark> &a_mark)
    {
        if (a_mark->get_name () == "insert") {
            update_line_col_info_from_iter (a_iter) ;
        }
    }

    void on_signal_insert (const Gtk::TextBuffer::iterator &a_iter,
                           const Glib::ustring &a_text,
                           int a_unknown)
    {
        if (a_text == "" || a_unknown) {}
        update_line_col_info_from_iter (a_iter) ;
    }

    void on_signal_insertion_moved (gint a_line, gint a_col)
    {
        if (a_line || a_col) {}
        current_line = a_line ;
        current_column = a_col ;
        update_line_col_label ();
    }

    void on_signal_mark_set
                        (const Gtk::TextBuffer::iterator &a_iter,
                         const Glib::RefPtr<Gtk::TextBuffer::Mark > &a_mark)
    {
        THROW_IF_FAIL (source_view) ;

        Glib::RefPtr<Gtk::TextBuffer::Mark> insert_mark =
                                source_view->get_buffer ()->get_insert ();
        if (insert_mark == a_mark) {
            insertion_changed_signal.emit (a_iter) ;
        }
    }

    //**************
    //</signal slots>
    //**************

    void init_signals ()
    {
        source_view->marker_region_got_clicked_signal ().connect
            (sigc::mem_fun (*this,
                            &SourceEditor::Priv::on_marker_region_got_clicked)) ;
        source_view->get_buffer ()->signal_mark_set ().connect
            (sigc::mem_fun (*this, &SourceEditor::Priv::on_mark_set_signal)) ;
        source_view->get_buffer ()->signal_insert ().connect
            (sigc::mem_fun (*this, &SourceEditor::Priv::on_signal_insert)) ;
        signal_insertion_moved.connect
            (sigc::mem_fun (*this,
                            &SourceEditor::Priv::on_signal_insertion_moved)) ;
        source_view->get_buffer ()->signal_mark_set ().connect
            (sigc::mem_fun (*this, &SourceEditor::Priv::on_signal_mark_set)) ;
    }

    void update_line_col_info_from_iter (const Gtk::TextBuffer::iterator &a_iter)
    {
        current_line = a_iter.get_line () + 1;
        current_column = get_column_from_iter (a_iter) ;
        signal_insertion_moved.emit  (current_line, current_column) ;
    }

    void update_line_col_label ()
    {
        gint line_count = 0 ;
        if (source_view && source_view->get_buffer ()) {
            line_count = source_view->get_buffer ()->get_line_count () ;
        }
        UString message;
        message.printf (_("Line: %i, Column: %i, Lines: %i"),
                        current_line, current_column, line_count) ;
        line_col_label->set_text (message);
    }

    gint get_column_from_iter (const Gtk::TextBuffer::iterator &a_iter)
    {
        if (a_iter) {}
        //TODO: code this !
        return 0 ;
    }

    bool get_absolute_resource_path (const UString &a_relative_path,
                                     string &a_absolute_path)
    {
        bool result (false) ;
        string absolute_path =
            Glib::build_filename (Glib::locale_from_utf8 (root_dir),
                                  a_relative_path) ;
        if (Glib::file_test (absolute_path,
                             Glib::FILE_TEST_IS_REGULAR
                             | Glib::FILE_TEST_EXISTS)) {
            result = true;
            a_absolute_path = absolute_path ;
        } else {
            LOG ("could not find file: " << a_absolute_path) ;
        }
        return result ;
    }

    void register_breakpoint_marker_type (const UString &a_name,
                                          const UString &a_image)
    {
        string path ;
        if (!get_absolute_resource_path (a_image,
                                         path)) {
            THROW ("could not get path to " + a_image) ;
        }

        Glib::RefPtr<Gdk::Pixbuf> bm_pixbuf =
                                Gdk::Pixbuf::create_from_file (path) ;
#ifdef WITH_SOURCEVIEWMM2
        source_view->set_mark_category_pixbuf (a_name, bm_pixbuf) ;
        source_view->set_mark_category_priority (a_name, 0);
#else
        source_view->set_marker_pixbuf (a_name, bm_pixbuf) ;
#endif  // WITH_SOURCEVIEWMM2
    }

    void init ()
    {
        update_line_col_label () ;
        status_box->pack_end (*line_col_label, Gtk::PACK_SHRINK) ;
        init_signals () ;
        source_view->set_editable (false) ;
        register_breakpoint_marker_type (BREAKPOINT_ENABLED_CATEGORY,
                                         "icons/breakpoint-marker.png");
        register_breakpoint_marker_type
                                (BREAKPOINT_DISABLED_CATEGORY,
                                 "icons/breakpoint-disabled-marker.png");
    }

    Priv () :
        current_column (-1),
        current_line (-1),
        source_view (Gtk::manage (new SourceView)),
        status_box (Gtk::manage (new Gtk::HBox )),
        line_col_label (Gtk::manage (new Gtk::Label ()))

    {
        init () ;
    }

    Priv (const UString &a_root_dir,
          Glib::RefPtr<SourceBuffer> &a_buf) :
        root_dir (a_root_dir),
        current_column (-1),
        current_line (-1),
        source_view (Gtk::manage (new SourceView (a_buf))),
        status_box (Gtk::manage (new Gtk::HBox)),
        line_col_label (Gtk::manage (new Gtk::Label ()))
    {
        init () ;
    }
};//end class SourceEditor

void
SourceEditor::init ()
{
    Gtk::ScrolledWindow *scrolled (Gtk::manage (new Gtk::ScrolledWindow));
    scrolled->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC) ;
    scrolled->add (*m_priv->source_view) ;
    scrolled->show_all () ;
    scrolled->set_shadow_type (Gtk::SHADOW_IN);
    pack_start (*scrolled) ;
    pack_end (*m_priv->status_box, Gtk::PACK_SHRINK) ;

    //****************************
    //set line pointer pixbuf
    //****************************
    string path = "" ;
    if (!m_priv->get_absolute_resource_path ("icons/line-pointer.png", path)) {
        THROW ("could not get path to line-pointer.png") ;
    }
    Glib::RefPtr<Gdk::Pixbuf> lp_pixbuf = Gdk::Pixbuf::create_from_file (path) ;
#ifdef WITH_SOURCEVIEWMM2
    source_view ().set_mark_category_pixbuf (WHERE_CATEGORY, lp_pixbuf) ;
    // show this on top
    source_view ().set_mark_category_priority (WHERE_CATEGORY, 100);
    source_view ().set_show_line_marks (true) ;
#else
    source_view ().set_marker_pixbuf (WHERE_CATEGORY, lp_pixbuf) ;
    source_view ().set_show_line_markers (true) ;
#endif  // WITH_SOURCEVIEWMM2
}

SourceEditor::SourceEditor ()
{
    m_priv.reset (new Priv) ;
    init () ;
}

SourceEditor::SourceEditor (const UString &a_root_dir,
                            Glib::RefPtr<SourceBuffer> &a_buf)
{
    m_priv.reset (new Priv (a_root_dir, a_buf)) ;
    init () ;
}

SourceEditor::~SourceEditor ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

gtksourceview::SourceView&
SourceEditor::source_view () const
{
    THROW_IF_FAIL (m_priv && m_priv->source_view) ;
    return *m_priv->source_view ;
}

gint
SourceEditor::current_line () const
{
    return m_priv->current_line ;
}

void
SourceEditor::current_line (gint &a_line)
{
    m_priv->current_line = a_line;
}

gint
SourceEditor::current_column () const
{
    return m_priv->current_column ;
}

void
SourceEditor::current_column (gint &a_col)
{
    LOG_DD ("current colnum " << (int) a_col) ;
    m_priv->current_column = a_col ;
}

void
SourceEditor::move_where_marker_to_line (int a_line, bool a_do_scroll)
{
    THROW_IF_FAIL (a_line >= 0) ;

    Gtk::TextIter line_iter =
            source_view ().get_source_buffer ()->get_iter_at_line (a_line - 1) ;
    THROW_IF_FAIL (line_iter) ;

#ifdef WITH_SOURCEVIEWMM2
    Glib::RefPtr<Gtk::TextMark> where_marker =
        source_view ().get_source_buffer ()->get_mark (WHERE_MARK) ;
#else
    Glib::RefPtr<gtksourceview::SourceMarker> where_marker =
        source_view ().get_source_buffer ()->get_marker (WHERE_MARK) ;
#endif  // WITH_SOURCEVIEWMM2
    if (!where_marker) {
#ifdef WITH_SOURCEVIEWMM2
        Glib::RefPtr<Gtk::TextMark> where_marker =
            source_view ().get_source_buffer ()->create_mark
#else
        Glib::RefPtr<gtksourceview::SourceMarker> where_marker =
            source_view ().get_source_buffer ()->create_marker
#endif  // WITH_SOURCEVIEWMM2
                                                        (WHERE_MARK,
                                                         WHERE_CATEGORY,
                                                         line_iter) ;
        THROW_IF_FAIL (where_marker) ;
    } else {
#ifdef WITH_SOURCEVIEWMM2
        source_view ().get_source_buffer ()->move_mark (where_marker,
                                                        line_iter) ;
#else
        source_view ().get_source_buffer ()->move_marker (where_marker,
                                                          line_iter) ;
#endif  // WITH_SOURCEVIEWMM2
    }
    if (a_do_scroll) {
        scroll_to_line (a_line) ;
    }

}

void
SourceEditor::unset_where_marker ()
{
#ifdef WITH_SOURCEVIEWMM2
    Glib::RefPtr<Gtk::TextMark> where_marker =
        source_view ().get_source_buffer ()->get_mark (WHERE_MARK) ;
#else
    Glib::RefPtr<gtksourceview::SourceMarker> where_marker =
        source_view ().get_source_buffer ()->get_marker (WHERE_MARK) ;
#endif  // WITH_SOURCEVIEWMM2
    if (where_marker && !where_marker->get_deleted ()) {
#ifdef WITH_SOURCEVIEWMM2
        source_view ().get_source_buffer ()->delete_mark (where_marker) ;
#else
        source_view ().get_source_buffer ()->delete_marker (where_marker) ;
#endif  // WITH_SOURCEVIEWMM2
    }
}

void
SourceEditor::set_visual_breakpoint_at_line (int a_line, bool enabled)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD

    UString marker_type;
    if (enabled) {
        marker_type = BREAKPOINT_ENABLED_CATEGORY;
    } else {
        marker_type = BREAKPOINT_DISABLED_CATEGORY;
    }

    std::map<int,
#ifdef WITH_SOURCEVIEWMM2
            Glib::RefPtr<gtksourceview::SourceMark> >::iterator mark_iter =
#else
            Glib::RefPtr<gtksourceview::SourceMarker> >::iterator mark_iter =
#endif  // WITH_SOURCEVIEWMM2
                                            m_priv->markers.find (a_line);
    if (mark_iter !=  m_priv->markers.end ()) {
        if (!mark_iter->second->get_deleted ()) {
            LOG_DD ("deleting marker") ;
#ifdef WITH_SOURCEVIEWMM2
            source_view ().get_source_buffer ()->delete_mark
#else
            source_view ().get_source_buffer ()->delete_marker
#endif  // WITH_SOURCEVIEWMM2
                                                    (mark_iter->second);
        }
        m_priv->markers.erase (a_line);
    }
    // marker doesn't yet exist, so create one of the correct type
    Gtk::TextIter iter =
        source_view ().get_source_buffer ()->get_iter_at_line (a_line) ;
    THROW_IF_FAIL (iter) ;
    UString marker_name = UString::from_int (a_line);

    LOG_DD ("creating marker of type: " << marker_type) ;
#ifdef WITH_SOURCEVIEWMM2
    Glib::RefPtr<gtksourceview::SourceMark> marker =
        source_view ().get_source_buffer ()->create_mark
#else
    Glib::RefPtr<gtksourceview::SourceMarker> marker =
        source_view ().get_source_buffer ()->create_marker
#endif  // WITH_SOURCEVIEWMM2
                                        (marker_name, marker_type, iter) ;
    m_priv->markers[a_line] = marker ;
}

void
SourceEditor::remove_visual_breakpoint_from_line (int a_line)
{
#ifdef WITH_SOURCEVIEWMM2
    std::map<int, Glib::RefPtr<gtksourceview::SourceMark> >::iterator iter ;
#else
    std::map<int, Glib::RefPtr<gtksourceview::SourceMarker> >::iterator iter ;
#endif  // WITH_SOURCEVIEWMM2
    iter = m_priv->markers.find (a_line) ;
    if (iter == m_priv->markers.end ()) {
        return ;
    }
    if (!iter->second->get_deleted ())
#ifdef WITH_SOURCEVIEWMM2
        source_view ().get_source_buffer ()->delete_mark (iter->second) ;
#else
        source_view ().get_source_buffer ()->delete_marker (iter->second) ;
#endif  // WITH_SOURCEVIEWMM2
    m_priv->markers.erase (iter) ;
}

bool
SourceEditor::is_visual_breakpoint_set_at_line (int a_line) const
{
#ifdef WITH_SOURCEVIEWMM2
    std::map<int, Glib::RefPtr<gtksourceview::SourceMark> >::iterator iter ;
#else
    std::map<int, Glib::RefPtr<gtksourceview::SourceMarker> >::iterator iter ;
#endif  // WITH_SOURCEVIEWMM2
    iter = m_priv->markers.find (a_line) ;
    if (iter == m_priv->markers.end ()) {
        return false ;
    }
    return true ;
}

struct ScrollToLine {
    int m_line ;
    SourceView *m_source_view ;

    ScrollToLine () :
        m_line (0),
        m_source_view (0)
    {}

    ScrollToLine (SourceView *a_source_view, int a_line) :
        m_line (a_line),
        m_source_view (a_source_view)
    {
    }

    bool do_scroll ()
    {
        if (!m_source_view) {return false;}
        Gtk::TextIter iter = m_source_view->get_buffer ()->get_iter_at_line (m_line) ;
        if (!iter) {return false;}
        m_source_view->scroll_to (iter, 0.1) ;
        return false ;
    }
};

void
SourceEditor::scroll_to_line (int a_line)
{
    static ScrollToLine s_scroll_functor ;
    s_scroll_functor.m_line = a_line ;
    s_scroll_functor.m_source_view = m_priv->source_view ;
    Glib::signal_idle ().connect (sigc::mem_fun (s_scroll_functor,
                                                 &ScrollToLine::do_scroll)) ;
}

void
SourceEditor::scroll_to_iter (Gtk::TextIter &a_iter)
{
    if (!a_iter) {
        LOG_DD ("iter points at end of buffer") ;
        return ;
    }
    static ScrollToLine s_scroll_functor ;
    s_scroll_functor.m_line = a_iter.get_line ();
    s_scroll_functor.m_source_view = m_priv->source_view ;
    Glib::signal_idle ().connect (sigc::mem_fun (s_scroll_functor,
                                                 &ScrollToLine::do_scroll)) ;
}

void
SourceEditor::set_path (const UString &a_path)
{
    m_priv->path = a_path ;
}

void
SourceEditor::get_path (UString &a_path) const
{
    a_path = m_priv->path ;
}

void
SourceEditor::get_file_name (UString &a_file_name)
{
    string path ;
    path = Glib::locale_from_utf8 (m_priv->path) ;
    path = Glib::path_get_basename (path) ;
    a_file_name = Glib::locale_to_utf8 (path) ;
}

bool
is_word_delimiter (gunichar a_char)
{
    if (!isalnum (a_char) && a_char != '_') {
        return true ;
    }
    return false ;
}

bool
SourceEditor::get_word_at_position (int a_x,
                                    int a_y,
                                    UString &a_word,
                                    Gdk::Rectangle &a_start_rect,
                                    Gdk::Rectangle &a_end_rect) const
{
    LOG_FUNCTION_SCOPE_NORMAL_DD

    THROW_IF_FAIL (m_priv) ;
    int buffer_x=0, buffer_y=0 ;
    source_view ().window_to_buffer_coords (Gtk::TEXT_WINDOW_TEXT,
                                            (int)a_x,
                                            (int)a_y,
                                            buffer_x, buffer_y) ;
    Gtk::TextBuffer::iterator clicked_at_iter ;
    source_view ().get_iter_at_location (clicked_at_iter, buffer_x, buffer_y) ;
    if (!clicked_at_iter) {
        return false ;
    }

    //go find the first white word delimiter before clicked_at_iter
    Gtk::TextBuffer::iterator cur_iter = clicked_at_iter;
    if (!cur_iter) {return false;}

    while (cur_iter.backward_char ()
           && !is_word_delimiter (cur_iter.get_char ())) {}
    THROW_IF_FAIL (cur_iter.forward_char ()) ;
    Gtk::TextBuffer::iterator start_word_iter = cur_iter ;

    //go find the first word delimiter after clicked_at_iter
    cur_iter = clicked_at_iter ;
    while (cur_iter.forward_char ()
           && !is_word_delimiter (cur_iter.get_char ())) {}
    Gtk::TextBuffer::iterator end_word_iter = cur_iter ;

    UString var_name = start_word_iter.get_slice (end_word_iter);
    while (var_name != "" && !isalpha (var_name[0]) && var_name[0] != '_') {
        var_name.erase (0, 1) ;
    }
    while (var_name != ""
           && !isalnum (var_name[var_name.size () - 1])
           && var_name[var_name.size () - 1] != '_') {
        var_name.erase (var_name.size () - 1, 1) ;
    }

    Gdk::Rectangle start_rect, end_rect ;
    source_view ().get_iter_location (start_word_iter, start_rect) ;
    source_view ().get_iter_location (end_word_iter, end_rect) ;
    if (!(start_rect.get_x () <= buffer_x) || !(buffer_x <= end_rect.get_x ())) {
        LOG_DD ("mouse not really on word: '" << var_name << "'") ;
        return false ;
    }
    LOG_DD ("got variable candidate name: '" << var_name << "'") ;
    a_word = var_name ;
    a_start_rect = start_rect ;
    a_end_rect = end_rect ;
    return true ;
}

bool
SourceEditor::do_search (const UString &a_str,
                         Gtk::TextIter &a_start,
                         Gtk::TextIter &a_end,
                         bool a_match_case,
                         bool a_match_entire_word,
                         bool a_search_backwards,
                         bool a_clear_selection)
{
    Glib::RefPtr<SourceBuffer> source_buffer = source_view ().get_source_buffer () ;
    THROW_IF_FAIL (source_buffer) ;

    if (a_clear_selection) {
        source_buffer->select_range (source_buffer->end (),
                                     source_buffer->end ()) ;
    }

    SourceIter search_iter, limit ;
    if (a_search_backwards) {
        search_iter = source_buffer->end () ;
        search_iter-- ;
        limit = source_buffer->begin () ;
    } else {
        search_iter = source_buffer->begin () ;
        limit = source_buffer->end () ;
        limit-- ;
    }

    Gtk::TextIter start, end ;
    if (source_buffer->get_selection_bounds (start, end)) {
        if (a_search_backwards) {
            search_iter = start ;
        } else {
            search_iter = end ;
        }
    }

    //*********************
    //build search flags
    //**********************
    namespace gsv=gtksourceview ;
    gsv::SearchFlags search_flags = gsv::SEARCH_TEXT_ONLY ;
    if (!a_match_case) {
        search_flags |= gsv::SEARCH_CASE_INSENSITIVE ;
    }

    bool found=false ;
    if (a_search_backwards) {
        if (search_iter.backward_search (a_str, search_flags,
                                         a_start, a_end, limit)) {
            found = true ;
        }
    } else {
        if (search_iter.forward_search (a_str, search_flags,
                    a_start, a_end, limit)) {
            found = true ;
        }
    }

    if (found && a_match_entire_word) {
        Gtk::TextIter iter = a_start ;
        if (iter.backward_char ()) {
            if (!is_word_delimiter (*iter)) {
                found = false ;
            }
        }
        if (found) {
            iter = a_end ;
            if (!is_word_delimiter (*iter)) {
                found = false;
            }
        }
    }

    if (found) {
        source_buffer->select_range (a_start, a_end) ;
        scroll_to_iter (a_start) ;
        return true ;
    }
    return false ;
}

sigc::signal<void, int>&
SourceEditor::marker_region_got_clicked_signal () const
{
    return m_priv->marker_region_got_clicked_signal ;
}

sigc::signal<void, const Gtk::TextBuffer::iterator&>&
SourceEditor::insertion_changed_signal () const
{
    return m_priv->insertion_changed_signal ;
}

}//end namespace nemiver

