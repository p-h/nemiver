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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include <map>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtksourceviewmm/sourcemarker.h>
#include "nmv-exception.h"
#include "nmv-source-editor.h"
#include "nmv-ustring.h"
#include "nmv-sequence.h"

using namespace std ;
using namespace nemiver::common ;
using gtksourceview::SourceMarker ;

namespace nemiver {

class SourceView : public gtksourceview::SourceView {

    sigc::signal<void, int> m_marker_region_got_clicked_signal ;

public:
    SourceView (Glib::RefPtr<SourceBuffer> &a_buf) :
        gtksourceview::SourceView (a_buf)
    {}

    SourceView () :
        gtksourceview::SourceView ()
    {}

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
    std::map<int, Glib::RefPtr<gtksourceview::SourceMarker> > markers ;
    UString root_dir ;
    gint current_column ;
    gint current_line ;
    nemiver::SourceView *source_view ;
    Gtk::HBox *status_box ;
    Gtk::Label *line_col_label ;
    Gtk::Label *line_count;
    sigc::signal<void, gint, gint> signal_insertion_moved ;
    sigc::signal<void, int> marker_region_got_clicked_signal ;
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
        update_line_col_info_from_iter (a_iter) ;
    }

    void on_signal_insertion_moved (gint a_line, gint a_col)
    {
        update_line_col_label ();
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
        line_col_label->set_text
            (UString ("line: ") + UString::from_int (current_line)
             + UString (", column: " + UString::from_int (current_column))
             + UString (", lines: ") + UString::from_int (line_count)) ;
    }

