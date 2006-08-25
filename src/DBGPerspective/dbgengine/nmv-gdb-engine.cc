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
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pty.h>
#include <termios.h>
#include <boost/variant.hpp>
#include <algorithm>
#include <memory>
#include <fstream>
#include <iostream>
#include "nmv-i-debugger.h"
#include "nmv-env.h"
#include "nmv-exception.h"
#include "nmv-sequence.h"

#define LOG_PARSING_ERROR(a_buf, a_from) \
{ \
Glib::ustring str_01 (a_buf, (a_from), a_buf.size () - (a_from)) ;\
LOG_ERROR ("parsing failed for buf: >>>" \
           << a_buf << "<<<" \
           << " cur index was: " << (int)(a_from)) ;\
}

#define CHECK_END(a_input, a_current, a_end) \
if (a_current >= (a_end)) {LOG_PARSING_ERROR (a_input, (a_current)); return false ;}

#define SKIP_WS(a_input, a_from, a_to) \
while (isspace (a_input[a_from])) {CHECK_END (a_input, a_from, end)} a_to = a_from ;

using namespace std ;
using namespace nemiver::common ;

namespace nemiver {

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
        IDebugger::Frame m_frame ;
        long m_breakpoint_number ;
        long m_thread_id ;

    public:

        OutOfBandRecord () {clear ();}

        UString stop_reason_to_string (StopReason a_reason) const
        {
            UString result ("undefined") ;

            switch (a_reason) {
                case UNDEFINED:
                    return "undefined" ;
                    break ;
                case BREAKPOINT_HIT:
                    return "breakpoint-hit" ;
                    break ;
                case WATCHPOINT_TRIGGER:
                    return "watchpoint-trigger" ;
                    break ;
                case READ_WATCHPOINT_TRIGGER:
                    return "read-watchpoint-trigger" ;
                    break ;
                case ACCESS_WATCHPOINT_TRIGGER:
                    return "access-watchpoint-trigger" ;
                    break ;
                case FUNCTION_FINISHED:
                    return "function-finished" ;
                    break ;
                case LOCATION_REACHED:
                    return "location-reacherd" ;
                    break ;
                case WATCHPOINT_SCOPE:
                    return "watchpoint-scope" ;
                    break ;
                case END_STEPPING_RANGE:
                    return "end-stepping-range" ;
                    break ;
                case EXITED_SIGNALLED:
                    return "exited-signalled" ;
                    break ;
                case EXITED:
                    return "exited" ;
                    break ;
                case EXITED_NORMALLY:
                    return "exited-normally" ;
                    break ;
                case SIGNAL_RECEIVED:
                    return "signal-received" ;
                    break ;
            }
            return result ;
        }

        /// \accessors


        /// @{
        bool has_stream_record () const {return m_has_stream_record;}
        void has_stream_record (bool a_in) {m_has_stream_record = a_in;}

        const StreamRecord& stream_record () const {return m_stream_record;}
        StreamRecord& stream_record () {return m_stream_record;}
        void stream_record (const StreamRecord &a_in) {m_stream_record=a_in;}

        bool is_stopped () const {return m_is_stopped;}
        void is_stopped (bool a_in) {m_is_stopped = a_in;}

        StopReason stop_reason () const {return m_stop_reason ;}
        UString stop_reason_as_str () const
        {
            return stop_reason_to_string (m_stop_reason) ;
        }
        void stop_reason (StopReason a_in) {m_stop_reason = a_in;}

        bool has_frame () const {return m_has_frame;}
        void has_frame (bool a_in) {m_has_frame = a_in;}

        const IDebugger::Frame& frame () const {return m_frame;}
        IDebugger::Frame& frame () {return m_frame;}
        void frame (const IDebugger::Frame &a_in) {m_frame = a_in;}

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
        map<int, IDebugger::BreakPoint> m_breakpoints ;
        map<UString, UString> m_attrs ;

    public:
        ResultRecord () {clear () ;}

        /// \name accessors

        /// @{
        Kind kind () const {return m_kind;}
        void kind (Kind a_in) {m_kind = a_in;}

        const map<int, IDebugger::BreakPoint>& breakpoints () const
        {
            return m_breakpoints;
        }
        map<int, IDebugger::BreakPoint>& breakpoints () {return m_breakpoints;}

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

/// A container of the IDebugger::Command sent to the debugger
/// and the output it sent back.
class CommandAndOutput {
    bool m_has_command ;
    IDebugger::Command m_command ;
    Output m_output ;

public:

    CommandAndOutput (const IDebugger::Command &a_command,
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

    const IDebugger::Command& command () const {return m_command;}
    IDebugger::Command& command () {return m_command;}
    void command (const IDebugger::Command &a_in) {m_command = a_in; has_command (true);}

    const Output& output () const {return m_output;}
    Output& output () {return m_output;}
    void output (const Output &a_in) {m_output = a_in;}
    /// @}
};//end CommandAndOutput

class Tuple ;
class Result ;
class Value ;
class ResultValueList ;
typedef SafePtr<Result, ObjectRef, ObjectUnref> ResultSafePtr ;
typedef SafePtr<Tuple, ObjectRef, ObjectUnref> TupleSafePtr ;
typedef SafePtr<Value, ObjectRef, ObjectUnref> ValueSafePtr;
typedef SafePtr<ResultValueList, ObjectRef, ObjectUnref> ResultValueListSafePtr ;

class Tuple : public Object {
    Tuple (const Tuple&) ;
    Tuple& operator= (const Tuple&) ;

    list<ResultSafePtr> m_content ;

public:

    Tuple () {}
    virtual ~Tuple () {}
    list<ResultSafePtr>& content () {return m_content;}
    void content (const list<ResultSafePtr> &a_in) {m_content = a_in;}
    void append (const ResultSafePtr &a_result) {m_content .push_back (a_result);}
    void clear () {m_content.clear ();}
};//end class Tuple

class Value : public Object {
    Value (const Value&) ;
    Value& operator= (const Value&) ;
    boost::variant<UString, ResultValueListSafePtr, TupleSafePtr> m_content ;
    friend class Result ;

    Value () {}
public:
    enum ValueTypes {
        STRING_TYPE=0,
        RESULT_OR_VALUE_LIST_TYPE,
        TUPLE_TYPE
    };

    Value (const UString &a_str) {m_content = a_str ;}

    Value (const ResultValueListSafePtr &a_list)
    {
        m_content = a_list ;
    }

    Value (const TupleSafePtr &a_tuple)
    {
        m_content = a_tuple ;
    }

    void append (const UString &a_str)
    {
        THROW_IF_FAIL (boost::get<UString> (&m_content)) ;
        boost::get<UString> (m_content) ;
    }

    const boost::variant<UString,ResultValueListSafePtr,TupleSafePtr>&
    content () const
    {
        return m_content;
    }
    void content
        (const boost::variant<UString,ResultValueListSafePtr,TupleSafePtr> &a_in)
    {
        m_content = a_in;
    }
};//end class value

class Result : public Object {
    Result (const Result&) ;
    Result& operator= (const Result&) ;

    UString m_variable ;
    ValueSafePtr m_value ;

public:

    Result () {}
    Result (const UString &a_variable,
            const ValueSafePtr &a_value) :
        m_variable (a_variable),
        m_value (a_value)
    {}
    virtual ~Result () {}
    const UString& variable () const {return m_variable;}
    void variable (const UString& a_in) {m_variable = a_in;}
    const ValueSafePtr& value () const {return m_value;}
    void value (const ValueSafePtr &a_in) {m_value = a_in;}
};//end class Result

class ResultValueList : public Object {
    ResultValueList (const ResultValueList &) ;
    ResultValueList& operator= (const ResultValueList &) ;

    boost::variant<list<ResultSafePtr>, list<ValueSafePtr> > m_content ;

public:
    enum ContentType {
        RESULT_TYPE=0,
        VALUE_TYPE
    };

    ResultValueList () {}

    ResultValueList (const TupleSafePtr &a_tuple)
    {
        ValueSafePtr value (new Value (a_tuple)) ;
        list<ValueSafePtr> value_list ; value_list.push_back (value) ;
        m_content = value_list ;
    }

    ResultValueList (const UString &a_str)
    {
        ValueSafePtr value (new Value (a_str)) ;
        list<ValueSafePtr> list ;
        list.push_back (value) ;
        m_content = list ;
    }

    ResultValueList (const ResultSafePtr &a_result)
    {
        list<ResultSafePtr> list ;
        list.push_back (a_result) ;
        m_content = list ;
    }

    ResultValueList (const ValueSafePtr &a_value)
    {
        list<ValueSafePtr> list ;
        list.push_back (a_value) ;
        m_content = list ;
    }

    virtual ~ResultValueList () {}
    ContentType content_type () const {return (ContentType) m_content.which ();}
    void append (const ResultSafePtr &a_in)
    {
        THROW_IF_FAIL (boost::get<list<ResultSafePtr> > (&m_content)) ;
        boost::get<list<ResultSafePtr> > (&m_content)->push_back (a_in);
    }
    void append (const ValueSafePtr &a_in)
    {
        THROW_IF_FAIL (boost::get<list<ValueSafePtr> > (&m_content)) ;
        boost::get<list<ValueSafePtr> > (&m_content)->push_back (a_in);
    }
    const list<ResultSafePtr> get_result_content () const
    {
        THROW_IF_FAIL (content_type () == RESULT_TYPE) ;
        return *boost::get<list<ResultSafePtr> > (&m_content) ;
    }

    const list<ValueSafePtr> get_value_content () const
    {
        THROW_IF_FAIL (content_type () == VALUE_TYPE) ;
        return *boost::get<list<ValueSafePtr> > (&m_content) ;
    }
};//end class ResultValueList

class GDBEngine : public IDebugger {

    GDBEngine (const GDBEngine &) ;
    GDBEngine& operator= (const GDBEngine &) ;

    struct Priv ;
    SafePtr<Priv> m_priv ;
    friend struct Priv ;

public:

    GDBEngine () ;
    virtual ~GDBEngine () ;
    void get_info (Info &a_info) const ;
    void do_init () ;

    //*************
    //<signals>
    //*************
    sigc::signal<void, Output&>& pty_signal () const ;

