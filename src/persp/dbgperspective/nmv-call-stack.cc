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
#include <sstream>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include "nmv-call-stack.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"

namespace nemiver {

struct GObjectMMRef {

    void operator () (Glib::Object *a_obj)
    {
        if (a_obj) {
            a_obj->reference () ;
        }
    }
};//end GlibObjectRef

struct GObjectMMUnref {
    void operator () (Glib::Object *a_obj)
    {
        if (a_obj) {
            a_obj->unreference () ;
        }
    }
};//end GlibObjectRef

typedef SafePtr<Glib::Object, GObjectMMRef, GObjectMMUnref> GObjectMMSafePtr ;

struct CallStackCols : public Gtk::TreeModelColumnRecord {
    Gtk::TreeModelColumn<Glib::ustring> location ;
    Gtk::TreeModelColumn<Glib::ustring> function_name ;
    Gtk::TreeModelColumn<Glib::ustring> function_args;
    Gtk::TreeModelColumn<int> frame_index;

    enum Index {
        LOCATION=0,
        FUNCTION_NAME,
        FUNCTION_ARGS
    };

    CallStackCols ()
    {
        add (location) ;
        add (function_name) ;
        add (function_args) ;
        add (frame_index) ;
    }
};//end cols

static CallStackCols&
columns ()
{
    static CallStackCols s_cols ;
    return s_cols ;
}

struct CallStack::Priv {
    IDebuggerSafePtr debugger ;
    vector<IDebugger::Frame> frames ;
    map<int, list<IDebugger::VariableSafePtr> > m_params;
    Glib::RefPtr<Gtk::ListStore> store ;
    SafePtr<Gtk::TreeView> widget ;
    bool waiting_for_stack_args ;
    bool in_set_cur_frame_trans ;
    IDebugger::Frame cur_frame ;
    int cur_frame_index ;
    sigc::signal<void, int, const IDebugger::Frame&> frame_selected_signal ;
    sigc::connection on_selection_changed_connection ;

    Priv (IDebuggerSafePtr a_dbg) :
        debugger (a_dbg),
        waiting_for_stack_args (false),
        in_set_cur_frame_trans (false),
        cur_frame_index (-1)
    {
        connect_debugger_signals () ;
    }

    void on_debugger_stopped_signal (const UString &a_reason,
                                     bool a_has_frame,
                                     const IDebugger::Frame &a_frame,
                                     int a_thread_id)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        if (a_reason == "" || a_has_frame || a_frame.line () || a_thread_id) {}
        THROW_IF_FAIL (debugger) ;
        debugger->list_frames () ;

        NEMIVER_CATCH
    }

    void on_frames_listed_signal (const vector<IDebugger::Frame> &a_stack)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        THROW_IF_FAIL (debugger) ;
        waiting_for_stack_args = true ;

        //**************************************************************
        //set the frame list without frame parameters,
        //then, request IDebugger for frame parameters.
        //When the frame params arrives, we will set the
        //the frame list again, that time with the parameters.
        //Okay, this forces us to set the frame list twice,
        //but it is more robust because sometimes, the second
        //request to IDebugger fail (the one to get the parameters).
        //This way, we have at least the frame list withouht params.
        //**************************************************************
        map<int, list<IDebugger::VariableSafePtr> > frames_params ;
        set_frame_list (a_stack, frames_params) ;
        debugger->list_frames_arguments () ;