    gint get_column_from_iter (const Gtk::TextBuffer::iterator &a_iter)
    {
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

    void register_breakpoint_marker_name (const UString &a_name)
    {
        string path ;
        if (!get_absolute_resource_path ("icons/breakpoint-marker.png",
                                                 path)) {
            THROW ("could not get path to breakpoint-marker.png") ;
        }

        Glib::RefPtr<Gdk::Pixbuf> bm_pixbuf =
                                Gdk::Pixbuf::create_from_file (path) ;
        source_view->set_marker_pixbuf (a_name, bm_pixbuf) ;
    }

    void init ()
    {
        update_line_col_label () ;
        status_box->pack_end (*line_col_label, Gtk::PACK_SHRINK) ;
        init_signals () ;
        source_view->set_editable (false) ;
    }

    Priv () :
        current_column (1),
        current_line (1),
        source_view (Gtk::manage (new SourceView)),
        status_box (Gtk::manage (new Gtk::HBox )),
        line_col_label (Gtk::manage (new Gtk::Label ()))

    {
        init () ;
    }

    Priv (const UString &a_root_dir,
          Glib::RefPtr<SourceBuffer> &a_buf) :
        root_dir (a_root_dir),
        current_column (1),
        current_line (1),
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
    pack_start (*scrolled) ;
    pack_end (*m_priv->status_box, Gtk::PACK_SHRINK) ;

    //****************************
    //set line pointer pixbuf
    //****************************
    string path = "" ;
    if (!m_priv->get_absolute_resource_path ("icons/line-pointer.xpm", path)) {
        THROW ("could not get path to line-pointer.xpm") ;
    }
    Glib::RefPtr<Gdk::Pixbuf> lp_pixbuf = Gdk::Pixbuf::create_from_file (path) ;
    source_view ().set_marker_pixbuf ("line-pointer-marker", lp_pixbuf) ;

    source_view ().set_show_line_markers (true) ;
    source_view ().set_show_line_numbers (true);
}

SourceEditor::SourceEditor ()
{
    m_priv = new Priv ;
    init () ;
}

SourceEditor::SourceEditor (const UString &a_root_dir,
                            Glib::RefPtr<SourceBuffer> &a_buf)
{
    m_priv = new Priv (a_root_dir, a_buf) ;
    init () ;
}

SourceEditor::~SourceEditor ()
{
    LOG ("deleted") ;
}

gtksourceview::SourceView&
SourceEditor::source_view ()
{
    THROW_IF_FAIL (m_priv && m_priv->source_view) ;
    return *m_priv->source_view ;
}

gint
SourceEditor::current_line ()
{
    return m_priv->current_line ;
}

void
SourceEditor::current_line (gint &a_line)
{
    m_priv->current_line = a_line;
}

gint
SourceEditor::current_column ()
{
    return m_priv->current_column ;
}

void
SourceEditor::current_column (gint &a_col)
{
    m_priv->current_column = a_col ;
}

void
SourceEditor::move_where_marker_to_line (int a_line)
{
    THROW_IF_FAIL (a_line >= 0) ;

    Gtk::TextIter line_iter =
            source_view ().get_source_buffer ()->get_iter_at_line (a_line - 1) ;
    THROW_IF_FAIL (line_iter) ;

    Glib::RefPtr<SourceMarker> where_marker =
        source_view ().get_source_buffer ()->get_marker ("where-marker") ;
    if (!where_marker) {
        Glib::RefPtr<SourceMarker> where_marker =
            source_view ().get_source_buffer ()->create_marker
                                                        ("where-marker",
                                                         "line-pointer-marker",
                                                         line_iter) ;
        THROW_IF_FAIL (where_marker) ;
    } else {
        source_view ().get_source_buffer ()->move_marker (where_marker,
                                                          line_iter) ;
    }
    scroll_to_line (a_line) ;
}

void
SourceEditor::unset_where_marker ()
{
    Glib::RefPtr<SourceMarker> where_marker =
        source_view ().get_source_buffer ()->get_marker ("where-marker") ;
    if (where_marker) {
        source_view ().get_source_buffer ()->delete_marker (where_marker) ;
    }
}

void
SourceEditor::set_visual_breakpoint_at_line (int a_line)
{
    if (m_priv->markers.find (a_line) !=  m_priv->markers.end ()) {
        return ;
    }

    Gtk::TextIter iter =
        source_view ().get_source_buffer ()->get_iter_at_line (a_line) ;
    THROW_IF_FAIL (iter) ;
    UString marker_name = UString::from_int (a_line);
    m_priv->register_breakpoint_marker_name (marker_name) ;

    Glib::RefPtr<gtksourceview::SourceMarker> marker =
        source_view ().get_source_buffer ()->create_marker
            (marker_name, marker_name, iter) ;
    m_priv->markers[a_line] = marker ;
}

void
SourceEditor::remove_visual_breakpoint_from_line (int a_line)
{
    std::map<int, Glib::RefPtr<gtksourceview::SourceMarker> >::iterator iter ;
    iter = m_priv->markers.find (a_line) ;
    if (iter == m_priv->markers.end ()) {
        return ;
    }
    source_view ().get_source_buffer ()->delete_marker (iter->second) ;
    m_priv->markers.erase (iter) ;
}

bool
SourceEditor::is_visual_breakpoint_set_at_line (int a_line)
{
    std::map<int, Glib::RefPtr<gtksourceview::SourceMarker> >::iterator iter ;
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
        m_source_view (NULL)
    {}

    ScrollToLine (SourceView *a_source_view,
                  int a_line) :
        m_line (a_line),
        m_source_view (a_source_view)
    {
    }

    bool do_scroll ()
    {
        if (!m_source_view) {return false;}
        Gtk::TextIter iter =
            m_source_view->get_buffer ()->get_iter_at_line (m_line) ;
        if (!iter) {return false;}
        m_source_view->scroll_to (iter) ;
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
SourceEditor::set_path (const UString &a_path)
{
    m_priv->path = a_path ;
}

UString
SourceEditor::get_path () const
{
    return m_priv->path ;
}

sigc::signal<void, int>&
SourceEditor::marker_region_got_clicked_signal ()
{
    return m_priv->marker_region_got_clicked_signal ;
}
}//end namespace nemiver

