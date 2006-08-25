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
#ifndef __NEMIVER_I_DEBUGGER_H__
#define __NEMIVER_I_DEBUGGER_H__

#include <vector>
#include <string>
#include <map>
#include <list>
#include "nmv-api-macros.h"
#include "nmv-ustring.h"
#include "nmv-dynamic-module.h"
#include "nmv-safe-ptr-utils.h"

namespace nemiver {

using nemiver::common::SafePtr ;
using nemiver::common::DynamicModule ;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::UString ;
using std::vector ;
using std::string ;
using std::map ;
using std::list;

class IDebugger ;
typedef SafePtr<IDebugger, ObjectRef, ObjectUnref> IDebuggerSafePtr ;

/// \brief a debugger engine.
///
///It should abstract the real debugger used underneath, but
///it is modeled after the interfaces exposed by gdb.
///Please, read GDB/MI interface documentation for more.
class NEMIVER_API IDebugger : public DynamicModule {

    IDebugger (const IDebugger&) ;
    IDebugger& operator= (const IDebugger&) ;

protected:
    IDebugger () {};

public:

    /// \brief A container of the textual command sent to the debugger
    class Command {
        string m_value ;
        mutable long m_id ;

    public:

        Command ()  {clear ();}

        /// \param a_value a textual command to send to the debugger.
        Command (const string &a_value) :
            m_value (a_value)
        {}

        /// \name accesors

        /// @{

        const string& value () const {return m_value;}
        void value (const string &a_in) {m_value = a_in; m_id=-1;}

        long id () const {return m_id;}
        void id (long a_in) const {m_id = a_in;}

        /// @}

        void clear ()
        {
            m_id = -1 ;
            m_value = "" ;
        }

    };//end class Command

    /// \brief a breakpoint descriptor
    class BreakPoint {
        int m_number ;
        bool m_enabled ;
        UString m_address ;
        UString m_function ;
        UString m_file_name ;
        UString m_full_file_name ;
        int m_line ;

    public:

        BreakPoint () {clear ();}

        /// \name accessors

        /// @{
        int number () const {return m_number ;}
        void number (int a_in) {m_number = a_in;}

        bool enabled () const {return m_enabled;}
        void enabled (bool a_in) {m_enabled = a_in;}

        const UString& address () const {return m_address;}
        void address (const UString &a_in) {m_address = a_in;}

        const UString& function () const {return m_function;}
        void function (const UString &a_in) {m_function = a_in;}

        const UString& file_name () const {return m_file_name;}
        void file_name (const UString &a_in) {m_file_name = a_in;}

        const UString& full_file_name () const {return m_full_file_name;}
        void full_file_name (const UString &a_in) {m_full_file_name = a_in;}

        int line () const {return m_line;}
        void line (int a_in) {m_line = a_in;}

        bool is_pending ()
        {
            if (m_address == "<PENDING>") {
                return true ;
            }
            return false ;
        }
        /// @}

        /// \brief clear this instance of breakpoint
        void clear ()
        {
            m_number = 0 ;
            m_enabled = false ;
            m_address = "" ;
            m_function = "" ;
            m_file_name = "" ;
            m_full_file_name = "" ;
            m_line = 0 ;
        }
    };//end class BreakPoint

    /// \brief a function frame as seen by the debugger.
    class Frame {
        UString m_address ;
        UString m_function ;
        map<UString, UString> m_args ;
        //present if the target has debugging info
        UString m_file_name ;
        UString m_file_full_name ;
        int m_line ;
        //present if the target doesn't have debugging info
        UString m_library ;

    public:

        Frame () {clear ();}

        /// \name accessors

        /// @{
        const UString& address () const {return m_address;}
        void address (const UString &a_in) {m_address = a_in;}

        const UString& function () const {return m_function;}
        void function (const UString &a_in) {m_function = a_in;}

        const map<UString, UString>& args () const {return m_args;}
        map<UString, UString>& args () {return m_args;}

        const UString& file_name () const {return m_file_name;}
        void file_name (const UString &a_in) {m_file_name = a_in;}