        NEMIVER_CATCH
    }

    void on_frames_params_listed_signal
            (const map<int, list<IDebugger::VariableSafePtr> > &a_frames_params)
    {
        LOG_D ("frames params listed", NMV_DEFAULT_DOMAIN) ;
        if (waiting_for_stack_args) {
            set_frame_list (frames, a_frames_params) ;
            waiting_for_stack_args = false ;
        } else {
            LOG_D ("not in the frame setting transaction", NMV_DEFAULT_DOMAIN) ;
        }
    }

    void on_command_done_signal (const UString &a_command)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        if (a_command == "") {}
        if (in_set_cur_frame_trans
            && !a_command.compare (0, 19, "-stack-select-frame")) {
            in_set_cur_frame_trans = false ;
            frame_selected_signal.emit (cur_frame_index, cur_frame) ;
            LOG_DD ("sent the frame selected signal") ;
        }
        NEMIVER_CATCH
    }

    void on_selection_changed_signal ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD ;

        NEMIVER_TRY

        Gtk::TreeView *tree_view = dynamic_cast<Gtk::TreeView*> (widget.get ());
        THROW_IF_FAIL (tree_view) ;
        Glib::RefPtr<Gtk::TreeSelection> selection = tree_view->get_selection () ;
        THROW_IF_FAIL (selection) ;
        list<Gtk::TreePath> selected_rows = selection->get_selected_rows () ;
        if (selected_rows.empty ()) {return;}

        Gtk::TreeModel::iterator row_iter =
                store->get_iter (selected_rows.front ()) ;
        cur_frame_index = (*row_iter)[columns ().frame_index] ;
        cur_frame = frames[cur_frame_index] ;
        THROW_IF_FAIL (cur_frame.level () >= 0) ;
        in_set_cur_frame_trans = true ;
        LOG_DD ("frame selected: '"<<  (int) cur_frame_index << "'") ;
        LOG_DD ("frame level: '" << (int) cur_frame.level () << "'") ;
        debugger->select_frame (cur_frame_index) ;

        NEMIVER_CATCH
    }

    void connect_debugger_signals ()
    {
        THROW_IF_FAIL (debugger) ;

        THROW_IF_FAIL (debugger) ;

        debugger->stopped_signal ().connect (sigc::mem_fun
                    (*this, &CallStack::Priv::on_debugger_stopped_signal)) ;
        debugger->frames_listed_signal ().connect (sigc::mem_fun
                    (*this, &CallStack::Priv::on_frames_listed_signal)) ;
        debugger->frames_params_listed_signal ().connect (sigc::mem_fun
                    (*this, &CallStack::Priv::on_frames_params_listed_signal)) ;
        debugger->command_done_signal ().connect (sigc::mem_fun
                    (*this, &CallStack::Priv::on_command_done_signal)) ;
    }

    Gtk::Widget* get_widget ()
    {
        if (!widget) {return 0;}
        return widget.get () ;
    }

    void build_widget ()
    {
        if (widget) {
            return ;
        }
        store = Gtk::ListStore::create (columns ());
        Gtk::TreeView *tree_view = new Gtk::TreeView (store) ;
        THROW_IF_FAIL (tree_view) ;
        widget.reset (tree_view) ;
        tree_view->append_column ("line", columns ().location) ;
        tree_view->append_column ("function", columns ().function_name) ;
        tree_view->append_column ("arguments", columns ().function_args) ;
        tree_view->set_headers_visible (true) ;
        tree_view->get_selection ()->set_mode (Gtk::SELECTION_SINGLE) ;

        on_selection_changed_connection =
            tree_view->get_selection ()->signal_changed ().connect
            (sigc::mem_fun (*this,
                            &CallStack::Priv::on_selection_changed_signal)) ;

        Gtk::TreeViewColumn* column =
                            tree_view->get_column (CallStackCols::FUNCTION_NAME) ;
        THROW_IF_FAIL (column) ;
        column->set_clickable (false) ;
        column->set_reorderable (false) ;

        THROW_IF_FAIL (column = tree_view->get_column (CallStackCols::LOCATION)) ;
        column->set_clickable (false) ;
        column->set_reorderable (false) ;

        THROW_IF_FAIL (column = tree_view->get_column
                                                (CallStackCols::FUNCTION_ARGS)) ;
        column->set_clickable (false) ;
        column->set_reorderable (false) ;
    }

    void set_frame_list
                (const vector<IDebugger::Frame> &a_frames,
                 const map<int, list<IDebugger::VariableSafePtr> >&a_params,
                 bool a_emit_signal=false)
    {
        THROW_IF_FAIL (get_widget ()) ;

        clear_frame_list () ;
        frames = a_frames ;

        // save the list of params around so that we can use it when converting
        // to a string later
        m_params = a_params;

        Gtk::TreeModel::iterator store_iter ;
        for (unsigned int i = 0 ; i < a_frames.size () ; ++i) {
            store_iter = store->append () ;
            (*store_iter)[columns ().function_name] = a_frames[i].function_name ();
            (*store_iter)[columns ().location] =
                            a_frames[i].file_name () + ":"
                            + UString::from_int (a_frames[i].line ()) ;
            (*store_iter)[columns ().frame_index] = i ;
            UString params_string = "(";
            map<int, list<IDebugger::VariableSafePtr> >::const_iterator iter ;
            list<IDebugger::VariableSafePtr>::const_iterator params_iter ;
            iter = a_params.find (i) ;
            if (iter != a_params.end ()) {
                LOG_D ("for frame "
                       << (int) i
                       << " NB params: "
                       << (int) iter->second.size (), NMV_DEFAULT_DOMAIN) ;

                params_iter = iter->second.begin () ;
                if (params_iter != iter->second.end ()) {
                    if (*params_iter) {
                        params_string += (*params_iter)->name () ;
                    }
                    ++params_iter ;
                }
                for ( ; params_iter != iter->second.end (); ++params_iter) {
                    if (!*params_iter) {continue;}
                     params_string += ", " + (*params_iter)->name () ;
                }
            }
            params_string += ")" ;
            (*store_iter)[columns ().function_args] = params_string ;
        }
        Gtk::TreeView *tree_view =
            dynamic_cast<Gtk::TreeView*> (widget.get ()) ;
        THROW_IF_FAIL (tree_view) ;

        if (!a_emit_signal) {
            on_selection_changed_connection.block () ;
        }
        tree_view->get_selection ()->select (Gtk::TreePath ("0")) ;
        if (!a_emit_signal) {
            on_selection_changed_connection.unblock () ;
        }
    }

    void clear_frame_list ()
    {
        THROW_IF_FAIL (store) ;
        store->clear () ;
    }
};//end struct CallStack::Priv