    sigc::signal<void, CommandAndOutput&>& stdout_signal () const ;

    sigc::signal<void, Output&>& stderr_signal () const ;

    sigc::signal<void>& engine_died_signal () const ;

    sigc::signal<void, const UString&>& console_message_signal () const ;

    sigc::signal<void, const UString&>& target_output_message_signal () const ;

    sigc::signal<void, const UString&>& error_message_signal () const ;

    sigc::signal<void, const UString&>& command_done_signal () const ;

    sigc::signal<void, const map<int, IDebugger::BreakPoint>& >&
                                                breakpoints_set_signal () const ;

    sigc::signal<void, const IDebugger::BreakPoint&, int>&
                                            breakpoint_deleted_signal () const ;


    sigc::signal<void, const UString&, bool, const IDebugger::Frame&>&
                                                        stopped_signal () const ;

    sigc::signal<void, const list<IDebugger::Frame>& >&
                                                frames_listed_signal () const ;

    sigc::signal<void>& running_signal () const ;

    sigc::signal<void>& program_finished_signal () const ;
    //*************
    //</signals>
    //*************

    //***************
    //<signal handlers>
    //***************

    void on_debugger_stdout_signal (CommandAndOutput &a_cao) ;
    //***************
    //</signal handlers>
    //***************

    void init () ;
    map<UString, UString>& properties () ;
    void set_event_loop_context (const Glib::RefPtr<Glib::MainContext> &) ;
    void run_loop_iterations (int a_nb_iters) ;
    void execute_command (const IDebugger::Command &a_command) ;
    bool queue_command (const IDebugger::Command &a_command) ;
    bool busy () const ;
    void load_program (const vector<UString> &a_argv,
            const vector<UString> &a_source_search_dirs,
            bool a_run_event_loops) ;
    void attach_to_program (unsigned int a_pid) ;
    void init_output_handlers () ;
    void append_breakpoints_to_cache (const map<int, IDebugger::BreakPoint>&) ;
    void do_continue (bool a_run_event_loops) ;
    void run (bool a_run_event_loops)  ;
    void step_in (bool a_run_event_loops) ;
    void step_out (bool a_run_event_loops) ;
    void step_over (bool a_run_event_loops) ;
    void continue_to_position (const UString &a_path,
            gint a_line_num,
            bool a_run_event_loops)  ;
    void set_breakpoint (const UString &a_path,
            gint a_line_num,
            bool a_run_event_loops)  ;
    void list_breakpoints (bool a_run_event_loops) ;
    map<int, IDebugger::BreakPoint>& get_cached_breakpoints () ;
    void set_breakpoint (const UString &a_func_name,
            bool a_run_event_loops)  ;
    void enable_breakpoint (const UString &a_path,
            gint a_line_num,
            bool a_run_event_loops) ;
    void disable_breakpoint (const UString &a_path,
            gint a_line_num,
            bool a_run_event_loops) ;
    void delete_breakpoint (const UString &a_path,
            gint a_line_num,
            bool a_run_event_loops) ;
    void delete_breakpoint (gint a_break_num,
            bool a_run_event_loops) ;

};//end class GDBEngine

struct OutputHandler : Object {

    //a method supposed to return
    //true if the current handler knows
    //how to handle a given debugger output
    virtual bool can_handle (CommandAndOutput &){return false;}

    //a method supposed to return
    //true if the current handler knows
    //how to handle a given debugger output
    virtual void do_handle (CommandAndOutput &) {}
};//end struct OutputHandler
typedef SafePtr<OutputHandler, ObjectRef, ObjectUnref> OutputHandlerSafePtr ;

//*************************
//<GDBEngine::Priv struct>
//*************************

struct GDBEngine::Priv {
    //***********************
    //<GDBEngine attributes>
    //************************
    map<UString, UString> properties ;
    UString cwd ;
    vector<UString> argv ;
    vector<UString> source_search_dirs ;
    Glib::Pid gdb_pid ;
    int gdb_stdout_fd ;
    int gdb_stderr_fd ;
    int gdb_master_pty_fd ;
    Glib::RefPtr<Glib::IOChannel> gdb_stdout_channel;
    Glib::RefPtr<Glib::IOChannel> gdb_stderr_channel ;
    Glib::RefPtr<Glib::IOChannel> gdb_master_pty_channel;
    UString gdb_stdout_buffer ;
    UString gdb_master_pty_buffer ;
    UString gdb_stderr_buffer;
    list<IDebugger::Command> command_queue ;
    list<IDebugger::Command> queued_commands ;
    list<IDebugger::Command> started_commands ;
    map<int, IDebugger::BreakPoint> cached_breakpoints;
    Sequence command_sequence ;
    enum InBufferStatus {
        FILLING,
        FILLED
    };
    InBufferStatus gdb_master_pty_buffer_status ;
    InBufferStatus gdb_stdout_buffer_status ;
    InBufferStatus error_buffer_status ;
    Glib::RefPtr<Glib::MainContext> loop_context ;

    list<OutputHandlerSafePtr> output_handlers ;
    sigc::signal<void> gdb_died_signal;
    sigc::signal<void, const UString& > gdb_master_pty_signal;
    sigc::signal<void, const UString& > gdb_stdout_signal;
    sigc::signal<void, const UString& > gdb_stderr_signal;

    mutable sigc::signal<void, Output&> pty_signal  ;

    mutable sigc::signal<void, CommandAndOutput&> stdout_signal  ;

    mutable sigc::signal<void, Output&> stderr_signal  ;

    mutable sigc::signal<void, const UString&> console_message_signal ;
    mutable sigc::signal<void, const UString&> target_output_message_signal;

    mutable sigc::signal<void, const UString&> error_message_signal ;

    mutable sigc::signal<void, const UString&> command_done_signal ;

    mutable sigc::signal<void, const map<int, IDebugger::BreakPoint>&>
                                                    breakpoints_set_signal;

    mutable sigc::signal<void, const IDebugger::BreakPoint&, int>
                                                breakpoint_deleted_signal ;

    mutable sigc::signal<void, const UString&,
                         bool, const IDebugger::Frame&> stopped_signal ;

    mutable sigc::signal<void, const list<IDebugger::Frame>& >
                                                    frames_listed_signal ;

    mutable sigc::signal<void> running_signal ;

    mutable sigc::signal<void> program_finished_signal ;

    //***********************
    //</GDBEngine attributes>
    //************************

    void attach_channel_to_loop_context_as_source
    (Glib::IOCondition a_cond,
     const sigc::slot<bool, Glib::IOCondition> &a_slot,
     const Glib::RefPtr<Glib::IOChannel> &a_chan,
     const Glib::RefPtr<Glib::MainContext>&a_ctxt)
    {
        THROW_IF_FAIL (a_chan) ;
        THROW_IF_FAIL (a_ctxt) ;

        Glib::RefPtr<Glib::IOSource> io_source =
        Glib::IOSource::create (a_chan, a_cond) ;
        io_source->connect (a_slot) ;
        io_source->attach (a_ctxt) ;
    }

    void set_event_loop_context
    (const Glib::RefPtr<Glib::MainContext> &a_ctxt)
    {
        loop_context = a_ctxt ;
    }

    void run_loop_iterations (int a_nb_iters)
    {
        if (!a_nb_iters) return ;

        if (a_nb_iters < 0) {
            while (get_event_loop_context ()->pending ()) {
                get_event_loop_context ()->iteration (false) ;
            }
        } else {
            while (a_nb_iters--) {
                get_event_loop_context ()->iteration (false) ;
            }
        }
    }

    Glib::RefPtr<Glib::MainContext>& get_event_loop_context ()
    {
        if (!loop_context) {
            loop_context = Glib::MainContext::get_default ();
        }
        THROW_IF_FAIL (loop_context) ;
        return loop_context ;
    }

    void on_gdb_pty_signal (const UString &a_buf)
    {
        Output result (a_buf) ;
        pty_signal.emit (result) ;
    }

    void on_gdb_stderr_signal (const UString &a_buf)
    {
        Output result (a_buf) ;
        stderr_signal.emit (result) ;
    }

    void on_gdb_stdout_signal (const UString &a_buf)
    {
        if (properties["log-debugger-output"] == "yes") {
            LOG ("<debuggeroutput>\n" << a_buf << "\n</debuggeroutput>") ;
        }

        Output output (a_buf);

        UString::size_type from (0), to (0), end (a_buf.size ()) ;
        for (; from < end ;) {
            if (!parse_output_record (a_buf, from, to, output)) {
                LOG_ERROR ("output record parsing failed: "
                        << a_buf.substr (from, end - from)
                        << "\npart of buf: " << a_buf
                        << "\nfrom: " << (int) from
                        << "\nto: " << (int) to << "\n"
                        << "\nstrlen: " << (int) a_buf.size ());
                break ;
            }

            //parsing GDB/MI output succeeded.
            //Check if the output contains the result to a command issued by
            //the user. If yes, build the IDebugger::CommandAndResult, update the
            //command queue and notify the user that the command it issued
            //has a result.
            //

            output.parsing_succeeded (true) ;
            UString output_value ;
            output_value.assign (a_buf, from, to - from +1) ;
            output.raw_value (output_value) ;
            CommandAndOutput command_and_output ;
            if (output.has_result_record ()) {
                if (!started_commands.empty ()) {
                    command_and_output.command (*started_commands.begin ()) ;
                }
            }
            command_and_output.output (output) ;
            stdout_signal.emit (command_and_output) ;
            from = to ;
            while (to < end && isspace (a_buf[from])) {++from;}
            if (output.has_result_record ()) {
                if (!started_commands.empty ()) {
                    started_commands.erase (started_commands.begin ()) ;
                }
                if (started_commands.empty ()
                        && !queued_commands.empty ()) {
                    issue_command (*queued_commands.begin ()) ;
                    queued_commands.erase (queued_commands.begin ()) ;
                }
            }
        }
    }