        const UString& file_full_name () const {return m_file_full_name;}
        void file_full_name (const UString &a_in) {m_file_full_name = a_in;}

        int line () const {return m_line;}
        void line (int a_in) {m_line = a_in;}

        const UString& library () const {return m_library;}
        void library (const UString &a_library) {m_library = a_library;}

        /// @}

        /// \brief clears the current instance
        void clear ()
        {
            m_address = "" ;
            m_function = "" ;
            m_args.clear () ;
            m_file_name = "" ;
            m_file_full_name = "" ;
            m_line = 0 ;
        }
    };//end class Frame


    virtual ~IDebugger () {}

    /// \name events you can connect to.

    /// @{

    virtual sigc::signal<void>& engine_died_signal () const = 0 ;

    virtual sigc::signal<void, const UString&>&
                                     console_message_signal () const = 0 ;

    virtual sigc::signal<void, const UString&>&
                                 target_output_message_signal () const = 0 ;

    virtual sigc::signal<void, const UString&>& error_message_signal () const = 0 ;

    virtual sigc::signal<void, const UString&>& command_done_signal () const = 0;

    virtual sigc::signal<void, const IDebugger::BreakPoint&, int>&
                                         breakpoint_deleted_signal () const  = 0;

    virtual sigc::signal<void, const map<int, IDebugger::BreakPoint>&>&
                                             breakpoints_set_signal () const = 0;

    virtual sigc::signal<void, const UString&, bool, const IDebugger::Frame&>&
                                                     stopped_signal () const = 0;

    virtual sigc::signal<void, const list<IDebugger::Frame>& >&
                                                frames_listed_signal () const=0;

    virtual sigc::signal<void>& running_signal () const = 0;
    virtual sigc::signal<void>& program_finished_signal () const = 0;

    /// @}

    virtual map<UString, UString>& properties () = 0;

    virtual void set_event_loop_context
                        (const Glib::RefPtr<Glib::MainContext> &) = 0 ;

    virtual void run_loop_iterations (int a_nb_iters) = 0;

    virtual void execute_command (const Command &a_command) = 0;

    virtual bool queue_command (const Command &a_command) = 0;

    virtual bool busy () const = 0;

    virtual void load_program
                (const vector<UString> &a_argv,
                 const vector<UString> &a_source_search_dirs,
                 bool a_run_event_loops=false) = 0;

    virtual void attach_to_program (unsigned int a_pid) = 0;

    virtual void do_continue (bool a_run_event_loops=false) = 0;

    virtual void run (bool a_run_event_loops=false) = 0 ;

    virtual void step_in (bool a_run_event_loops=false) = 0;

    virtual void step_over (bool a_run_event_loops=false) = 0;

    virtual void step_out (bool a_run_event_loops=false) = 0;

    virtual void continue_to_position (const UString &a_path,
                                       gint a_line_num,
                                       bool a_run_event_loops=false ) = 0 ;
    virtual void set_breakpoint (const UString &a_path,
                                 gint a_line_num,
                                 bool a_run_event_loops=false) = 0 ;
    virtual void set_breakpoint (const UString &a_func_name,
                                 bool a_run_event_loops=false) = 0 ;

    virtual void list_breakpoints (bool a_run_event_loops=false) = 0 ;

    virtual const map<int, BreakPoint>& get_cached_breakpoints () = 0 ;

    virtual void enable_breakpoint (const UString &a_path,
                                    gint a_line_num,
                                    bool a_run_event_loops=false) = 0;
    virtual void disable_breakpoint (const UString &a_path,
                                     gint a_line_num,
                                     bool a_run_event_loops=false) = 0;
    virtual void delete_breakpoint (const UString &a_path,
                                    gint a_line_num,
                                    bool a_run_event_loops=false) = 0;
    virtual void delete_breakpoint (gint a_break_num,
                                    bool a_run_event_loops=false) = 0;
};//end IDebugger

}//end namespace nemiver

#endif //__NEMIVER_I_DEBUGGER_H__

