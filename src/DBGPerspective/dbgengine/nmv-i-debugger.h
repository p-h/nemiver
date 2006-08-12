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

    /// \brief the output received from the debugger.
    ///
    /// This is tightly modeled after the interface exposed
    ///by gdb. See the documentation of GDB/MI for more.
    class Output {
    public:

        /// \brief debugger stream record.
        ///
        ///either the output stream of the
        ///debugger console (to be displayed by the CLI),
        ///or the target output stream or the debugger log stream.
        class StreamRecord {
            UString m_debugger_console ;
            UString m_target_output ;
            UString m_debugger_log ;

        public:

            StreamRecord () {clear ();}
            StreamRecord (const UString &a_debugger_console,
                          const UString &a_target_output,
                          const UString &a_debugger_log) :
                m_debugger_console (a_debugger_console),
                m_target_output (a_target_output),
                m_debugger_log (a_debugger_log)
            {}

            /// \name accessors

            /// @{
            const UString& debugger_console () const {return m_debugger_console;}
            void debugger_console (const UString &a_in)
            {
                m_debugger_console = a_in;
            }

            const UString& target_output () const {return m_target_output;}
            void target_output (const UString &a_in) {m_target_output = a_in;}

            const UString& debugger_log () const {return m_debugger_log;}
            void debugger_log (const UString &a_in) {m_debugger_log = a_in;}
            /// @}

            void clear ()
            {
                m_debugger_console = "" ;
                m_target_output = "" ;
                m_debugger_log = "" ;
            }
        };//end class StreamRecord


        /// \brief the out of band record we got from GDB.
        ///
        ///Out of band record is either
        ///a set of messages sent by gdb
        ///to tell us about the reason why the target has stopped,
        ///or, a stream record.
        class OutOfBandRecord {
        public:

            enum StopReason {
                UNDEFINED=0,
                BREAKPOINT_HIT,
                WATCHPOINT_TRIGGER,
                READ_WATCHPOINT_TRIGGER,
                ACCESS_WATCHPOINT_TRIGGER,
                FUNCTION_FINISHED,
                LOCATION_REACHED,
                WATCHPOINT_SCOPE,
                END_STEPPING_RANGE,
                EXITED_SIGNALLED,
                EXITED,
                EXITED_NORMALLY,
                SIGNAL_RECEIVED
            };//end enum StopReason

        private:
            bool m_has_stream_record;
            StreamRecord m_stream_record ;
            bool m_is_stopped ;
            StopReason m_stop_reason ;
            bool m_has_frame ;
            Frame m_frame ;
            long m_breakpoint_number ;
            long m_thread_id ;

        public:

            OutOfBandRecord () {clear ();}

            /// \name


            /// @{
            bool has_stream_record () const {return m_has_stream_record;}
            void has_stream_record (bool a_in) {m_has_stream_record = a_in;}

            const StreamRecord& stream_record () const {return m_stream_record;}
            StreamRecord& stream_record () {return m_stream_record;}
            void stream_record (const StreamRecord &a_in) {m_stream_record=a_in;}

            bool is_stopped () const {return m_is_stopped;}
            void is_stopped (bool a_in) {m_is_stopped = a_in;}

            StopReason stop_reason () const {return m_stop_reason ;}
            void stop_reason (StopReason a_in) {m_stop_reason = a_in;}

            bool has_frame () const {return m_has_frame;}
            void has_frame (bool a_in) {m_has_frame = a_in;}

            const Frame& frame () const {return m_frame;}
            Frame& frame () {return m_frame;}
            void frame (const Frame &a_in) {m_frame = a_in;}

            long breakpoint_number () const {return m_breakpoint_number ;}
            void breakpoint_number (long a_in) {m_breakpoint_number = a_in;}

            long thread_id () const {return m_thread_id;}
            void thread_id (long a_in) {m_thread_id = a_in;}
            /// @}

            void clear ()
            {
                m_has_stream_record = false ;
                m_stream_record.clear () ;
                m_is_stopped = false ;
                m_stop_reason = UNDEFINED ;
                m_has_frame = false ;
                m_frame.clear () ;
                m_breakpoint_number = 0 ;
                m_thread_id = 0 ;
            }
        };//end class OutOfBandRecord

        /// \debugger result record
        ///
        /// this is a notification of gdb that about the last operation
        /// he was asked to perform. This basically can be either
        /// "running" --> the operation was successfully started, the target
        /// is running.
        /// "done" --> the synchronous operation was successful. In this case,
        /// gdb also returns the result of the operation.
        /// "error" the operation failed. In this case, gdb returns the
        /// corresponding error message.
        class ResultRecord {
        public:
            enum Kind {
               UNDEFINED=0,
               DONE,
               RUNNING,
               CONNECTED,
               ERROR,
               EXIT
            };//end enum Kind

        private:
            Kind m_kind ;
            map<int, BreakPoint> m_breakpoints ;
            map<UString, UString> m_attrs ;

        public:
            ResultRecord () {clear () ;}

            /// \name accessors

            /// @{
            Kind kind () const {return m_kind;}
            void kind (Kind a_in) {m_kind = a_in;}

            const map<int, BreakPoint>& breakpoints () const
            {
                return m_breakpoints;
            }
            map<int, BreakPoint>& breakpoints () {return m_breakpoints;}

            map<UString, UString>& attrs () {return m_attrs;}
            const map<UString, UString>& attrs () const {return m_attrs;}
            /// @}

            void clear ()
            {
                m_kind = UNDEFINED ;
                m_breakpoints.clear () ;
                m_attrs.clear () ;
            }
        };//end class ResultRecord

    private:
        UString m_value ;
        bool m_parsing_succeeded ;
        bool m_has_out_of_band_record ;
        list<OutOfBandRecord> m_out_of_band_records ;
        bool m_has_result_record ;
        ResultRecord m_result_record ;

    public:

        Output () {clear ();}

        Output (const UString &a_value) {clear ();}

        /// \name accessors

        /// @{
        const UString& raw_value () const {return m_value;}
        void raw_value (const UString &a_in) {m_value = a_in;}

        bool parsing_succeeded () const {return m_parsing_succeeded;}
        void parsing_succeeded (bool a_in) {m_parsing_succeeded = a_in;}

        bool has_out_of_band_record () const {return m_has_out_of_band_record;}
        void has_out_of_band_record (bool a_in) {m_has_out_of_band_record = a_in;}

        const list<OutOfBandRecord>& out_of_band_records () const
        {
            return m_out_of_band_records;
        }

        list<OutOfBandRecord>& out_of_band_records ()
        {
            return m_out_of_band_records;
        }

        bool has_result_record () const {return m_has_result_record;}
        void has_result_record (bool a_in) {m_has_result_record = a_in;}

        const ResultRecord& result_record () const {return m_result_record;}
        ResultRecord& result_record () {return m_result_record;}
        void result_record (const ResultRecord &a_in) {m_result_record = a_in;}
        /// @}

        void clear ()
        {
            m_value = "" ;
            m_parsing_succeeded = false ;
            m_has_out_of_band_record = false ;
            m_out_of_band_records.clear () ;
            m_has_result_record = false ;
            m_result_record.clear () ;
        }
    };//end class Output

    /// A container of the Command sent to the debugger
    /// and the output it sent back.
    class CommandAndOutput {
        bool m_has_command ;
        Command m_command ;
        Output m_output ;

    public:

        CommandAndOutput (const Command &a_command,
                          const Output &a_output) :
            m_has_command (true),
            m_command (a_command),
            m_output (a_output)
        {
        }

        CommandAndOutput () :
            m_has_command (false)
        {}

        /// \name accessors

        /// @{

        bool has_command () const {return m_has_command ;}
        void has_command (bool a_in) {m_has_command = a_in;}

        const Command& command () const {return m_command;}
        Command& command () {return m_command;}
        void command (const Command &a_in) {m_command = a_in; has_command (true);}

        const Output& output () const {return m_output;}
        Output& output () {return m_output;}
        void output (const Output &a_in) {m_output = a_in;}
        /// @}
    };//end CommandAndOutput

    virtual ~IDebugger () {}

    /// \name events you can connect to.

    /// @{

    virtual sigc::signal<void, Output&>& pty_signal () const = 0;
    virtual sigc::signal<void, Output&>& stderr_signal () const = 0;
    virtual sigc::signal<void, CommandAndOutput&>& stdout_signal () const = 0;
    virtual sigc::signal<void>& engine_died_signal () const = 0 ;

    virtual sigc::signal<void,
                         const UString&,
                         IDebugger::CommandAndOutput&>&
                             console_message_signal () const = 0 ;

    virtual sigc::signal<void,
                         const UString&,
                         IDebugger::CommandAndOutput&>&
                             target_output_message_signal () const = 0 ;

    virtual sigc::signal<void,
                         const UString&,
                         IDebugger::CommandAndOutput&>&
                             error_message_signal () const = 0 ;

    virtual sigc::signal<void,
                         const UString&,
                         IDebugger::CommandAndOutput&>&
                             command_done_signal () const = 0;

    virtual sigc::signal<void,
                         const IDebugger::BreakPoint&,
                         int,
                         IDebugger::CommandAndOutput&>&
                             breakpoint_deleted_signal () const  = 0;

    virtual sigc::signal<void,
                         const map<int, IDebugger::BreakPoint>&,
                         IDebugger::CommandAndOutput&>&
                             breakpoints_set_signal () const = 0;

    virtual sigc::signal<void,
                         const IDebugger::Frame&,
                         IDebugger::CommandAndOutput&>&
                             stopped_signal () const = 0;

    virtual sigc::signal<void,
                         IDebugger::CommandAndOutput&>&
                                         running_signal () const = 0;

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