    Priv () :
        cwd ("."), gdb_pid (0), gdb_stdout_fd (0),
        gdb_stderr_fd (0), gdb_master_pty_fd (0),
        gdb_master_pty_buffer_status (FILLED),
        error_buffer_status (FILLED)
    {
        gdb_stdout_signal.connect (sigc::mem_fun
                (*this, &Priv::on_gdb_stdout_signal)) ;
        gdb_master_pty_signal.connect (sigc::mem_fun
                (*this, &Priv::on_gdb_pty_signal)) ;
        gdb_stderr_signal.connect (sigc::mem_fun
                (*this, &Priv::on_gdb_stderr_signal)) ;
    }

    void free_resources ()
    {
        if (gdb_pid) {
            g_spawn_close_pid (gdb_pid) ;
            gdb_pid = 0 ;
        }
        if (gdb_stdout_channel) {
            gdb_stdout_channel->close () ;
            gdb_stdout_channel.clear () ;
        }
        if (gdb_master_pty_channel) {
            gdb_master_pty_channel->close () ;
            gdb_master_pty_channel.clear () ;
        }
        if (gdb_stderr_channel) {
            gdb_stderr_channel->close () ;
            gdb_stderr_channel.clear () ;
        }
    }

    void on_child_died_signal (Glib::Pid a_pid,
            int a_priority)
    {
        gdb_died_signal.emit () ;
        free_resources () ;
    }

    bool is_gdb_running ()
    {
        if (gdb_pid) {return true ;}
        return false ;
    }

    void kill_gdb ()
    {
        if (is_gdb_running ()) {
            kill (gdb_pid, SIGKILL) ;
        }
        free_resources() ;
    }

    bool launch_program (const vector<UString> &a_args,
            int &a_pid,
            int &a_master_pty_fd,
            int &a_stdout_fd,
            int &a_stderr_fd)
    {
        for (vector<UString>::const_iterator it = a_args.begin () ;
                it != a_args.end ();
                ++it) {
            cout << *it << " " ;
        }
        cout << "\n" ;

        RETURN_VAL_IF_FAIL (!a_args.empty (), false) ;

        enum ReadWritePipe {
            READ_PIPE=0,
            WRITE_PIPE=1
        };

        int stdout_pipes[2] = {0};
        int stderr_pipes[2]= {0} ;
        //int stdin_pipes[2]= {0} ;
        int master_pty_fd (0) ;

        RETURN_VAL_IF_FAIL (pipe (stdout_pipes) == 0, false) ;
        //TODO: this line leaks the preceding pipes if it fails
        RETURN_VAL_IF_FAIL (pipe (stderr_pipes) == 0, false) ;
        //RETURN_VAL_IF_FAIL (pipe (stdin_pipes) == 0, false) ;

        int pid  = forkpty (&master_pty_fd, NULL, NULL, NULL);
        //int pid  = fork () ;
        if (pid == 0) {
            //in the child process
            //*******************************************
            //wire stderr to stderr_pipes[WRITER] pipe
            //******************************************
            close (2) ;
            dup (stderr_pipes[WRITE_PIPE]) ;

            //*******************************************
            //wire stdout to stdout_pipes[WRITER] pipe
            //******************************************
            close (1) ;
            dup (stdout_pipes[WRITE_PIPE]) ;

            //close (0) ;
            //log << "at 5" << endl;
            //dup (stdin_pipes[READ_PIPE]) ;
            //log << "at 6" << endl;

            //*****************************
            //close the unnecessary pipes
            //****************************
            close (stderr_pipes[READ_PIPE]) ;
            close (stdout_pipes[READ_PIPE]) ;
            //close (stdin_pipes[WRITE_PIPE]) ;

            //**********************************************
            //configure the pipes to be to have no buffering
            //*********************************************
            int state_flag (0) ;
            if ((state_flag = fcntl (stdout_pipes[WRITE_PIPE],
                            F_GETFL)) != -1) {
                fcntl (stdout_pipes[WRITE_PIPE],
                        F_SETFL,
                        O_SYNC | state_flag) ;
            }
            if ((state_flag = fcntl (stderr_pipes[WRITE_PIPE],
                            F_GETFL)) != -1) {
                fcntl (stderr_pipes[WRITE_PIPE],
                        F_SETFL,
                        O_SYNC | state_flag) ;
            }

            auto_ptr<char *> args;
            args.reset (new char* [a_args.size () + 1]) ;
            memset (args.get (), 0,
                    sizeof (char*) * (a_args.size () + 1)) ;
            if (!args.get ()) {
                exit (-1) ;
            }
            vector<UString>::const_iterator iter ;
            unsigned int i (0) ;
            for (i=0 ;
                    i < a_args.size () ;
                    ++i) {
                args.get ()[i] =
                const_cast<char*> (a_args[i].c_str ());
            }

            execvp (args.get ()[0], args.get ()) ;
            exit (-1) ;
        } else if (pid > 0) {
            //in the parent process

            //**************************
            //close the useless pipes
            //*************************
            close (stderr_pipes[WRITE_PIPE]) ;
            close (stdout_pipes[WRITE_PIPE]) ;
            //close (stdin_pipes[READ_PIPE]) ;

            //****************************************
            //configure the pipes to be non blocking
            //****************************************
            int state_flag (0) ;
            if ((state_flag = fcntl (stdout_pipes[READ_PIPE],
                            F_GETFL)) != -1) {
                fcntl (stdout_pipes[READ_PIPE], F_SETFL, O_NONBLOCK|state_flag);
            }
            if ((state_flag = fcntl (stderr_pipes[READ_PIPE],
                            F_GETFL)) != -1) {
                fcntl (stderr_pipes[READ_PIPE], F_SETFL, O_NONBLOCK|state_flag);
            }

            if ((state_flag = fcntl (master_pty_fd, F_GETFL)) != -1) {
                fcntl (master_pty_fd, F_SETFL, O_NONBLOCK|state_flag);
            }
            struct termios termios_flags;
            tcgetattr (master_pty_fd, &termios_flags);
            termios_flags.c_iflag &= ~(IGNPAR | INPCK
                    |INLCR | IGNCR
                    | ICRNL | IXON
                    |IXOFF | ISTRIP);
            termios_flags.c_iflag |= IGNBRK | BRKINT | IMAXBEL | IXANY;
            termios_flags.c_oflag &= ~OPOST;
            termios_flags.c_cflag &= ~(CSTOPB | CREAD | PARENB | HUPCL);
            termios_flags.c_cflag |= CS8 | CLOCAL;
            termios_flags.c_cc[VMIN] = 0;
            //echo off
            termios_flags.c_lflag &= ~(ECHOKE | ECHOE |ECHO| ECHONL | ECHOPRT
                    |ECHOCTL | ISIG | ICANON
                    |IEXTEN | NOFLSH | TOSTOP);
            cfsetospeed(&termios_flags, __MAX_BAUD);
            tcsetattr(master_pty_fd, TCSANOW, &termios_flags);
            a_pid = pid ;
            a_master_pty_fd = master_pty_fd ;
            a_stdout_fd = stdout_pipes[READ_PIPE] ;
            a_stderr_fd = stderr_pipes[READ_PIPE] ;
            gdb_pid = pid ;
        } else {
            //the fork failed.
            close (stderr_pipes[READ_PIPE]) ;
            close (stdout_pipes[READ_PIPE]) ;
            LOG_ERROR ("fork() failed\n") ;
            return false ;
        }
        return true ;
    }

    void set_communication_charset (const string &a_charset)
    {
        gdb_stdout_channel->set_encoding (a_charset) ;
        gdb_stderr_channel->set_encoding (a_charset) ;
        gdb_master_pty_channel->set_encoding (a_charset) ;
    }

    bool launch_gdb (const vector<UString> &a_prog_args,
            const vector<UString> &a_source_search_dirs,
            const UString a_gdb_options="")
    {
        if (is_gdb_running ()) {
            kill_gdb () ;
        }
        argv.clear () ;
        argv.push_back (env::get_gdb_program ()) ;
        argv.push_back ("--interpreter=mi2") ;
        if (a_gdb_options != "") {
            argv.push_back (a_gdb_options) ;
        }
        if (!a_prog_args.empty ()) {
            argv.push_back (a_prog_args[0]) ;
        }


        source_search_dirs = a_source_search_dirs;
        RETURN_VAL_IF_FAIL (launch_program (argv,
                    gdb_pid,
                    gdb_master_pty_fd,
                    gdb_stdout_fd,
                    gdb_stderr_fd),
                false) ;
        if (!gdb_pid) {return false;}

        gdb_stdout_channel =
        Glib::IOChannel::create_from_fd (gdb_stdout_fd) ;
        gdb_stderr_channel =
        Glib::IOChannel::create_from_fd (gdb_stderr_fd) ;
        gdb_master_pty_channel =
        Glib::IOChannel::create_from_fd (gdb_master_pty_fd) ;

        string charset ;
        Glib::get_charset (charset) ;
        set_communication_charset (charset) ;

        attach_channel_to_loop_context_as_source
        (Glib::IO_IN | Glib::IO_PRI
         | Glib::IO_HUP | Glib::IO_ERR,
         sigc::mem_fun
         (this,
          &Priv::on_gdb_master_pty_has_data_signal),
         gdb_master_pty_channel,
         get_event_loop_context ()) ;

        attach_channel_to_loop_context_as_source
        (Glib::IO_IN | Glib::IO_PRI
         | Glib::IO_HUP | Glib::IO_ERR,
         sigc::mem_fun
         (this,
          &Priv::on_gdb_stderr_has_data_signal),
         gdb_stderr_channel,
         get_event_loop_context ()) ;

        attach_channel_to_loop_context_as_source
        (Glib::IO_IN | Glib::IO_PRI
         | Glib::IO_HUP | Glib::IO_ERR,
         sigc::mem_fun
         (this,
          &Priv::on_gdb_stdout_has_data_signal),
         gdb_stdout_channel,
         get_event_loop_context ()) ;

        if (!a_prog_args.empty ()) {
            UString args ;
            for (vector<UString>::size_type i=1; i < a_prog_args.size () ; ++i) {
                args += a_prog_args[i] + " " ;
            }

            if (args != "") {
                return issue_command (Command ("set args " + args)) ;
            }
        }
        return true ;
    }

