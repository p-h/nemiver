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
#ifndef __NMV_DBG_PERSPECTIVE_H__
#define __NMV_DBG_PERSPECTIVE_H__

#include "nmv-i-perspective.h"
#include "nmv-source-editor.h"
#include "nmv-i-debugger.h"
namespace nemiver {

class DBGPerspective : public IPerspective {
    //non copyable
    DBGPerspective (const IPerspective&) ;
    DBGPerspective& operator= (const IPerspective&) ;
    struct Priv ;
    SafePtr<Priv> m_priv ;

private:

    struct SlotedButton : Gtk::Button {
        UString uri ;
        DBGPerspective *perspective ;

        SlotedButton () :
            Gtk::Button (),
            perspective (NULL)
        {}

        SlotedButton (const Gtk::StockID &a_id) :
            Gtk::Button (a_id),
            perspective (NULL)
        {}

        void on_clicked ()
        {
            if (perspective) {
                perspective->close_file (uri) ;
            }
        }

        ~SlotedButton ()
        {
        }
    };

    //************
    //<signal slots>
    //************
    void on_open_action () ;
    void on_close_action () ;
    void on_execute_program_action () ;
    void on_run_action () ;
    void on_next_action () ;
    void on_step_into_action () ;
    void on_step_out_action () ;
    void on_continue_action () ;
    void on_set_breakpoint_action () ;
    void on_unset_breakpoint_action () ;

    void on_switch_page_signal (GtkNotebookPage *a_page, guint a_page_num) ;
    void on_debugger_stdout_signal (IDebugger::CommandAndOutput &a_cao) ;
    void on_debugger_ready_signal (bool a_is_ready) ;

    void on_insert_in_command_view_signal (const Gtk::TextBuffer::iterator &a_iter,
                                           const Glib::ustring &a_text,
                                           int a_dont_know) ;
    bool on_button_pressed_in_source_view_signal (GdkEventButton *a_event) ;
    //************
    //</signal slots>
    //************

    string build_resource_path (const UString &a_dir, const UString &a_name) ;
    void add_stock_icon (const UString &a_stock_id,
                         const UString &icon_dir,
                         const UString &icon_name) ;
    void add_perspective_menu_entries () ;
    void add_perspective_toolbar_entries () ;
    void init_icon_factory () ;
    void init_actions () ;
    void init_toolbar () ;
    void init_body () ;
    void init_signals () ;
    void init_debugger_output_handlers () ;
    void append_source_editor (SourceEditor &a_sv,
                               const Glib::RefPtr<Gnome::Vfs::Uri> &a_uri) ;
    SourceEditor* get_current_source_editor () ;
    UString get_current_file_path () ;
    SourceEditor* get_source_editor_from_uri (const UString &a_uri) ;
    void bring_source_as_current (const UString &a_uri) ;
    int get_n_pages () ;
    void popup_source_view_contextual_menu (GdkEventButton *a_event) ;

public:

    DBGPerspective () ;
    virtual ~DBGPerspective () ;
    void get_info (Info &a_info) const ;
    void do_init () ;
    void do_init (IWorkbenchSafePtr &a_workbench) ;
    const UString& get_perspective_identifier () ;
    void get_toolbars (list<Gtk::Toolbar*> &a_tbs)  ;
    Gtk::Widget* get_body ()  ;
    void edit_workbench_menu () ;
    void open_file () ;
    bool open_file (const UString &a_uri,
                    int current_line=-1) ;
    void close_current_file () ;
    void close_file (const UString &a_uri) ;
    void execute_program () ;
    void execute_program (const UString &a_prog,
                          const UString &a_args,
                          const UString &a_cwd) ;
    void run () ;
    void step_over () ;
    void step_into () ;
    void step_out () ;
    void do_continue () ;
    void set_breakpoint () ;
    void set_breakpoint (const UString &a_file,
                         int a_line) ;
    void append_breakpoints (map<int, IDebugger::BreakPoint> &a_breaks) ;
    bool get_breakpoint_number (const UString &a_file_name,
                                int a_linenum,
                                int &a_break_num) ;
    bool delete_breakpoint () ;
    bool delete_breakpoint (int a_breakpoint_num) ;
    bool delete_breakpoint (const UString &a_file_uri,
                            int a_linenum) ;
    void append_visual_breakpoint (const UString &a_file_name,
                                   int a_linenum) ;
    void delete_visual_breakpoint (const UString &a_file_name, int a_linenum) ;
    void delete_visual_breakpoint (int a_breaknum) ;

    IDebuggerSafePtr& debugger () ;
    Gtk::TextView* get_command_view () ;
    Gtk::TextView* get_program_output_view () ;
    Gtk::TextView* get_error_view () ;
    void add_text_to_command_view (const UString &a_text) ;
    void add_text_to_program_output_view (const UString &a_text) ;
    void add_text_to_error_view (const UString &a_text) ;
    void set_where (const UString &a_uri, int line) ;
    Gtk::Widget* get_contextual_menu () ;
    sigc::signal<void, bool>& activated_signal () ;
    sigc::signal<void, bool>& debugger_ready_signal () ;
};//end class DBGPerspective
}//end namespace nemiver

#endif //__NMV_DBG_PERSPECTIVE_H__