CallStack::CallStack (IDebuggerSafePtr &a_debugger)
{
    THROW_IF_FAIL (a_debugger) ;
    m_priv.reset (new Priv (a_debugger)) ;
}

CallStack::~CallStack ()
{
    LOG_D ("deleted", "destructor-domain") ;
}

bool
CallStack::is_empty ()
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frames.empty () ;
}


UString
CallStack::to_string()
{
    THROW_IF_FAIL (m_priv) ;
    int i = 0;
    std::ostringstream frame_stream;
    vector<IDebugger::Frame>::const_iterator frame_iter;
    map<int, list<IDebugger::VariableSafePtr> >::const_iterator params_iter;
    // convert list of stack frames to a string (maybe Frame should just
    // implement operator<< ?
    for (frame_iter=m_priv->frames.begin(), params_iter=m_priv->m_params.begin();
            frame_iter != m_priv->frames.end(); ++frame_iter)
    {
        frame_stream << "#" << UString::from_int(i++) << "  " <<
            frame_iter->function_name () << " (";
        // if the params map exists, add the function params to the stack trace
        if (params_iter != m_priv->m_params.end())
        {
            for (list<IDebugger::VariableSafePtr>::const_iterator it =
                    params_iter->second.begin();
                    it != params_iter->second.end(); ++it)
            {
                if (it != params_iter->second.begin())
                    frame_stream << ", ";
                frame_stream << (*it)->name() << "=" << (*it)->value();
            }
            // advance to the list of params for the next stack frame
            ++params_iter;
        }
        frame_stream << ") at " << frame_iter->file_name () << ":"
            << UString::from_int(frame_iter->line ()) << std::endl;
    }
    return frame_stream.str();
}

const vector<IDebugger::Frame>&
CallStack::frames () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frames ;
}

Gtk::Widget&
CallStack::widget () const
{
    THROW_IF_FAIL (m_priv) ;

    if (!m_priv->get_widget ()) {
        m_priv->build_widget () ;
        THROW_IF_FAIL (m_priv->widget) ;
    }
    return *m_priv->get_widget () ;
}

void
CallStack::update_stack ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->debugger) ;

    m_priv->debugger->list_frames () ;
}

sigc::signal<void, int, const IDebugger::Frame&>&
CallStack::frame_selected_signal () const
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->frame_selected_signal ;
}
}//end namespace nemiver