    bool issue_command (const IDebugger::Command &a_command,
            bool a_run_event_loops=false)
    {
        if (!gdb_master_pty_channel) {
            return false ;
        }
        if (gdb_master_pty_channel->write
                (a_command.value () + "\n") == Glib::IO_STATUS_NORMAL) {
            gdb_master_pty_channel->flush () ;
            if (a_run_event_loops) {
                run_loop_iterations (-1) ;
            }
            THROW_IF_FAIL (started_commands.size () <= 1) ;

            started_commands.push_back (a_command) ;
            return true ;
        }
        return false ;
    }

    bool on_gdb_master_pty_has_data_signal (Glib::IOCondition a_cond)
    {
        try {

            RETURN_VAL_IF_FAIL (gdb_master_pty_channel, false) ;
            if ((a_cond & Glib::IO_IN) || (a_cond & Glib::IO_PRI)) {
                char buf[513] = {0} ;
                gsize nb_read (0), CHUNK_SIZE(512) ;
                Glib::IOStatus status (Glib::IO_STATUS_NORMAL) ;
                bool got_data (false) ;
                while (true) {
                    status = gdb_master_pty_channel->read (buf,
                            CHUNK_SIZE,
                            nb_read) ;
                    if (status == Glib::IO_STATUS_NORMAL
                            && nb_read && (nb_read <= CHUNK_SIZE)) {
                        if (gdb_master_pty_buffer_status == FILLED) {
                            gdb_master_pty_buffer.clear () ;
                            gdb_master_pty_buffer_status = FILLING ;
                        }
                        std::string raw_str(buf, nb_read) ;
                        UString tmp = Glib::locale_to_utf8 (raw_str) ;
                        gdb_master_pty_buffer.append (tmp) ;
                        UString::size_type len = gdb_master_pty_buffer.size () ;
                        if (gdb_stdout_buffer[len - 1] == '\n'
                                && gdb_master_pty_buffer[len - 2] == ' '
                                && gdb_master_pty_buffer[len - 3] == ')'
                                && gdb_master_pty_buffer[len - 4] == 'b'
                                && gdb_master_pty_buffer[len - 5] == 'd'
                                && gdb_master_pty_buffer[len - 6] == 'g'
                                && gdb_master_pty_buffer[len - 7] == '(') {
                            got_data = true ;
                        }
                    } else {
                        break ;
                    }
                    nb_read = 0 ;
                }
                if (got_data) {
                    gdb_master_pty_buffer_status = FILLED ;
                    gdb_master_pty_signal.emit (gdb_master_pty_buffer) ;
                    gdb_master_pty_buffer.clear () ;
                }
            }
            if (a_cond & Glib::IO_HUP) {
                LOG_ERROR ("Connection lost from master pty channel") ;
                gdb_master_pty_channel.clear () ;
                //kill_gdb () ;
                //gdb_died_signal.emit () ;
            }
            if (a_cond & Glib::IO_ERR) {
                LOG_ERROR ("Error over the wire") ;
            }
        } catch (exception &e) {
        } catch (Glib::Error &e) {
        }
        return true ;
    }

    bool on_gdb_stdout_has_data_signal (Glib::IOCondition a_cond)
    {
        RETURN_VAL_IF_FAIL (gdb_stdout_channel, false) ;
        try {
            if ((a_cond & Glib::IO_IN) || (a_cond & Glib::IO_PRI)) {
                gsize nb_read (0), CHUNK_SIZE(512) ;
                char buf[CHUNK_SIZE+1] ;
                Glib::IOStatus status (Glib::IO_STATUS_NORMAL) ;
                bool got_data (false) ;
                while (true) {
                    memset (buf, 0, CHUNK_SIZE + 1) ;
                    status = gdb_stdout_channel->read (buf, CHUNK_SIZE, nb_read) ;
                    if (status == Glib::IO_STATUS_NORMAL
                            && nb_read && (nb_read <= CHUNK_SIZE)) {
                        if (gdb_stdout_buffer_status == FILLED) {
                            gdb_stdout_buffer.clear () ;
                            gdb_stdout_buffer_status = FILLING ;
                        }
                        std::string raw_str(buf, nb_read) ;
                        UString tmp = Glib::locale_to_utf8 (raw_str) ;
                        gdb_stdout_buffer.append (tmp) ;
                        UString::size_type len = gdb_stdout_buffer.size (), i=0 ;

                        while (isspace (gdb_stdout_buffer[len-i-1])) {++i;}

                        if (gdb_stdout_buffer[len-i-1] == ')'
                                && gdb_stdout_buffer[len-i-2] == 'b'
                                && gdb_stdout_buffer[len-i-3] == 'd'
                                && gdb_stdout_buffer[len-i-4] == 'g'
                                && gdb_stdout_buffer[len-i-5] == '(') {
                            got_data = true ;
                        }
                    } else {
                        break ;
                    }
                    nb_read = 0 ;
                }
                if (got_data) {
                    gdb_stdout_buffer_status = FILLED ;
                    gdb_stdout_signal.emit (gdb_stdout_buffer) ;
                    gdb_stdout_buffer.clear () ;
                }
            }
            if (a_cond & Glib::IO_HUP) {
                LOG_ERROR ("Connection lost from stdout channel to gdb") ;
                gdb_stdout_channel.clear () ;
                kill_gdb () ;
                gdb_died_signal.emit () ;
                LOG_ERROR ("GDB killed") ;
            }
            if (a_cond & Glib::IO_ERR) {
                LOG_ERROR ("Error over the wire") ;
            }
        } catch (Glib::Error &e) {
            TRACE_EXCEPTION (e) ;
            return false ;
        } catch (exception &e) {
            TRACE_EXCEPTION (e) ;
            return false ;
        } catch (...) {
            LOG_ERROR ("got an unknown exception") ;
            return false ;
        }

        return true ;
    }

    bool on_gdb_stderr_has_data_signal (Glib::IOCondition a_cond)
    {
        RETURN_VAL_IF_FAIL (gdb_stderr_channel, false) ;
        try {

            if (a_cond & Glib::IO_IN || a_cond & Glib::IO_PRI) {
                char buf[513] = {0} ;
                gsize nb_read (0), CHUNK_SIZE(512) ;
                Glib::IOStatus status (Glib::IO_STATUS_NORMAL) ;
                bool got_data (false) ;
                while (true) {
                    status = gdb_stderr_channel->read (buf,
                            CHUNK_SIZE,
                            nb_read) ;
                    if (status == Glib::IO_STATUS_NORMAL
                            && nb_read && (nb_read <= CHUNK_SIZE)) {
                        if (error_buffer_status == FILLED) {
                            gdb_stderr_buffer.clear () ;
                            error_buffer_status = FILLING ;
                        }
                        std::string raw_str(buf, nb_read) ;
                        UString tmp = Glib::locale_to_utf8 (raw_str) ;
                        gdb_stderr_buffer.append (tmp) ;
                        got_data = true ;

                    } else {
                        break ;
                    }
                    nb_read = 0 ;
                }
                if (got_data) {
                    error_buffer_status = FILLED ;
                    gdb_stderr_signal.emit (gdb_stderr_buffer) ;
                    gdb_stderr_buffer.clear () ;
                }
            }

            if (a_cond & Glib::IO_HUP) {
                gdb_stderr_channel.clear () ;
                //kill_gdb () ;
                //gdb_died_signal.emit () ;
            }
        } catch (exception e) {
        } catch (Glib::Error &e) {
        }
        return true ;
    }

    bool breakpoint_has_failed (const CommandAndOutput &a_in)
    {
        if (a_in.has_command ()
                && a_in.command ().value ().compare (0, 5, "break")) {
            return false ;
        }
        if (a_in.output ().has_result_record ()
                && a_in.output ().result_record ().breakpoints ().empty ()) {
            return true ;
        }
        return false ;
    }

    bool parse_attribute (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            UString &a_name,
            UString &a_value)
    {
        UString::size_type cur = a_from,
        end = a_input.size (),
        name_start = cur ;
        if (cur >= end) {return false;}

        UString name, value ;
        UString::value_type sep (0);
        //goto first '='
        for (;
                cur<end
                && !isspace (a_input[cur])
                && a_input[cur] != '=';
                ++cur) {
        }

        if (a_input[cur] != '='
                || ++cur >= end ) {
            return false ;
        }

        sep = a_input[cur] ;
        if (sep != '"' && sep != '{' && sep != '[') {return false;}
        if (sep == '{') {sep = '}';}
        if (sep == '[') {sep = ']';}

        if (++cur >= end) {
            return false;
        }
        UString::size_type name_end = cur-3, value_start = cur ;

        //goto $sep
        for (;
                cur<end
                && a_input[cur] != sep;
                ++cur) {
        }
        if (a_input[cur] != sep) {return false;}
        UString::size_type value_end = cur-1 ;

        name.assign (a_input, name_start, name_end - name_start + 1);
        value.assign (a_input, value_start, value_end-value_start + 1);

        a_to = ++cur ;
        a_name = name ;
        a_value = value ;
        return true ;
    }

    /// \brief parses an attribute list
    ///
    /// An attribute list has the form:
    /// attr0="val0",attr1="bal1",attr2="val2"
    bool parse_attributes (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            map<UString, UString> &a_attrs)
    {
        UString::size_type cur = a_from, end = a_input.size () ;

        if (cur == end) {return false;}
        UString name, value ;
        map<UString, UString> attrs ;

        while (true) {
            if (!parse_attribute (a_input, cur, cur, name, value)) {break ;}
            if (!name.empty () && !value.empty ()) {
                attrs[name] = value ;
                name.clear (); value.clear () ;
            }

            while (isspace (a_input[cur])) {++cur ;}
            if (cur >= end || a_input[cur] != ',') {break;}
            if (++cur >= end) {break;}
        }
        a_attrs = attrs ;
        a_to = cur ;
        return true ;
    }

    /// \brief parses a breakpoint definition as returned by gdb.
    ///
    ///breakpoint definition string looks like this:
    ///bkpt={number="3",type="breakpoint",disp="keep",enabled="y",
    ///addr="0x0804860e",func="func2()",file="fooprog.cc",line="13",times="0"}
    ///
    ///\param a_input the input string to parse.
    ///\param a_from where to start the parsing from
    ///\param a_to out parameter. A past the end iterator that
    /// point the the end of the parsed text. This is set if and only
    /// if the function completes successfuly
    /// \param a_output the output datatructure filled upon parsing.
    /// \return true in case of successful parsing, false otherwise.
    bool parse_breakpoint (const UString &a_input,
            Glib::ustring::size_type a_from,
            Glib::ustring::size_type &a_to,
            IDebugger::BreakPoint &a_bkpt)
    {
        Glib::ustring::size_type cur = a_from, end = a_input.size () ;

        if (a_input.compare (cur, 6, "bkpt={")) {return false;}

        cur += 6 ;
        if (cur >= end) {return false;}

        map<UString, UString> attrs ;
        bool is_ok = parse_attributes (a_input, cur, cur, attrs) ;
        if (!is_ok) {return false;}

        if (a_input[cur++] != '}') {return false;}

        map<UString, UString>::iterator iter, null_iter = attrs.end () ;
        if (   (iter = attrs.find ("number"))  == null_iter
                || (iter = attrs.find ("type"))    == null_iter
                || (iter = attrs.find ("disp"))    == null_iter
                || (iter = attrs.find ("enabled")) == null_iter
                || (iter = attrs.find ("addr"))    == null_iter
                || (iter = attrs.find ("func"))    == null_iter
                || (iter = attrs.find ("file"))    == null_iter
                || (iter = attrs.find ("fullname"))== null_iter
                || (iter = attrs.find ("line"))    == null_iter
                || (iter = attrs.find ("times"))   == null_iter
           ) {
            return false ;
        }

        a_bkpt.number (atoi (attrs["number"].c_str ())) ;
        if (attrs["enabled"] == "y") {
            a_bkpt.enabled (true) ;
        } else {
            a_bkpt.enabled (false) ;
        }
        a_bkpt.address (attrs["addr"]) ;
        a_bkpt.function (attrs["func"]) ;
        a_bkpt.file_name (attrs["file"]) ;
        a_bkpt.full_file_name (attrs["fullname"]) ;
        a_bkpt.line (atoi (attrs["line"].c_str ())) ;
        a_to = cur ;
        return true;
    }

    bool parse_breakpoint_table (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            map<int, IDebugger::BreakPoint> &a_breakpoints)
    {
        UString::size_type cur=a_from, end=a_input.size () ;

        if (a_input.compare (cur, 17, "BreakpointTable={")) {return false;}
        cur += 17 ;
        if (cur >= end) {return false;}

        //skip table headers and got to table body.
        cur = a_input.find ("body=[", 0)  ;
        if (!cur) {return false;}
        cur += 6 ;
        if (cur >= end) {return false;}

        map<int, IDebugger::BreakPoint> breakpoint_table ;
        if (a_input[cur] == ']') {
            //there are zero breakpoints ...
        } else if (!a_input.compare (cur, 6, "bkpt={")){
            //there are some breakpoints
            IDebugger::BreakPoint breakpoint ;
            while (true) {
                if (!parse_breakpoint (a_input, cur, cur, breakpoint)) {
                    break;
                }
                breakpoint_table[breakpoint.number ()] = breakpoint ;
                if (a_input[cur] == ',') {
                    ++cur ;
                    if (cur >= end) {return false;}
                }
                breakpoint.clear () ;
            }
            if (breakpoint_table.empty ()) {return false;}
        } else {
            //weird things are happening, get out.
            return false ;
        }

        if (a_input[cur] != ']') {return false;}
        ++cur ;
        if (cur >= end) {return false;}
        if (a_input[cur] != '}') {return false;}
        ++cur ;

        a_to = cur ;
        a_breakpoints = breakpoint_table ;
        return true;
    }

    bool parse_tuple (const UString &a_input,
                      UString::size_type a_from,
                      UString::size_type &a_to,
                      TupleSafePtr &a_tuple)
    {
        LOG_FUNCTION_SCOPE_NORMAL ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        if (a_input[cur] != '{') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        ++cur ;
        CHECK_END (a_input, cur, end) ;

        if (a_input[cur] == '}') {
            ++cur ;
            a_from = cur ;
            return true ;
        }

        TupleSafePtr tuple ;
        ResultSafePtr result ;

        for (;;) {
            if (parse_result (a_input, cur, cur, result)) {
                SKIP_WS (a_input, cur, cur) ;
                CHECK_END (a_input, cur, end) ;
                if (!tuple) {
                    tuple = TupleSafePtr (new Tuple) ;
                    THROW_IF_FAIL (tuple) ;
                }
                tuple->append (result) ;
                if (a_input[cur] == ',') {
                    ++cur ;
                    CHECK_END (a_input, cur, end) ;
                    SKIP_WS (a_input, cur, cur) ;
                    continue ;
                }
            } else {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false ;
            }
            break ;
        }

        SKIP_WS (a_input, cur, cur) ;
        if (a_input[cur] != '}') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        ++cur ;
        a_to = cur ;
        a_tuple = tuple ;
        return true ;
    }

    bool parse_result_value_list (const UString &a_input,
                                  UString::size_type a_from,
                                  UString::size_type &a_to,
                                  ResultValueListSafePtr &a_list)
    {
        LOG_FUNCTION_SCOPE_NORMAL ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        ResultValueListSafePtr return_list (new ResultValueList) ;
        if (a_input[cur] != '[') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        CHECK_END (a_input, cur + 1, end) ;
        if (a_input[cur + 1] == ']') {
            ++cur ;
            a_list = return_list ;
            a_to = ++cur ;
            return true ;
        }

        ++cur ;
        CHECK_END (a_input, cur, end) ;
        SKIP_WS (a_input, cur, cur) ;

        ValueSafePtr value ;
        ResultSafePtr result ;
        if (parse_result (a_input, cur, cur, result)) {
            THROW_IF_FAIL (result) ;
            return_list->append (result) ;
            CHECK_END (a_input, cur, end) ;
            for (;;) {
                if (a_input[cur] == ',') {
                    ++cur ;
                    CHECK_END (a_input, cur, end) ;
                    continue ;
                }
                break ;
            }
        } else if (parse_value (a_input, cur, cur, value)) {
            THROW_IF_FAIL (value);
            return_list->append (value) ;
            CHECK_END (a_input, cur, end) ;
            for (;;) {
                if (a_input[cur] == ',') {
                    ++cur ;
                    CHECK_END (a_input, cur, end) ;
                    continue ;
                }
                break ;
            }
        } else {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        a_to = cur ;
        a_list = return_list ;
        return true ;
    }

    bool parse_result (const UString &a_input,
                       UString::size_type a_from,
                       UString::size_type &a_to,
                       ResultSafePtr &a_value)
    {
        LOG_FUNCTION_SCOPE_NORMAL ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        UString variable ;
        if (!parse_string (a_input, cur, cur, variable)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        CHECK_END (a_input, cur, end) ;
        SKIP_WS (a_input, cur, cur) ;
        if (a_input[cur] != '=') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        ++cur ;
        CHECK_END (a_input, cur, end) ;

        ValueSafePtr value;
        if (!parse_value (a_input, cur, cur, value)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        THROW_IF_FAIL (value) ;

        ResultSafePtr result (new Result (variable, value)) ;
        THROW_IF_FAIL (result) ;
        a_to = cur ;
        a_value = result ;
        return true ;
    }

    /// \brief parse a GDB/MI value type.
    /// GDB/MI value type is defined as:
    /// VALUE ==> CONST | TUPLE | LIST
    /// \param a_input the input string to parse
    /// \param a_from where to start parsing from
    /// \param a_to (out parameter) a pointer to the current char, after the parsing.
    //// \param a_value the result of the parsing.
    bool parse_value (const UString &a_input,
                      UString::size_type a_from,
                      UString::size_type &a_to,
                      ValueSafePtr &a_value)
    {
        LOG_FUNCTION_SCOPE_NORMAL ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        ValueSafePtr value ;
        if (a_input[cur] == '"') {
            UString const_string ;
            if (parse_c_string (a_input, cur, cur, const_string)) {
                value = ValueSafePtr (new Value (const_string)) ;
            }
        } else if (a_input[cur] == '{') {
            TupleSafePtr tuple ;
            if (parse_tuple (a_input, cur, cur, tuple)) {
                THROW_IF_FAIL (tuple) ;
                value = ValueSafePtr (new Value (tuple)) ;
            }
        } else if (a_input[cur] == '[') {
            ResultValueListSafePtr list ;
            if (parse_result_value_list (a_input, cur, cur, list)) {
                THROW_IF_FAIL (list) ;
                value = ValueSafePtr (new Value (list)) ;
            }
        } else {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        if (!value) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        a_value = value ;
        a_to = cur ;
        return true ;
    }

    bool parse_call_stack (const UString &a_input,
                           const UString::size_type a_from,
                           UString::size_type &a_to,
                           list<IDebugger::Frame> &a_stack)
    {
        return false ;
    }

    /// \brief parse function arguments list
    ///
    /// function args list have the form:
    /// args=[{name="name0",value="0"},{name="name1",value="1"}]
    ///
    /// This function parses only started from (including) the first '{'
    /// \param a_input the input string to parse
    /// \param a_from where to parse from.
    /// \param out parameter. End of the parsed string. This is a past the end
    ///  offset.
    /// \param a_args the map of the parsed attributes. This is set if and
    /// only if the function returned true.
    /// \return true upon successful parsing, false otherwise.
    bool parse_function_args (const UString &a_input,
                              const UString::size_type a_from,
                              UString::size_type &a_to,
                              map<UString, UString> a_args)
    {
        UString::size_type cur = a_from, end = a_input.size () ;

        if (a_input.compare (cur, 1, "{")) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        cur ++  ;
        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        UString::size_type name_start (0),
        name_end (0),
        value_start (0),
        value_end (0);

        Glib::ustring name, value ;
        map<UString, UString> args ;

        while (true) {
            if (a_input.compare (cur, 6, "name=\"")) {break;}
            cur += 6 ;
            if (cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            name_start = cur;
            for (; cur < end && a_input[cur] != '"'; ++cur) {}
            if (a_input[cur] != '"') {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            name_end = cur - 1 ;
            if (++cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            if (a_input.compare (cur, 8, ",value=\"")) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            cur += 8 ; if (cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            value_start = cur ;
            for (; cur < end && a_input[cur] != '"'; ++cur) {}
            if (a_input[cur] != '"') {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            value_end = cur - 1 ;
            name.clear (), value.clear () ;
            name.assign (a_input, name_start, name_end - name_start + 1) ;
            value.assign (a_input, value_start, value_end - value_start + 1) ;
            args[name] = value ;
            if (++cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            if (a_input[cur] != '}') {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            if (++cur >= end) {break;}

            if (!a_input.compare(cur, 2,",{") ){
                cur += 2 ;
                continue ;
            } else {
                break ;
            }
        }

        a_args = args ;
        a_to = cur ;
        return true;
    }

    /// \brief parses a function frame
    ///
    /// function frames have the form:
    /// frame={addr="0x080485fa",func="func1",args=[{name="foo", value="bar"}],
    /// file="fooprog.cc",fullname="/foo/fooprog.cc",line="6"}
    ///
    /// \param a_input the input string to parse
    /// \param a_from where to parse from.
    /// \param a_to out parameter. Where the parser went after the parsing.
    /// \param a_frame the parsed frame. It is set if and only if the function
    ///  returns true.
    /// \return true upon successful parsing, false otherwise.
    bool parse_frame (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            IDebugger::Frame &a_frame)
    {
        UString::size_type cur = a_from, end = a_input.size () ;
        if (a_input.compare (a_from, 7, "frame={")) {
            return false ;
        }
        cur += 7 ;
        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        map<UString, UString> attrs ;
        if (!parse_attributes (a_input, cur, cur, attrs)) {return false;}
        if (a_input[cur] != '}') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        ++cur ;

        map<UString, UString>::const_iterator iter, null_iter = attrs.end () ;
        if (   (iter = attrs.find ("addr")) == null_iter) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        a_frame.address (attrs["addr"]) ;
        a_frame.function (attrs["func"]) ;
        UString args_str = attrs["args"] ;
        if (args_str != "") {
            map<UString, UString> args ;
            UString::size_type from(0), to(0) ;
            if (!parse_function_args (args_str, from, to, args)) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            a_frame.args () = args ;
        }
        a_frame.file_name (attrs["file"]) ;
        a_frame.file_full_name (attrs["fullname"]) ;
        a_frame.line (atoi (attrs["line"].c_str())) ;
        a_frame.library (attrs["from"].c_str()) ;
        a_to = cur ;
        return true;
    }

    /*
    /// parses a string that has the form:
    /// \"blah\"
    bool parse_c_string (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            UString &a_c_string)
    {
        UString::size_type cur=a_from, end = a_input.size () ;

        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        if (a_input[cur] != '"') {return false;}
        ++cur ;

        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        UString::size_type str_start = cur, str_end (0) ;

        while (true) {
            if (cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            if (a_input[cur] == '"' && a_input[cur - 1] != '\\') {break ;}
            ++cur ;
        }
        if (a_input[cur] != '"') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        str_end = cur - 1 ;
        ++cur ;
        Glib::ustring str (a_input, str_start, str_end - str_start + 1) ;
        a_c_string = str ;
        a_to = cur ;
        return true ;
    }
    */

    /// parses a string that has the form:
    /// \"blah\"
    bool parse_c_string (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            UString &a_c_string)
    {
        LOG_FUNCTION_SCOPE_NORMAL ;
        UString::size_type cur=a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        if (a_input[cur] != '"') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        ++cur ;
        CHECK_END (a_input, cur, end) ;

        UString str ;
        if (!parse_c_string_body (a_input, cur, cur, str)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        if (a_input[cur] != '"') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        ++cur ;
        a_c_string = str ;
        a_to = cur ;
        return true ;
    }

    bool parse_c_string_body (const UString &a_input,
                              UString::size_type a_from,
                              UString::size_type &a_to,
                              UString &a_string)
    {
        LOG_FUNCTION_SCOPE_NORMAL ;
        UString::size_type cur=a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        if (a_input[cur] == '"') {
            a_string = "" ;
            a_to = cur ;
            return true ;
        }

        if (!isascii (a_input[cur])) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        UString::size_type str_start (cur), str_end (0) ;
        ++cur ;
        CHECK_END (a_input, cur, end) ;

        for (;;) {
            if (isascii (a_input[cur])) {
                if (a_input[cur] == '"' && a_input[cur-1] != '\\') {
                    break ;
                }
                ++cur ;
                CHECK_END (a_input, cur, end) ;
                continue ;
            }
            str_end = cur - 1 ;
            break;
        }
        Glib::ustring str (a_input, str_start, str_end - str_start + 1) ;
        a_string = str ;
        a_to = cur ;
        return true ;
    }

    /// parses a string that has the form:
    /// blah
    bool parse_string (const UString &a_input,
                       UString::size_type a_from,
                       UString::size_type &a_to,
                       UString &a_string)
    {
        LOG_FUNCTION_SCOPE_NORMAL ;
        UString::size_type cur=a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        if (!isalpha (a_input[cur]) && a_input[cur] != '_') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        UString::size_type str_start (cur), str_end (0) ;
        ++cur ;
        CHECK_END (a_input, cur, end) ;

        for (;;) {
            if (isalnum (a_input[cur])
                || a_input[cur] == '_'
                || a_input[cur] == '-') {
                ++cur ;
                CHECK_END (a_input, cur, end) ;
                continue ;
            }
            str_end = cur - 1 ;
            break;
        }
        Glib::ustring str (a_input, str_start, str_end - str_start + 1) ;
        a_string = str ;
        a_to = cur ;
        return true ;
    }


    bool parse_stream_record (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            Output::StreamRecord &a_record)
    {
        UString::size_type cur=a_from, end = a_input.size () ;

        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        UString console, target, log ;

        if (a_input[cur] == '~') {
            //console stream output
            ++cur ;
            if (cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false ;
            }
            if (!parse_c_string (a_input, cur, cur, console)) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
        } else if (a_input[cur] == '@') {
            //target stream output
            ++cur ;
            if (cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false ;
            }
            if (!parse_c_string (a_input, cur, cur, target)) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
        } else if (a_input[cur] == '&') {
            //log stream output
            ++cur ;
            if (cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false ;
            }
            if (!parse_c_string (a_input, cur, cur, log)) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
        } else {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        for (; cur < end && isspace (a_input[cur]) ; ++cur) {}
        bool found (false) ;
        if (!console.empty ()) {
            found = true ;
            a_record.debugger_console (console) ;
        }
        if (!target.empty ()) {
            found = true ;
            a_record.target_output (target) ;
        }
        if (!log.empty ()) {
            found = true ;
            a_record.debugger_log (log) ;
        }

        if (!found) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        a_to = cur ;
        return true;
    }

    bool parse_stopped_async_output (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            bool &a_got_frame,
            IDebugger::Frame &a_frame,
            map<UString, UString> &a_attrs)
    {
        UString::size_type cur=a_from, end=a_input.size () ;

        if (cur >= end) {return false;}

        if (a_input.compare (cur, 9,"*stopped,")) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        cur += 9 ; if (cur >= end) {return false;}

        map<UString, UString> attrs ;
        UString name, value;
        bool got_frame (false) ;
        IDebugger::Frame frame ;
        while (true) {
            if (!a_input.compare (cur, 7, "frame={")) {
                if (!parse_frame (a_input, cur, cur, frame)) {
                    LOG_PARSING_ERROR (a_input, cur) ;
                    return false;
                }
                got_frame = true ;
            } else {
                if (!parse_attribute (a_input, cur, cur, name, value)) {break;}
                attrs[name] = value ;
                name.clear () ; value.clear () ;
            }

            if (cur >= end) {break ;}
            if (a_input[cur] == ',') {++cur;}
            if (cur >= end) {break ;}
        }

        for (; cur < end && a_input[cur] != '\n' ; ++cur) {}

        if (a_input[cur] != '\n') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }
        ++cur ;

        a_got_frame = got_frame ;
        if (a_got_frame) {
            a_frame = frame ;
        }
        a_to = cur ;
        a_attrs = attrs ;
        return true ;
    }

    Output::OutOfBandRecord::StopReason str_to_stopped_reason
    (const UString &a_str)
    {
        if (a_str == "breakpoint-hit") {
            return Output::OutOfBandRecord::BREAKPOINT_HIT ;
        } else if (a_str == "watchpoint-trigger") {
            return Output::OutOfBandRecord::WATCHPOINT_TRIGGER ;
        } else if (a_str == "read-watchpoint-trigger") {
            return Output::OutOfBandRecord::READ_WATCHPOINT_TRIGGER ;
        } else if (a_str == "function-finished") {
            return Output::OutOfBandRecord::FUNCTION_FINISHED;
        } else if (a_str == "location-reached") {
            return Output::OutOfBandRecord::LOCATION_REACHED;
        } else if (a_str == "watchpoint-scope") {
            return Output::OutOfBandRecord::WATCHPOINT_SCOPE;
        } else if (a_str == "end-stepping-range") {
            return Output::OutOfBandRecord::END_STEPPING_RANGE;
        } else if (a_str == "exited-signalled") {
            return Output::OutOfBandRecord::EXITED_SIGNALLED;
        } else if (a_str == "exited") {
            return Output::OutOfBandRecord::EXITED;
        } else if (a_str == "exited-normally") {
            return Output::OutOfBandRecord::EXITED_NORMALLY;
        } else if (a_str == "signal-received") {
            return Output::OutOfBandRecord::SIGNAL_RECEIVED;
        } else {
            return Output::OutOfBandRecord::UNDEFINED ;
        }
    }

    bool parse_out_of_band_record (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            Output::OutOfBandRecord &a_record)
    {
        UString::size_type cur=a_from, end = a_input.size () ;
        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        Output::OutOfBandRecord record ;
        if (   a_input[cur] == '~'
                || a_input[cur] == '@'
                || a_input[cur] == '&') {
            Output::StreamRecord stream_record ;
            if (!parse_stream_record (a_input, cur, cur,
                        stream_record)) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            record.has_stream_record (true) ;
            record.stream_record (stream_record) ;

            while (cur < end && isspace (a_input[cur])) {++cur;}
        }

        if (!a_input.compare (cur, 9,"*stopped,")) {
            map<UString, UString> attrs ;
            bool got_frame (false) ;
            IDebugger::Frame frame ;
            if (!parse_stopped_async_output (a_input, cur, cur,
                        got_frame, frame, attrs)) {
                return false ;
            }
            record.is_stopped (true) ;
            record.stop_reason (str_to_stopped_reason (attrs["reason"])) ;
            if (got_frame) {
                record.frame (frame) ;
                record.has_frame (true);
            }

            if (attrs.find ("bkptno") != attrs.end ()) {
                record.breakpoint_number (atoi (attrs["bkptno"].c_str ())) ;
            }
            record.thread_id (atoi (attrs["thread-id"].c_str ())) ;
        }

        while (cur < end && isspace (a_input[cur])) {++cur;}
        a_to = cur ;
        a_record = record ;
        return true ;
    }

    bool parse_result_record (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            Output::ResultRecord &a_record)
    {
        UString::size_type cur=a_from, end=a_input.size () ;
        if (cur == end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        UString name, value ;
        Output::ResultRecord result_record ;
        if (!a_input.compare (cur, 5, "^done")) {
            cur += 5 ;
            result_record.kind (Output::ResultRecord::DONE) ;
            if (cur < end && a_input[cur] == ',') {
                cur++;
                if (cur >= end) {
                    LOG_PARSING_ERROR (a_input, cur) ;
                    return false;
                }

                if (!a_input.compare (cur, 6, "bkpt={")) {
                    IDebugger::BreakPoint breakpoint ;
                    if (parse_breakpoint (a_input, cur, cur, breakpoint)) {
                        result_record.breakpoints ()[breakpoint.number ()] =
                        breakpoint ;
                    }
                } else if (!a_input.compare (cur, 17, "BreakpointTable={")) {
                    map<int, IDebugger::BreakPoint> breaks ;
                    parse_breakpoint_table (a_input, cur, cur, breaks) ;
                    result_record.breakpoints () = breaks ;
                } else if (!a_input.compare (cur, 7, "stack=[")) {
                    ResultSafePtr result ;
                    parse_result (a_input, cur, cur, result) ;
                    THROW_IF_FAIL (result) ;
                    LOG ("parsed result") ;
                }

                for (;cur < end && a_input[cur] != '\n';++cur) {}
            }
        } else if (!a_input.compare (cur, 8, "^running")) {
            result_record.kind (Output::ResultRecord::RUNNING) ;
            cur += 8 ;
            for (;cur < end && a_input[cur] != '\n';++cur) {}
        } else if (!a_input.compare (cur, 5, "^exit")) {
            result_record.kind (Output::ResultRecord::EXIT) ;
            cur += 5 ;
            for (;cur < end && a_input[cur] != '\n';++cur) {}
        } else if (!a_input.compare (cur, 10, "^connected")) {
            result_record.kind (Output::ResultRecord::CONNECTED) ;
            cur += 10 ;
            for (;cur < end && a_input[cur] != '\n';++cur) {}
        } else if (!a_input.compare (cur, 6, "^error")) {
            result_record.kind (Output::ResultRecord::ERROR) ;
            cur += 6 ; if (cur >= end)
            if (cur < end && a_input[cur] == ',') {++cur ;}
            if (cur >= end) {return false;}
            if (!parse_attribute (a_input, cur, cur, name, value)) {
                return false;
            }
            if (name != "") {
                result_record.attrs ()[name] = value ;
            }
            for (;cur < end && a_input[cur] != '\n';++cur) {}
        } else {
            return false ;
        }

        if (a_input[cur] != '\n') {return false;}

        a_record = result_record ;
        a_to = cur ;
        return true;
    }

    bool parse_output_record (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            Output &a_output)
    {
        UString::size_type cur=a_from, end=a_input.size () ;

        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        Output output ;

fetch_out_of_band_record:
        if (   a_input[cur] == '*'
                || a_input[cur] == '~'
                || a_input[cur] == '@'
                || a_input[cur] == '&'
                || a_input[cur] == '+'
                || a_input[cur] == '=') {
            Output::OutOfBandRecord oo_record ;
            if (!parse_out_of_band_record (a_input, cur, cur, oo_record)) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
            output.has_out_of_band_record (true) ;
            output.out_of_band_records ().push_back (oo_record) ;
            goto fetch_out_of_band_record ;
        }

        if (cur >= end) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false;
        }

        if (a_input[cur] == '^') {
            Output::ResultRecord result_record ;
            if (parse_result_record (a_input, cur, cur, result_record)) {
                output.has_result_record (true) ;
                output.result_record (result_record) ;
            }
            if (cur >= end) {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false;
            }
        }

        while (cur < end && isspace (a_input[cur])) {++cur;}

        if (!a_input.compare (cur, 5, "(gdb)")) {
            cur += 5 ;
        }

        if (cur == a_from) {
            //we didn't parse anything
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        while (cur < end && isspace (a_input[cur])) {++cur;}

        a_output = output ;
        a_to = cur ;
        return true;
    }

    ~Priv ()
    {
        kill_gdb () ;
    }
};//end GDBEngine::Priv

    //*************************
    //</GDBEngine::Priv struct>
    //*************************

    //****************************
    //<GDBengine output handlers
    //***************************

    struct OnStreamRecordHandler: OutputHandler{
        GDBEngine *m_engine ;

        OnStreamRecordHandler (GDBEngine *a_engine) :
            m_engine (a_engine)
        {}

        bool can_handle (CommandAndOutput &a_in)
        {
            if (!a_in.output ().has_out_of_band_record ()) {
                return false;
            }
            return true ;
        }

        void do_handle (CommandAndOutput &a_in)
        {
            THROW_IF_FAIL (m_engine) ;

            list<Output::OutOfBandRecord>::const_iterator iter ;
            for (iter = a_in.output ().out_of_band_records ().begin ();
                    iter != a_in.output ().out_of_band_records ().end ();
                    ++iter) {
                if (iter->has_stream_record ()) {
                    if (iter->stream_record ().debugger_console () != ""){
                        m_engine->console_message_signal ().emit
                        (iter->stream_record ().debugger_console ()) ;
                    }
                    if (iter->stream_record ().target_output () != ""){
                        m_engine->target_output_message_signal ().emit
                        (iter->stream_record ().target_output ()) ;
                    }
                    if (iter->stream_record ().debugger_log () != ""){
                        m_engine->error_message_signal ().emit
                        (iter->stream_record ().debugger_log ()) ;
                    }
                }
            }

        }
    };//end struct OnStreamRecordHandler

    struct OnBreakPointHandler: OutputHandler {
        GDBEngine * m_engine ;

        OnBreakPointHandler (GDBEngine *a_engine=NULL) :
            m_engine (a_engine)
        {}

        bool has_breakpoints_set (CommandAndOutput &a_in)
        {
            if (a_in.output ().has_result_record ()
                    && a_in.output ().result_record ().breakpoints ().size ()) {
                return true ;
            }
            return false ;
        }

        bool can_handle (CommandAndOutput &a_in)
        {
            if (!a_in.output ().has_result_record ()) {
                return false;
            }
            return true ;
        }

        void do_handle (CommandAndOutput &a_in)
        {
            THROW_IF_FAIL (m_engine) ;

            bool has_breaks=false ;
            //if breakpoint where set, put them in cache !
            if (has_breakpoints_set (a_in)) {
                m_engine->append_breakpoints_to_cache
                (a_in.output ().result_record ().breakpoints ()) ;
                has_breaks=true;
            }

            if (a_in.output ().has_result_record ()
                    && a_in.output ().result_record ().kind ()
                    == Output::ResultRecord::DONE
                    && a_in.command ().value ().find ("-break-delete")
                    != Glib::ustring::npos) {
                UString tmp = a_in.command ().value () ;
                tmp = tmp.erase (0, 13) ;
                if (tmp.size () == 0) {return ;}
                tmp.chomp () ;
                int bkpt_number = atoi (tmp.c_str ()) ;
                if (bkpt_number) {
                    map<int, IDebugger::BreakPoint>::iterator iter ;
                    map<int, IDebugger::BreakPoint> &breaks =
                    m_engine->get_cached_breakpoints () ;
                    iter = breaks.find (bkpt_number) ;
                    if (iter != breaks.end ()) {
                        m_engine->breakpoint_deleted_signal ().emit
                        (iter->second, iter->first) ;
                        breaks.erase (iter) ;
                    }
                }
            } else if (has_breaks){
                m_engine->breakpoints_set_signal ().emit
                (a_in.output ().result_record ().breakpoints ()) ;
            }
        }
    };//end struct OnBreakPointHandler

    struct OnStoppedHandler: OutputHandler {
        GDBEngine *m_engine ;
        Output::OutOfBandRecord m_out_of_band_record ;
        bool m_is_stopped ;

        OnStoppedHandler (GDBEngine *a_engine) :
            m_engine (a_engine),
            m_is_stopped (false)
        {}

        bool can_handle (CommandAndOutput &a_in)
        {
            if (!a_in.output ().has_out_of_band_record ()) {
                return false;
            }
            list<Output::OutOfBandRecord>::iterator iter ;

            for (iter = a_in.output ().out_of_band_records ().begin () ;
                    iter != a_in.output ().out_of_band_records ().end () ;
                    ++iter) {
                if (iter->is_stopped ()) {
                    m_is_stopped = true ;
                    m_out_of_band_record = *iter ;
                    return true ;
                }
            }
            return false;
        }

        void do_handle (CommandAndOutput &a_in)
        {
            THROW_IF_FAIL (m_is_stopped
                    && m_engine) ;
            m_engine->stopped_signal ().emit
            (m_out_of_band_record.stop_reason_as_str (),
             m_out_of_band_record.has_frame (),
             m_out_of_band_record.frame ()) ;
        }
    };//end struct OnStoppedHandler

    struct OnCommandDoneHandler : OutputHandler {

        GDBEngine *m_engine ;

        OnCommandDoneHandler (GDBEngine *a_engine) :
            m_engine (a_engine)
        {}

        bool can_handle (CommandAndOutput &a_in)
        {
            if (a_in.output ().has_result_record () &&
                    a_in.output ().result_record ().kind ()
                    == Output::ResultRecord::DONE) {
                return true ;
            }
            return false ;
        }

        void do_handle (CommandAndOutput &a_in)
        {
            m_engine->command_done_signal ().emit (a_in.command ().value ()) ;
        }
    };//struct OnCommandDoneHandler

    struct OnRunningHandler : OutputHandler {

        GDBEngine *m_engine ;

        OnRunningHandler (GDBEngine *a_engine) :
            m_engine (a_engine)
        {}

        bool can_handle (CommandAndOutput &a_in)
        {
            if (a_in.output ().has_result_record () &&
                    a_in.output ().result_record ().kind ()
                    == Output::ResultRecord::RUNNING) {
                return true ;
            }
            return false ;
        }

        void do_handle (CommandAndOutput &a_in)
        {
            m_engine->running_signal ().emit () ;
        }
    };//struct OnRunningHandler

    //****************************
    //</GDBengine output handlers
    //***************************

    //****************************
    //<GDBEngine methods>
    //****************************
GDBEngine::GDBEngine ()
{
    m_priv = new Priv ;
    init () ;
}

GDBEngine::~GDBEngine ()
{
    m_priv = 0;
}

void
GDBEngine::load_program (const vector<UString> &a_argv,
                         const vector<UString> &a_source_search_dirs,
                         bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (!a_argv.empty ()) ;

    if (!m_priv->is_gdb_running ()) {
        THROW_IF_FAIL (m_priv->launch_gdb (a_argv, a_source_search_dirs)) ;

        IDebugger::Command command ;

        command.value ("set breakpoint pending auto") ;
        queue_command (command) ;
    } else {
        UString args ;
        UString::size_type len (a_argv.size ()) ;
        for (UString::size_type i = 1 ; i < len; ++i) {
            args += " " + a_argv[i] ;
        }

        IDebugger::Command command
                        (UString ("-file-exec-and-symbols ") + a_argv[0]) ;
        queue_command (command) ;

        command.value ("set args " + args) ;
        queue_command (command) ;
    }
}

void
GDBEngine::attach_to_program (unsigned int a_pid)
{
    THROW_IF_FAIL (m_priv) ;
    vector<UString> args, source_search_dirs ;

    if (!m_priv->is_gdb_running ()) {
        THROW_IF_FAIL (m_priv->launch_gdb (args,
                    source_search_dirs)) ;

        IDebugger::Command command ;
        command.value ("set breakpoint pending auto") ;
        queue_command (command) ;
    }
    queue_command (Command ("attach " + UString::from_int (a_pid))) ;
}

void
GDBEngine::init_output_handlers ()
{
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnStreamRecordHandler (this))) ;
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnStoppedHandler (this))) ;
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnBreakPointHandler (this))) ;
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnCommandDoneHandler (this))) ;
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnRunningHandler (this))) ;
}

void
GDBEngine::get_info (Info &a_info) const
{
    const static Info s_info ("debuggerengine",
                              "The GDB debugger engine backend. "
                              "Implements the IDebugger interface",
                              "1.0") ;
    a_info = s_info ;
}

void
GDBEngine::do_init ()
{
}

sigc::signal<void, Output&>&
GDBEngine::pty_signal () const
{
    return m_priv->pty_signal ;
}

sigc::signal<void, Output&>&
GDBEngine::stderr_signal () const
{
    return m_priv->stderr_signal ;
}

sigc::signal<void, CommandAndOutput&>&
GDBEngine::stdout_signal () const
{
    return m_priv->stdout_signal ;
}

sigc::signal<void>&
GDBEngine::engine_died_signal () const
{
    return m_priv->gdb_died_signal ;
}

sigc::signal<void, const UString&>&
GDBEngine::console_message_signal () const
{
    return m_priv->console_message_signal ;
}

sigc::signal<void, const UString&>&
GDBEngine::target_output_message_signal () const
{
    return m_priv->target_output_message_signal ;
}

sigc::signal<void, const UString&>&
GDBEngine::error_message_signal () const
{
    return m_priv->error_message_signal ;
}

sigc::signal<void, const UString&>&
GDBEngine::command_done_signal () const
{
    return m_priv->command_done_signal ;
}

sigc::signal<void, const IDebugger::BreakPoint&, int>&
GDBEngine::breakpoint_deleted_signal () const
{
    return m_priv->breakpoint_deleted_signal ;
}

sigc::signal<void, const map<int, IDebugger::BreakPoint>&>&
GDBEngine::breakpoints_set_signal () const
{
    return m_priv->breakpoints_set_signal ;
}

sigc::signal<void, const UString&, bool, const IDebugger::Frame&>&
GDBEngine::stopped_signal () const
{
    return m_priv->stopped_signal ;
}

sigc::signal<void, const list<IDebugger::Frame>& >&
GDBEngine::frames_listed_signal () const
{
    return m_priv->frames_listed_signal ;
}

sigc::signal<void>&
GDBEngine::running_signal () const
{
    return m_priv->running_signal ;
}

sigc::signal<void>&
GDBEngine::program_finished_signal () const
{
    return m_priv->program_finished_signal ;
}

//******************
//<signal handlers>
//******************
void
GDBEngine::on_debugger_stdout_signal (CommandAndOutput &a_cao)
{
    //****************************************
    //call the output handlers so that we can
    //emit proper, more precise signal
    //to give greater benefit to the clients.
    //****************************************

    NEMIVER_TRY

    list<OutputHandlerSafePtr>::iterator iter ;
    for (iter = m_priv->output_handlers.begin () ;
            iter != m_priv->output_handlers.end ();
            ++iter)
    {
        if ((*iter)->can_handle (a_cao)) {
            (*iter)->do_handle (a_cao) ;
        }
    }

    NEMIVER_CATCH_NOX
}

//******************
//</signal handlers>
//******************
void
GDBEngine::init ()
{
    stdout_signal ().connect
    (sigc::mem_fun (*this,
                    &GDBEngine::on_debugger_stdout_signal)) ;
    init_output_handlers () ;
}

map<UString, UString>&
GDBEngine::properties ()
{
    return m_priv->properties;
}

void
GDBEngine::set_event_loop_context
(const Glib::RefPtr<Glib::MainContext> &a_ctxt)
{
    m_priv->set_event_loop_context (a_ctxt) ;
}

void
GDBEngine::run_loop_iterations (int a_nb_iters)
{
    THROW_IF_FAIL (m_priv) ;
    m_priv->run_loop_iterations (a_nb_iters) ;
}

void
GDBEngine::execute_command (const IDebugger::Command &a_command)
{
    THROW_IF_FAIL (m_priv && m_priv->is_gdb_running ()) ;
    queue_command (a_command) ;
}

bool
GDBEngine::queue_command (const IDebugger::Command &a_command)
{
    bool result (false) ;
    THROW_IF_FAIL (m_priv && m_priv->is_gdb_running ()) ;
    m_priv->queued_commands.push_back (a_command) ;
    if (m_priv->started_commands.empty ()) {
        result = m_priv->issue_command (*m_priv->queued_commands.begin ()) ;
        m_priv->queued_commands.erase (m_priv->queued_commands.begin ()) ;
    }
    return result ;
}

bool
GDBEngine::busy () const
{
    return false ;
}


void
GDBEngine::do_continue (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-continue")) ;
}

void
GDBEngine::run (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-run")) ;
}

void
GDBEngine::step_in (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-step")) ;
}

void
GDBEngine::step_out (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-finish")) ;
}

void
GDBEngine::step_over (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-next")) ;
}

void
GDBEngine::continue_to_position (const UString &a_path,
        gint a_line_num,
        bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-until "
                + a_path
                + ":"
                + UString::from_int (a_line_num))) ;
}

void
GDBEngine::set_breakpoint (const UString &a_path,
        gint a_line_num,
        bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    //here, don't use the gdb/mi format, because only the cmd line
    //format supports the 'set breakpoint pending' option that lets
    //gdb set pending breakpoint when a breakpoint location doesn't exist.
    //read http://sourceware.org/gdb/current/onlinedocs/gdb_6.html#SEC33
    //Also, we don't neet to explicitely 'set breakpoint pending' to have it
    //work. Even worse, setting it doesn't work.
    queue_command (Command ("break "
                + a_path
                + ":"
                + UString::from_int (a_line_num))) ;
    queue_command (Command ("-break-list ")) ;
}

void
GDBEngine::list_breakpoints (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-break-list")) ;
}

map<int, IDebugger::BreakPoint>&
GDBEngine::get_cached_breakpoints ()
{
    THROW_IF_FAIL (m_priv) ;
    return m_priv->cached_breakpoints ;
}

void
GDBEngine::append_breakpoints_to_cache
                            (const map<int, IDebugger::BreakPoint> &a_breaks)
{
    map<int, IDebugger::BreakPoint>::const_iterator iter ;
    for (iter = a_breaks.begin () ; iter != a_breaks.end () ; ++iter) {
        m_priv->cached_breakpoints[iter->first] = iter->second ;
    }
}

void
GDBEngine::set_breakpoint (const UString &a_func_name,
                           bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-break-insert " + a_func_name)) ;
}

void
GDBEngine::enable_breakpoint (const UString &a_path,
                              gint a_line_num,
                              bool a_run_event_loops)
{
}

void
GDBEngine::disable_breakpoint (const UString &a_path,
                               gint a_line_num,
                               bool a_run_event_loops)
{
}

void
GDBEngine::delete_breakpoint (const UString &a_path,
                              gint a_line_num,
                              bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-break-delete "
                + a_path
                + ":"
                + UString::from_int (a_line_num))) ;
}

void
GDBEngine::delete_breakpoint (gint a_break_num, bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-break-delete "
                + UString::from_int (a_break_num))) ;
}

//****************************
//</GDBEngine methods>
//****************************

}//end namespace nemiver

//the dynmod initial factory.
extern "C" {
bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    *a_new_instance = new nemiver::GDBEngine () ;
    return (*a_new_instance != 0) ;
}

}//end extern C

