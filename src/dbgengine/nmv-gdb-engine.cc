// Author: Dodji Seketeli
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
#include "nmv-proc-utils.h"

const nemiver::common::UString GDBMI_FRAME_PARSING_DOMAIN =
                                                    "gdbmi-frame-parsing-domain";
const nemiver::common::UString GDBMI_OUTPUT_DOMAIN = "gdbmi-output-domain" ;

#define LOG_PARSING_ERROR(a_buf, a_from) \
{ \
Glib::ustring str_01 (a_buf, (a_from), a_buf.size () - (a_from)) ;\
LOG_ERROR_D ("parsing failed for buf: >>>" \
             << a_buf << "<<<" \
             << " cur index was: " << (int)(a_from), \
             NMV_DEFAULT_DOMAIN) ;\
}

#define CHECK_END(a_input, a_current, a_end) \
if (a_current >= (a_end)) {LOG_PARSING_ERROR (a_input, (a_current)); return false ;}

#define SKIP_WS(a_input, a_from, a_to) \
while (isspace (a_input[a_from])) {CHECK_END (a_input, a_from, end);++a_from;} a_to = a_from ;

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
        vector<IDebugger::Frame> m_call_stack ;
        bool m_has_call_stack ;
        map<int, list<IDebugger::VariableSafePtr> > m_frames_parameters ;
        bool m_has_frames_parameters;
        list<IDebugger::VariableSafePtr> m_local_variables ;
        bool m_has_local_variables ;

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

        bool has_call_stack () const {return m_has_call_stack;}
        void has_call_stack (bool a_flag) {m_has_call_stack = a_flag;}

        const vector<IDebugger::Frame>& call_stack () const {return m_call_stack;}
        vector<IDebugger::Frame>& call_stack () {return m_call_stack;}
        void call_stack (const vector<IDebugger::Frame> &a_in)
        {
            m_call_stack = a_in;
            has_call_stack (true) ;
        }

        const map<int, list<IDebugger::VariableSafePtr> >&
                                                    frames_parameters () const
        {
            return m_frames_parameters ;
        }
        void frames_parameters
                    (const map<int, list<IDebugger::VariableSafePtr> > &a_in)
        {
            m_frames_parameters = a_in ;
            has_frames_parameters (true) ;
        }

        bool has_frames_parameters () const {return m_has_frames_parameters;}
        void has_frames_parameters (bool a_yes) {m_has_frames_parameters = a_yes;}

        const list<IDebugger::VariableSafePtr>& local_variables () const
        {
            return m_local_variables ;
        }
        void local_variables (const list<IDebugger::VariableSafePtr> &a_in)
        {
            m_local_variables = a_in ;
            has_local_variables (true) ;
        }

        bool has_local_variables () const {return m_has_local_variables;}
        void has_local_variables (bool a_in) {m_has_local_variables = a_in;}

        /// @}

        void clear ()
        {
            m_kind = UNDEFINED ;
            m_breakpoints.clear () ;
            m_attrs.clear () ;
            m_call_stack.clear () ;
            m_has_call_stack = false ;
            m_frames_parameters.clear () ;
            m_has_frames_parameters = false ;
            m_local_variables.clear () ;
            m_has_local_variables = false ;
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
    void command (const IDebugger::Command &a_in)
    {
        m_command = a_in; has_command (true);
    }

    const Output& output () const {return m_output;}
    Output& output () {return m_output;}
    void output (const Output &a_in) {m_output = a_in;}
    /// @}
};//end CommandAndOutput

class GDBMITuple ;
class GDBMIResult ;
class GDBMIValue ;
class GDBMIList ;
typedef SafePtr<GDBMIResult, ObjectRef, ObjectUnref> GDBMIResultSafePtr ;
typedef SafePtr<GDBMITuple, ObjectRef, ObjectUnref> GDBMITupleSafePtr ;
typedef SafePtr<GDBMIValue, ObjectRef, ObjectUnref> GDBMIValueSafePtr;
typedef SafePtr<GDBMIList, ObjectRef, ObjectUnref> GDBMIListSafePtr ;

class GDBMITuple : public Object {
    GDBMITuple (const GDBMITuple&) ;
    GDBMITuple& operator= (const GDBMITuple&) ;

    list<GDBMIResultSafePtr> m_content ;

public:

    GDBMITuple () {}
    virtual ~GDBMITuple () {}
    const list<GDBMIResultSafePtr>& content () const {return m_content;}
    void content (const list<GDBMIResultSafePtr> &a_in) {m_content = a_in;}
    void append (const GDBMIResultSafePtr &a_result)
    {
        m_content .push_back (a_result);
    }
    void clear () {m_content.clear ();}
};//end class GDBMITuple

/// A GDB/MI Value.
/// the syntax of a GDB/MI value is:
/// VALUE ==> CONST | TUPLE | LIST
/// In our case, CONST is a UString class, TUPLE is a GDBMITuple class and
/// LIST is a GDBMIList class.
/// please, read the GDB/MI output syntax documentation for more.
class GDBMIValue : public Object {
    GDBMIValue (const GDBMIValue&) ;
    GDBMIValue& operator= (const GDBMIValue&) ;
    boost::variant<UString, GDBMIListSafePtr, GDBMITupleSafePtr> m_content ;
    friend class GDBMIResult ;

    GDBMIValue () {}

public:
    enum Type {
        STRING_TYPE=0,
        LIST_TYPE,
        TUPLE_TYPE
    };

    GDBMIValue (const UString &a_str) {m_content = a_str ;}

    GDBMIValue (const GDBMIListSafePtr &a_list)
    {
        m_content = a_list ;
    }

    GDBMIValue (const GDBMITupleSafePtr &a_tuple)
    {
        m_content = a_tuple ;
    }

    Type content_type () const {return (Type) m_content.which ();}

    const UString& get_string_content ()
    {
        THROW_IF_FAIL (content_type () == STRING_TYPE) ;
        return boost::get<UString> (m_content) ;
    }

    const GDBMIListSafePtr get_list_content () const
    {
        THROW_IF_FAIL (content_type () == LIST_TYPE) ;
        return boost::get<GDBMIListSafePtr> (m_content) ;
    }
    GDBMIListSafePtr get_list_content ()
    {
        THROW_IF_FAIL (content_type () == LIST_TYPE) ;
        return boost::get<GDBMIListSafePtr> (m_content) ;
    }

    const GDBMITupleSafePtr get_tuple_content () const
    {
        THROW_IF_FAIL (content_type () == TUPLE_TYPE) ;
        THROW_IF_FAIL (boost::get<GDBMITupleSafePtr> (&m_content)) ;
        return boost::get<GDBMITupleSafePtr> (m_content) ;
    }
    GDBMITupleSafePtr get_tuple_content ()
    {
        THROW_IF_FAIL (content_type () == TUPLE_TYPE) ;
        THROW_IF_FAIL (boost::get<GDBMITupleSafePtr> (&m_content)) ;
        return boost::get<GDBMITupleSafePtr> (m_content) ;
    }


    const boost::variant<UString,GDBMIListSafePtr,GDBMITupleSafePtr>&
    content () const
    {
        return m_content;
    }
    void content
        (const boost::variant<UString,GDBMIListSafePtr,GDBMITupleSafePtr> &a_in)
    {
        m_content = a_in;
    }
};//end class value

/// A GDB/MI Result . This is the
/// It syntax looks like VARIABLE=VALUE,
/// where VALUE is a complex type.
class GDBMIResult : public Object {
    GDBMIResult (const GDBMIResult&) ;
    GDBMIResult& operator= (const GDBMIResult&) ;

    UString m_variable ;
    GDBMIValueSafePtr m_value ;

public:

    GDBMIResult () {}
    GDBMIResult (const UString &a_variable,
                 const GDBMIValueSafePtr &a_value) :
        m_variable (a_variable),
        m_value (a_value)
    {}
    virtual ~GDBMIResult () {}
    const UString& variable () const {return m_variable;}
    void variable (const UString& a_in) {m_variable = a_in;}
    const GDBMIValueSafePtr& value () const {return m_value;}
    void value (const GDBMIValueSafePtr &a_in) {m_value = a_in;}
};//end class GDBMIResult

/// A GDB/MI LIST. It can be a list of either GDB/MI Result or GDB/MI Value.
class GDBMIList : public Object {
    GDBMIList (const GDBMIList &) ;
    GDBMIList& operator= (const GDBMIList &) ;

    //boost::variant<list<GDBMIResultSafePtr>, list<GDBMIValueSafePtr> > m_content ;
    list<boost::variant<GDBMIResultSafePtr, GDBMIValueSafePtr> >  m_content ;
    bool m_empty ;

public:
    enum ContentType {
        RESULT_TYPE=0,
        VALUE_TYPE,
        UNDEFINED_TYPE
    };

    GDBMIList () :
        m_empty (true)
    {}

    GDBMIList (const GDBMITupleSafePtr &a_tuple) :
        m_empty (false)
    {
        GDBMIValueSafePtr value (new GDBMIValue (a_tuple)) ;
        //list<GDBMIValueSafePtr> value_list ; value_list.push_back (value) ;
        //list<GDBMIValueSafePtr> value_list ; value_list.push_back (value) ;
        //m_content = value_list ;
        m_content.push_back (value) ;
    }

    GDBMIList (const UString &a_str) :
        m_empty (false)
    {
        GDBMIValueSafePtr value (new GDBMIValue (a_str)) ;
        //list<GDBMIValueSafePtr> list ;
        //list.push_back (value) ;
        //m_content = list ;
        m_content.push_back (value) ;
    }

    GDBMIList (const GDBMIResultSafePtr &a_result) :
        m_empty (false)
    {
        //list<GDBMIResultSafePtr> list ;
        //list.push_back (a_result) ;
        //m_content = list ;
        m_content.push_back (a_result) ;
    }

    GDBMIList (const GDBMIValueSafePtr &a_value) :
        m_empty (false)
    {
        //list<GDBMIValueSafePtr> list ;
        //list.push_back (a_value) ;
        //m_content = list ;
        m_content.push_back (a_value) ;
    }

    virtual ~GDBMIList () {}
    ContentType content_type () const
    {
        if (m_content.empty ()) {
            return UNDEFINED_TYPE ;
        }
        return (ContentType) m_content.front ().which ();
    }

    bool empty () const {return m_empty;}

    void append (const GDBMIResultSafePtr &a_result)
    {
        THROW_IF_FAIL (a_result) ;
        if (!m_content.empty ()) {
            THROW_IF_FAIL (m_content.front ().which () == RESULT_TYPE) ;
        }
        m_content.push_back (a_result) ;
        m_empty = false ;
    }
    void append (const GDBMIValueSafePtr &a_value)
    {
        THROW_IF_FAIL (a_value) ;
        if (!m_content.empty ()) {
            THROW_IF_FAIL (m_content.front ().which () == VALUE_TYPE) ;
        }
        m_content.push_back (a_value) ;
        m_empty = false ;
    }

    void get_result_content (list<GDBMIResultSafePtr> &a_list) const
    {
        THROW_IF_FAIL (!empty () && content_type () == RESULT_TYPE) ;
        list<boost::variant<GDBMIResultSafePtr,GDBMIValueSafePtr> >::const_iterator it;
        for (it= m_content.begin () ; it!= m_content.end () ; ++it) {
            a_list.push_back (boost::get<GDBMIResultSafePtr> (*it)) ;
        }
    }

    void get_value_content (list<GDBMIValueSafePtr> &a_list) const
    {
        THROW_IF_FAIL (!empty () && content_type () == VALUE_TYPE) ;
        list<boost::variant<GDBMIResultSafePtr,GDBMIValueSafePtr> >::const_iterator it;
        for (it= m_content.begin () ; it!= m_content.end () ; ++it) {
            a_list.push_back (boost::get<GDBMIValueSafePtr> (*it)) ;
        }
    }
};//end class GDBMIList

//******************************************
//<gdbmi datastructure streaming operators>
//******************************************

ostream& operator<< (ostream &a_out, const GDBMIValueSafePtr &a_val) ;

ostream&
operator<< (ostream &a_out, const GDBMIResultSafePtr &a_result)
{
    if (!a_result) {
        a_out << "<result nilpointer/>" ;
        return a_out ;
    }
    a_out << "<result variable='" ;
    a_out << Glib::locale_from_utf8 (a_result->variable ()) << "'>" ;
    a_out << a_result->value () ;
    a_out << "</result>" ;
    return a_out ;
}

ostream&
operator<< (ostream &a_out, const GDBMITupleSafePtr &a_tuple)
{
    if (!a_tuple) {
        a_out << "<tuple nilpointer/>" ;
        return a_out ;
    }
    list<GDBMIResultSafePtr>::const_iterator iter ;
    a_out << "<tuple>"  ;
    for (iter=a_tuple->content ().begin () ; iter!=a_tuple->content ().end(); ++iter) {
        a_out  << *iter ;
    }
    a_out  << "</tuple>";
    return a_out ;
}

ostream&
operator<< (ostream &a_out, const GDBMIListSafePtr a_list)
{
    if (!a_list) {
        a_out << "<list nilpointer/>" ;
        return a_out ;
    }
    if (a_list->content_type () == GDBMIList::RESULT_TYPE) {
        a_out << "<list type='result'>" ;
        list<GDBMIResultSafePtr>::const_iterator iter ;
        list<GDBMIResultSafePtr> result_list ;
        a_list->get_result_content (result_list) ;
        for (iter = result_list.begin () ;
             iter != result_list.end ();
             ++iter) {
            a_out << *iter ;
        }
        a_out << "</list>" ;
    } else if (a_list->content_type () == GDBMIList::VALUE_TYPE) {
        a_out << "<list type='value'>" ;
        list<GDBMIValueSafePtr>::const_iterator iter ;
        list<GDBMIValueSafePtr> value_list;
        a_list->get_value_content (value_list) ;
        for (iter = value_list.begin () ;
             iter != value_list.end ();
             ++iter) {
            a_out << *iter ;
        }
        a_out << "</list>" ;
    } else {
        THROW_IF_FAIL ("assert not reached") ;
    }
    return a_out ;
}
ostream&
operator<< (ostream &a_out, const GDBMIValueSafePtr &a_val)
{
    if (!a_val) {
        a_out << "<value nilpointer/>" ;
        return a_out ;
    }

    switch (a_val->content_type ()) {
        case GDBMIValue::TUPLE_TYPE :
            a_out << "<value type='tuple'>"
                  << a_val->get_tuple_content ()
                  << "</value>";
            break ;
        case GDBMIValue::LIST_TYPE :
            a_out << "<value type='list'>\n" << a_val->get_list_content ()<< "</value>" ;
            break ;
        case GDBMIValue::STRING_TYPE:
            a_out << "<value type='string'>"
                  << Glib::locale_from_utf8 (a_val->get_string_content ())
                  << "</value>" ;
            break ;
    }
    return a_out ;
}
//******************************************
//</gdbmi datastructure streaming operators>
//******************************************

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

    sigc::signal<void, const vector<IDebugger::Frame>& >&
                                                frames_listed_signal () const ;

    sigc::signal<void, const map<int,
                                 list<IDebugger::VariableSafePtr> >&>&
                                        frames_params_listed_signal () const;

    sigc::signal<void, int, IDebugger::Frame&> & current_frame_signal () const  ;

    sigc::signal<void, const list<VariableSafePtr>& >&
                        local_variables_listed_signal () const ;

    sigc::signal<void, int>& got_proc_info_signal () const  ;

    sigc::signal<void>& running_signal () const ;

    sigc::signal<void>& program_finished_signal () const ;
    //*************
    //</signals>
    //*************

    //***************
    //<signal handlers>
    //***************

    void on_debugger_stdout_signal (CommandAndOutput &a_cao) ;
    void on_got_proc_info_signal (int a_pid) ;
    //***************
    //</signal handlers>
    //***************

    void init () ;
    map<UString, UString>& properties () ;
    void set_event_loop_context (const Glib::RefPtr<Glib::MainContext> &) ;
    void run_loop_iterations (int a_nb_iters) ;
    bool stop (bool a_run_event_loops=false)  ;
    void execute_command (const IDebugger::Command &a_command) ;
    bool queue_command (const IDebugger::Command &a_command,
                        bool a_run_event_loops=false) ;
    bool busy () const ;

    void load_program (const vector<UString> &a_argv,
                       const vector<UString> &a_source_search_dirs,
                       bool a_run_event_loops) ;

    void load_core_file (const UString &a_prog_file,
                         const UString &a_core_path,
                         bool a_run_event_loop);

    bool attach_to_program (unsigned int a_pid) ;

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

    void list_frames (bool a_run_event_loops) ;

    void list_frames_arguments (int a_low_frame,
                                int a_high_frame,
                                bool a_run_event_loops) ;

    void list_local_variables (bool a_run_event_loops) ;

    bool extract_proc_info (Output &a_output, int &a_proc_pid) ;

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
    Glib::Pid target_pid ;
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

    mutable sigc::signal<void, const vector<IDebugger::Frame>& >
                                                    frames_listed_signal ;

    mutable sigc::signal<void, const map<int,
                                         list<IDebugger::VariableSafePtr> >&>
                                                frames_params_listed_signal ;

    mutable sigc::signal<void, int, IDebugger::Frame&> current_frame_signal ;

    mutable sigc::signal<void, const list<VariableSafePtr>& >
                                    local_variables_listed_signal  ;

    sigc::signal<void, int> got_proc_info_signal ;

    mutable sigc::signal<void> running_signal ;

    mutable sigc::signal<void> program_finished_signal ;

    //***********************
    //</GDBEngine attributes>
    //************************

    void set_event_loop_context (const Glib::RefPtr<Glib::MainContext> &a_ctxt)
    {
        loop_context = a_ctxt ;
    }

    void run_loop_iterations_real (int a_nb_iters)
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
        LOG_D ("<debuggeroutput>\n" << a_buf << "\n</debuggeroutput>",
               GDBMI_OUTPUT_DOMAIN) ;

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
        cwd ("."), gdb_pid (0), target_pid (0),
        gdb_stdout_fd (0), gdb_stderr_fd (0),
        gdb_master_pty_fd (0),
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


    void set_communication_charset (const string &a_charset)
    {
        gdb_stdout_channel->set_encoding (a_charset) ;
        gdb_stderr_channel->set_encoding (a_charset) ;
        gdb_master_pty_channel->set_encoding (a_charset) ;
    }

    bool launch_gdb_real (const vector<UString> a_argv)
    {
        RETURN_VAL_IF_FAIL (launch_program (a_argv,
                                            gdb_pid,
                                            gdb_master_pty_fd,
                                            gdb_stdout_fd,
                                            gdb_stderr_fd),
                            false) ;

        RETURN_VAL_IF_FAIL (gdb_pid, false) ;

        gdb_stdout_channel = Glib::IOChannel::create_from_fd (gdb_stdout_fd) ;
        THROW_IF_FAIL (gdb_stdout_channel) ;

        gdb_stderr_channel = Glib::IOChannel::create_from_fd (gdb_stderr_fd) ;
        THROW_IF_FAIL (gdb_stderr_channel) ;

        gdb_master_pty_channel = Glib::IOChannel::create_from_fd
                                                            (gdb_master_pty_fd) ;
        THROW_IF_FAIL (gdb_master_pty_channel) ;

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

        return true ;
    }


    bool launch_gdb (const vector<UString> &a_source_search_dirs,
                     const vector<UString> &a_gdb_options,
                     const UString a_prog="")
    {
        if (is_gdb_running ()) {
            kill_gdb () ;
        }
        argv.clear () ;
        argv.push_back (env::get_gdb_program ()) ;
        argv.push_back ("--interpreter=mi2") ;
        if (!a_gdb_options.empty ()) {
            for (vector<UString>::const_iterator it = a_gdb_options.begin ();
                 it != a_gdb_options.end ();
                 ++it) {
                argv.push_back (*it) ;
            }
        }
        if (a_prog != "") {
            argv.push_back (a_prog) ;
        }

        source_search_dirs = a_source_search_dirs;
        return launch_gdb_real (argv) ;
    }

    bool launch_gdb_and_set_args (const vector<UString> &a_source_search_dirs,
                                  const vector<UString> &a_prog_args,
                                  const vector<UString> a_gdb_options)
    {
        bool result (false);
        result = launch_gdb (a_source_search_dirs, a_gdb_options, a_prog_args[0]);
        if (!result) {return false;}

        if (!a_prog_args.empty ()) {
            UString args ;
            for (vector<UString>::size_type i=1; i < a_prog_args.size () ; ++i) {
                args += a_prog_args[i] + " " ;
            }

            if (args != "") {
                return issue_command (Command ("set args " + args)) ;
            }
        }
        return true;
    }

    bool launch_gdb_on_core_file (const UString &a_prog_path,
                                  const UString &a_core_path)
    {
        vector<UString> argv ;
        argv.push_back (env::get_gdb_program ()) ;
        argv.push_back ("--interpreter=mi2") ;
        argv.push_back (a_prog_path) ;
        argv.push_back (a_core_path) ;
        return launch_gdb_real (argv) ;
    }

    bool issue_command (const IDebugger::Command &a_command,
                        bool a_run_event_loops=false)
    {
        if (!gdb_master_pty_channel) {
            return false ;
        }

        LOG_D ("issuing command: '" << a_command.value () << "'",
               NMV_DEFAULT_DOMAIN) ;

        if (gdb_master_pty_channel->write
                (a_command.value () + "\n") == Glib::IO_STATUS_NORMAL) {
            gdb_master_pty_channel->flush () ;
            if (a_run_event_loops) {
                run_loop_iterations_real (-1) ;
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
                kill_gdb () ;
                gdb_died_signal.emit () ;
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
                kill_gdb () ;
                gdb_died_signal.emit () ;
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

    //*****************************************************************
    //                 <GDB/MI parsing functions>
    //*****************************************************************

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

    /// parse a GDB/MI Tuple is a actualy a set of name=value constructs,
    /// where 'value' can be quite complicated.
    /// Look at the GDB/MI output syntax for more.
    bool parse_tuple (const UString &a_input,
                      UString::size_type a_from,
                      UString::size_type &a_to,
                      GDBMITupleSafePtr &a_tuple)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
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

        GDBMITupleSafePtr tuple ;
        GDBMIResultSafePtr result ;

        for (;;) {
            if (parse_result (a_input, cur, cur, result)) {
                THROW_IF_FAIL (result) ;
                SKIP_WS (a_input, cur, cur) ;
                CHECK_END (a_input, cur, end) ;
                if (!tuple) {
                    tuple = GDBMITupleSafePtr (new GDBMITuple) ;
                    THROW_IF_FAIL (tuple) ;
                }
                tuple->append (result) ;
                if (a_input[cur] == ',') {
                    ++cur ;
                    CHECK_END (a_input, cur, end) ;
                    SKIP_WS (a_input, cur, cur) ;
                    continue ;
                }
                if (a_input[cur] == '}') {
                    ++cur ;
                }
            } else {
                LOG_PARSING_ERROR (a_input, cur) ;
                return false ;
            }
            LOG_D ("getting out at char '"
                   << (char)a_input[cur]
                   << "', at offset '"
                   << (int)cur
                   << "' for text >>>"
                   << a_input
                   << "<<<",
                   GDBMI_FRAME_PARSING_DOMAIN) ;
            break ;
        }

        SKIP_WS (a_input, cur, cur) ;
        a_to = cur ;
        a_tuple = tuple ;
        return true ;
    }

    /// Parse a GDB/MI LIST. A list is either a list of GDB/MI Results
    /// or a list of GDB/MI Values.
    /// Look at the GDB/MI output syntax documentation for more.
    bool parse_list (const UString &a_input,
                     UString::size_type a_from,
                     UString::size_type &a_to,
                     GDBMIListSafePtr &a_list)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        GDBMIListSafePtr return_list ;
        if (a_input[cur] != '[') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        CHECK_END (a_input, cur + 1, end) ;
        if (a_input[cur + 1] == ']') {
            a_list = GDBMIListSafePtr (new GDBMIList);
            cur += 2;
            a_to = cur ;
            return true ;
        }

        ++cur ;
        CHECK_END (a_input, cur, end) ;
        SKIP_WS (a_input, cur, cur) ;

        GDBMIValueSafePtr value ;
        GDBMIResultSafePtr result ;
        if ((isalpha (a_input[cur]) || a_input[cur] == '_')
             && parse_result (a_input, cur, cur, result)) {
            CHECK_END (a_input, cur, end) ;
            THROW_IF_FAIL (result) ;
            return_list = GDBMIListSafePtr (new GDBMIList (result)) ;
            for (;;) {
                if (a_input[cur] == ',') {
                    ++cur ;
                    CHECK_END (a_input, cur, end) ;
                    result = NULL ;
                    if (parse_result (a_input, cur, cur, result)) {
                        THROW_IF_FAIL (result) ;
                        return_list->append (result) ;
                        continue ;
                    }
                }
                break ;
            }
        } else if (parse_value (a_input, cur, cur, value)) {
            CHECK_END (a_input, cur, end) ;
            THROW_IF_FAIL (value);
            return_list =
                    GDBMIListSafePtr (new GDBMIList (value)) ;
            for (;;) {
                if (a_input[cur] == ',') {
                    ++cur ;
                    CHECK_END (a_input, cur, end) ;
                    value = NULL ;
                    if (parse_value (a_input, cur, cur, value)) {
                        THROW_IF_FAIL (value) ;
                        return_list->append (value) ;
                        continue ;
                    }
                }
                break ;
            }
        } else {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        if (a_input[cur] != ']') {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        ++cur ;

        a_to = cur ;
        a_list = return_list ;
        return true ;
    }

    /// parse a GDB/MI Result data structure.
    /// A result basically has the form:
    /// variable=value.
    /// Beware value is more complicated than what it looks like :-)
    /// Look at the GDB/MI spec for more.
    bool parse_result (const UString &a_input,
                       UString::size_type a_from,
                       UString::size_type &a_to,
                       GDBMIResultSafePtr &a_value)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
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

        GDBMIValueSafePtr value;
        if (!parse_value (a_input, cur, cur, value)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        THROW_IF_FAIL (value) ;

        GDBMIResultSafePtr result (new GDBMIResult (variable, value)) ;
        THROW_IF_FAIL (result) ;
        a_to = cur ;
        a_value = result ;
        return true ;
    }

    /// \brief parse a GDB/MI value.
    /// GDB/MI value type is defined as:
    /// VALUE ==> CONST | TUPLE | LIST
    /// CONSTis a string, basically. Look at parse_string() for more.
    /// TUPLE is a GDB/MI tuple. Look at parse_tuple() for more.
    /// LIST is a GDB/MI list. It is either  a list of GDB/MI Result or
    /// a list of GDB/MI value. Yeah, that can be recursive ...
    /// To parse a GDB/MI list, we use parse_list() defined above.
    /// You can look at the GDB/MI output syntax for more.
    /// \param a_input the input string to parse
    /// \param a_from where to start parsing from
    /// \param a_to (out parameter) a pointer to the current char,
    /// after the parsing.
    //// \param a_value the result of the parsing.
    bool parse_value (const UString &a_input,
                      UString::size_type a_from,
                      UString::size_type &a_to,
                      GDBMIValueSafePtr &a_value)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        GDBMIValueSafePtr value ;
        if (a_input[cur] == '"') {
            UString const_string ;
            if (parse_c_string (a_input, cur, cur, const_string)) {
                value = GDBMIValueSafePtr (new GDBMIValue (const_string)) ;
            }
        } else if (a_input[cur] == '{') {
            GDBMITupleSafePtr tuple ;
            if (parse_tuple (a_input, cur, cur, tuple)) {
                THROW_IF_FAIL (tuple) ;
                value = GDBMIValueSafePtr (new GDBMIValue (tuple)) ;
            }
        } else if (a_input[cur] == '[') {
            GDBMIListSafePtr list ;
            if (parse_list (a_input, cur, cur, list)) {
                THROW_IF_FAIL (list) ;
                value = GDBMIValueSafePtr (new GDBMIValue (list)) ;
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

    bool parse_frame (const UString &a_input,
                      UString::size_type a_from,
                      UString::size_type &a_to,
                      IDebugger::Frame &a_frame,
                      int &a_level)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        GDBMIResultSafePtr result ;
        if (!parse_result (a_input, cur, cur, result)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        THROW_IF_FAIL (result) ;
        CHECK_END (a_input, cur, end) ;

        if (result->variable () != "frame") {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        if (!result->value ()
            ||result->value ()->content_type ()
                    != GDBMIValue::TUPLE_TYPE) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        GDBMITupleSafePtr result_value_tuple =
                                    result->value ()->get_tuple_content () ;
        if (!result_value_tuple) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        list<GDBMIResultSafePtr>::const_iterator res_it ;
        GDBMIResultSafePtr tmp_res ;
        Frame frame ;
        UString name, value, level_str;
        for (res_it = result_value_tuple->content ().begin () ;
             res_it != result_value_tuple->content ().end ();
             ++res_it) {
            if (!(*res_it)) {continue;}
            tmp_res = *res_it ;
            if (!tmp_res->value ()
                ||tmp_res->value ()->content_type () != GDBMIValue::STRING_TYPE) {
                continue ;
            }
            name = tmp_res->variable () ;
            value = tmp_res->value ()->get_string_content () ;
            if (name == "level") {
                level_str = value ;
            } else if (name == "addr") {
                frame.address (value) ;
            } else if (name == "func") {
                frame.function (value) ;
            }
        }
        int level=-1 ;
        if (level_str != "") {
            level = atoi (level_str.c_str ()) ;
        }

        a_level = level ;
        a_frame = frame ;
        a_to = cur ;
        return true ;
    }

    /// parse a callstack as returned by the gdb/mi command:
    /// -stack-list-frames
    bool parse_call_stack (const UString &a_input,
                           const UString::size_type a_from,
                           UString::size_type &a_to,
                           vector<IDebugger::Frame> &a_stack)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        GDBMIResultSafePtr result ;
        if (!parse_result (a_input, cur, cur, result)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        THROW_IF_FAIL (result) ;
        CHECK_END (a_input, cur, end) ;

        if (result->variable () != "stack") {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        if (!result->value ()
            ||result->value ()->content_type ()
                    != GDBMIValue::LIST_TYPE) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        GDBMIListSafePtr result_value_list =
            result->value ()->get_list_content () ;
        if (!result_value_list) {
            a_to = cur ;
            a_stack.clear () ;
            return true ;
        }

        if (result_value_list->content_type () != GDBMIList::RESULT_TYPE) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        list<GDBMIResultSafePtr> result_list  ;
        result_value_list->get_result_content (result_list) ;

        GDBMITupleSafePtr frame_tuple ;
        vector<IDebugger::Frame> stack ;
        list<GDBMIResultSafePtr>::const_iterator iter, frame_part_iter ;
        UString value ;
        for (iter = result_list.begin (); iter != result_list.end () ; ++iter) {
            if (!(*iter)) {continue;}
            THROW_IF_FAIL ((*iter)->value ()
                           && (*iter)->value ()->content_type ()
                           == GDBMIValue::TUPLE_TYPE) ;

            frame_tuple = (*iter)->value ()->get_tuple_content () ;
            THROW_IF_FAIL (frame_tuple) ;
            IDebugger::Frame frame ;
            for (frame_part_iter = frame_tuple->content ().begin ();
                 frame_part_iter != frame_tuple->content ().end ();
                 ++frame_part_iter) {
                THROW_IF_FAIL ((*frame_part_iter)->value ()) ;
                value = (*frame_part_iter)->value ()->get_string_content () ;
                if ((*frame_part_iter)->variable () == "addr") {
                    frame.address (value) ;
                } else if ((*frame_part_iter)->variable () == "func") {
                    frame.function (value) ;
                } else if ((*frame_part_iter)->variable () == "file") {
                    frame.file_name (value) ;
                } else if ((*frame_part_iter)->variable () == "fullname") {
                    frame.file_full_name (value) ;
                } else if ((*frame_part_iter)->variable () == "line") {
                    frame.line (atol (value.c_str ())) ;
                }
            }
            THROW_IF_FAIL (frame.address () != "") ;
            stack.push_back (frame) ;
            frame.clear () ;
        }
        a_stack = stack ;
        a_to = cur ;
        return true ;
    }

    /// Parse the arguments of the call stack.
    /// The call stack arguments is the result of the
    /// GDB/MI command -stack-list-arguments 1.
    /// It is basically the arguments of the functions of the call stack.
    /// See the GDB/MI documentation for more.
    bool parse_stack_arguments
                        (const UString &a_input,
                         UString::size_type a_from,
                         UString::size_type &a_to,
                         map<int, list<IDebugger::VariableSafePtr> > &a_params)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        if (a_input.compare (cur, 12, "stack-args=[")) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        GDBMIResultSafePtr gdbmi_result ;
        if (!parse_result (a_input, cur, cur, gdbmi_result)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        THROW_IF_FAIL (gdbmi_result
                       && gdbmi_result->variable () == "stack-args") ;

        if (!gdbmi_result->value ()
            || gdbmi_result->value ()->content_type ()
                != GDBMIValue::LIST_TYPE) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        GDBMIListSafePtr gdbmi_list =
            gdbmi_result->value ()->get_list_content () ;
        if (!gdbmi_list) {
            a_to = cur ;
            a_params.clear () ;
            return true ;
        }

        if (gdbmi_list->content_type () != GDBMIList::RESULT_TYPE) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        list<GDBMIResultSafePtr> frames_params_list ;
        gdbmi_list->get_result_content (frames_params_list) ;
        LOG_D ("number of frames: " << (int) frames_params_list.size (),
               GDBMI_FRAME_PARSING_DOMAIN) ;

        list<GDBMIResultSafePtr>::const_iterator frames_iter,
                                            params_records_iter,
                                            params_iter;
        map<int, list<IDebugger::VariableSafePtr> > all_frames_args;
        //walk through the list of frames
        //each frame is a tuple of the form:
        //{level="2", args=[list-of-arguments]}
        for (frames_iter = frames_params_list.begin () ;
             frames_iter != frames_params_list.end ();
             ++frames_iter) {
            if (!(*frames_iter)) {
                LOG_D ("Got a null frmae, skipping", GDBMI_FRAME_PARSING_DOMAIN) ;
                continue;
            }
            THROW_IF_FAIL ((*frames_iter)->variable () != "stack") ;
            THROW_IF_FAIL ((*frames_iter)->value ()
                            && (*frames_iter)->value ()->content_type ()
                            == GDBMIValue::TUPLE_TYPE)

            //params_record is a tuple that has the form:
            //{level="2", args=[list-of-arguments]}
            GDBMITupleSafePtr params_record ;
            params_record = (*frames_iter)->value ()->get_tuple_content () ;
            THROW_IF_FAIL (params_record) ;

            //walk through the tuple {level="2", args=[list-of-arguments]}
            int cur_frame_level=-1 ;
            for (params_records_iter = params_record->content ().begin ();
                 params_records_iter != params_record->content ().end () ;
                 ++params_records_iter) {
                THROW_IF_FAIL ((*params_records_iter)->value ()) ;

                if ((*params_records_iter)->variable () == "level") {
                    THROW_IF_FAIL
                    ((*params_records_iter)->value ()
                     && (*params_records_iter)->value ()->content_type ()
                     == GDBMIValue::STRING_TYPE) ;
                    cur_frame_level = atoi
                        ((*params_records_iter)->value
                             ()->get_string_content ().c_str ());
                    LOG_D ("frame level '" << (int) cur_frame_level << "'",
                           GDBMI_FRAME_PARSING_DOMAIN) ;
                } else if ((*params_records_iter)->variable () == "args") {
                    //this gdbmi result is of the form:
                    //args=[{name="foo0", value="bar0"},
                    //      {name="foo1", bar="bar1"}]

                    THROW_IF_FAIL
                    ((*params_records_iter)->value ()
                     && (*params_records_iter)->value ()->get_list_content ()) ;

                    GDBMIListSafePtr arg_list =
                        (*params_records_iter)->value ()->get_list_content () ;
                    list<GDBMIValueSafePtr>::const_iterator args_as_value_iter ;
                    list<IDebugger::VariableSafePtr> cur_frame_args;
                    if (arg_list && !(arg_list->empty ())) {
                        LOG_D ("arg list is *not* empty for frame level '"
                               << (int)cur_frame_level,
                               GDBMI_FRAME_PARSING_DOMAIN) ;
                        //walk each parameter.
                        //Each parameter is a tuple (in a value)
                        list<GDBMIValueSafePtr> arg_as_value_list ;
                        arg_list->get_value_content (arg_as_value_list);
                        LOG_D ("arg list size: "
                               << (int)arg_as_value_list.size (),
                               GDBMI_FRAME_PARSING_DOMAIN) ;
                        for (args_as_value_iter=arg_as_value_list.begin();
                             args_as_value_iter!=arg_as_value_list.end();
                             ++args_as_value_iter) {
                            if (!*args_as_value_iter) {
                                LOG_D ("got NULL arg, skipping",
                                       GDBMI_FRAME_PARSING_DOMAIN) ;
                                continue;
                            }
                            GDBMITupleSafePtr args =
                                (*args_as_value_iter)->get_tuple_content () ;
                            list<GDBMIResultSafePtr>::const_iterator arg_iter ;
                            IDebugger::VariableSafePtr parameter
                                                    (new IDebugger::Variable) ;
                            THROW_IF_FAIL (parameter) ;
                            THROW_IF_FAIL (args) ;
                            //walk the name and value of the parameter
                            for (arg_iter = args->content ().begin ();
                                 arg_iter != args->content ().end ();
                                 ++arg_iter) {
                                THROW_IF_FAIL (*arg_iter) ;
                                if ((*arg_iter)->variable () == "name") {
                                    THROW_IF_FAIL ((*arg_iter)->value ()) ;
                                    parameter->name
                                    ((*arg_iter)->value()->get_string_content());
                                } else if ((*arg_iter)->variable () == "value") {
                                    THROW_IF_FAIL ((*arg_iter)->value ()) ;
                                    parameter->value
                                        ((*arg_iter)->value()->get_string_content()) ;
                                } else {
                                    THROW ("should not reach this line") ;
                                }
                            }
                            LOG_D ("pushing arg '"
                                   <<parameter->name()<<"'='"<<parameter->value() <<"'"
                                   <<" for frame level='"
                                   <<(int)cur_frame_level
                                   <<"'",
                                   GDBMI_FRAME_PARSING_DOMAIN) ;
                            cur_frame_args.push_back (parameter) ;
                        }
                    } else {
                        LOG_D ("arg list is empty for frame level '"
                               << (int)cur_frame_level,
                               GDBMI_FRAME_PARSING_DOMAIN) ;
                    }
                    THROW_IF_FAIL (cur_frame_level >= 0) ;
                    LOG_D ("cur_frame_level: '"
                           << (int) cur_frame_level
                           << "', NB Params: "
                           << (int) cur_frame_args.size (),
                           NMV_DEFAULT_DOMAIN) ;
                    all_frames_args[cur_frame_level] = cur_frame_args ;
                } else {
                    LOG_PARSING_ERROR (a_input, cur) ;
                    return false ;
                }
            }

        }

        a_to = cur ;
        a_params = all_frames_args ;
        LOG_D ("number of frames parsed: "
               << (int)a_params.size (),
               NMV_DEFAULT_DOMAIN) ;
        return true;
    }

    /// parse a list of local variables as returned by
    /// the GDBMI command -stack-list-locals 2
    bool parse_local_var_list
                        (const UString &a_input,
                         UString::size_type a_from,
                         UString::size_type &a_to,
                         list<IDebugger::VariableSafePtr> &a_vars)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

        if (a_input.compare (cur, 8, "locals=[")) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        GDBMIResultSafePtr gdbmi_result ;
        if (!parse_result (a_input, cur, cur, gdbmi_result)) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }
        THROW_IF_FAIL (gdbmi_result
                       && gdbmi_result->variable () == "locals") ;

        if (!gdbmi_result->value ()
            || gdbmi_result->value ()->content_type ()
                != GDBMIValue::LIST_TYPE) {
            LOG_PARSING_ERROR (a_input, cur) ;
            return false ;
        }

        GDBMIListSafePtr gdbmi_list =
            gdbmi_result->value ()->get_list_content () ;
        if (!gdbmi_list) {
            a_to = cur ;
            a_vars.clear () ;
            return true ;
        }
        RETURN_VAL_IF_FAIL (gdbmi_list->content_type () == GDBMIList::VALUE_TYPE,
                            false);

        std::list<GDBMIValueSafePtr> gdbmi_value_list ;
        gdbmi_list->get_value_content (gdbmi_value_list) ;
        RETURN_VAL_IF_FAIL (!gdbmi_value_list.empty (), false) ;

        std::list<IDebugger::VariableSafePtr> variables ;
        std::list<GDBMIValueSafePtr>::const_iterator value_iter;
        std::list<GDBMIResultSafePtr> tuple_content ;
        std::list<GDBMIResultSafePtr>::const_iterator tuple_iter ;
        for (value_iter = gdbmi_value_list.begin () ;
             value_iter != gdbmi_value_list.end () ;
             ++value_iter) {
            if (!(*value_iter)) {continue;}
            if ((*value_iter)->content_type () != GDBMIValue::TUPLE_TYPE) {
                LOG_ERROR_D ("list of tuple should contain only tuples",
                             GDBMI_FRAME_PARSING_DOMAIN) ;
                continue ;
            }
            GDBMITupleSafePtr gdbmi_tuple = (*value_iter)->get_tuple_content ();
            RETURN_VAL_IF_FAIL (gdbmi_tuple, false) ;
            RETURN_VAL_IF_FAIL (!gdbmi_tuple->content ().empty (), false) ;

            tuple_content.clear () ;
            tuple_content = gdbmi_tuple->content () ;
            RETURN_VAL_IF_FAIL (!tuple_content.empty (), false) ;
            IDebugger::VariableSafePtr variable (new Variable) ;
            for (tuple_iter = tuple_content.begin () ;
                 tuple_iter != tuple_content.end ();
                 ++tuple_iter) {
                if (!(*tuple_iter)) {
                    LOG_ERROR_D ("got and empty tuple member",
                                 GDBMI_FRAME_PARSING_DOMAIN) ;
                    continue ;
                }

                if (!(*tuple_iter)->value ()
                    ||(*tuple_iter)->value ()->content_type ()
                        != GDBMIValue::STRING_TYPE) {
                    LOG_ERROR_D ("Got a tuple member which value is not a string",
                                 GDBMI_FRAME_PARSING_DOMAIN) ;
                    continue ;
                }

                UString variable_str = (*tuple_iter)->variable () ;
                UString value_str =
                            (*tuple_iter)->value ()->get_string_content () ;
                if (variable_str == "name") {
                    variable->name (value_str) ;
                } else if (variable_str == "type") {
                    variable->type (value_str) ;
                } else if (variable_str == "value") {
                    variable->value (value_str) ;
                } else {
                    LOG_ERROR_D ("got an unknown tuple member with name: '"
                                 << variable_str << "'",
                                 GDBMI_FRAME_PARSING_DOMAIN)
                    continue ;
                }

            }
            variables.push_back (variable) ;
        }

        LOG_D ("got '" << (int)variables.size () << "' variables",
               GDBMI_FRAME_PARSING_DOMAIN) ;

        a_vars = variables ;
        a_to = cur ;
        return true;
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
                              UString::size_type a_from,
                              UString::size_type &a_to,
                              map<UString, UString> a_args)
    {
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
        UString::size_type cur = a_from, end = a_input.size () ;
        CHECK_END (a_input, cur, end) ;

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
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
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


    /// parses a string that has the form:
    /// \"blah\"
    bool parse_c_string (const UString &a_input,
            UString::size_type a_from,
            UString::size_type &a_to,
            UString &a_c_string)
    {
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
                    str_end = cur - 1 ;
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
        LOG_FUNCTION_SCOPE_NORMAL_D (GDBMI_FRAME_PARSING_DOMAIN) ;
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

    /// parse a GDB/MI result record.
    /// a result record is the result of the command that has been issued right
    /// before.
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

fetch_gdbmi_result:
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
                    vector<IDebugger::Frame> call_stack ;
                    if (!parse_call_stack (a_input, cur, cur, call_stack)) {
                        LOG_PARSING_ERROR (a_input, cur) ;
                        return false ;
                    }
                    result_record.call_stack (call_stack) ;
                    LOG_D ("parsed a call stack of depth: "
                           << (int) call_stack.size (),
                           GDBMI_FRAME_PARSING_DOMAIN) ;
                    vector<IDebugger::Frame>::iterator frame_iter ;
                    for (frame_iter = call_stack.begin () ;
                         frame_iter != call_stack.end ();
                         ++frame_iter) {
                        LOG_D ("function-name: " << frame_iter->function (),
                               GDBMI_FRAME_PARSING_DOMAIN) ;
                    }
                } else if (!a_input.compare (cur, 7, "frame={")) {
                    IDebugger::Frame frame ;
                    int level=-1 ;
                    if (!parse_frame (a_input, cur, cur, frame, level)) {
                        LOG_PARSING_ERROR (a_input, cur) ;
                    } else {
                        LOG_D ("parsed result", GDBMI_FRAME_PARSING_DOMAIN) ;
                        THROW_IF_FAIL (level != -1) ;
                        //this is a hack in the model used thoughout the
                        //GDBEngine code. Normally, output handlers
                        //are responsible of filtering the parsed
                        //result record and fire the right signal based
                        //on the result of the filtering. Here, I find
                        //that gdb is not very "consistent" in the way
                        //it handles core file stack trace listing, compared
                        //to stack trace listing in "normal operation".
                        //That's why I shunt the output handler scheme.
                        //Otherwise, I don't know how  I could add this
                        //frame level information the the parse result record
                        //without making it look ugly.
                        //So I fire the signal directly from here.
                        current_frame_signal.emit (level, frame) ;
                    }
                } else if (!a_input.compare (cur, 7, "depth=\"")) {
                    GDBMIResultSafePtr result ;
                    parse_result (a_input, cur, cur, result) ;
                    THROW_IF_FAIL (result) ;
                    LOG_D ("parsed result", GDBMI_FRAME_PARSING_DOMAIN) ;
                } else if (!a_input.compare (cur, 12, "stack-args=[")) {
                    map<int, list<IDebugger::VariableSafePtr> > frames_args ;
                    if (!parse_stack_arguments (a_input, cur, cur, frames_args)) {
                        LOG_PARSING_ERROR (a_input, cur) ;
                    } else {
                        LOG_D ("parsed stack args", GDBMI_FRAME_PARSING_DOMAIN) ;
                    }
                    result_record.frames_parameters (frames_args)  ;
                } else if (!a_input.compare (cur, 8, "locals=[")) {
                    //TODO: finish this!
                    list<VariableSafePtr> vars ;
                    if (!parse_local_var_list (a_input, cur, cur, vars)) {
                        LOG_PARSING_ERROR (a_input, cur) ;
                    } else {
                        LOG_D ("parsed local vars", GDBMI_FRAME_PARSING_DOMAIN) ;
                        result_record.local_variables (vars) ;
                    }
                } else {
                    GDBMIResultSafePtr result ;
                    if (!parse_result (a_input, cur, cur, result)) {
                        LOG_PARSING_ERROR (a_input, cur) ;
                    } else {
                        LOG_D ("parsed unknown gdbmi result",
                               GDBMI_FRAME_PARSING_DOMAIN) ;
                    }
                }

                if (a_input[cur] == ',') {
                    goto fetch_gdbmi_result;
                }

                //skip the remaining things we couldn't parse, until the
                //'end of line' character.
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

    /// parse a GDB/MI output record
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

    //*****************************************************************
    //                 </GDB/MI parsing functions>
    //*****************************************************************

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
        UString reason = m_out_of_band_record.stop_reason_as_str () ;

        if (reason == "exited-signalled"
            || reason == "exited-normally"
            || reason == "exited") {
            m_engine->program_finished_signal ().emit () ;
        }
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

struct OnFramesListedHandler : OutputHandler {

    GDBEngine *m_engine ;

    OnFramesListedHandler (GDBEngine *a_engine) :
        m_engine (a_engine)
    {}

    bool can_handle (CommandAndOutput &a_in)
    {
        if (a_in.output ().has_result_record ()
            && (a_in.output ().result_record ().kind ()
                == Output::ResultRecord::DONE)
            && (a_in.output ().result_record ().has_call_stack ())) {
            LOG_D ("handler selected", NMV_DEFAULT_DOMAIN) ;
            return true ;
        }
        return false ;
    }

    void do_handle (CommandAndOutput &a_in)
    {
        m_engine->frames_listed_signal ().emit
            (a_in.output ().result_record ().call_stack ()) ;
    }
};//struct OnFramesListedHandler

struct OnFramesParamsListedHandler : OutputHandler {

    GDBEngine *m_engine ;

    OnFramesParamsListedHandler (GDBEngine *a_engine) :
        m_engine (a_engine)
    {}

    bool can_handle (CommandAndOutput &a_in)
    {
        if (a_in.output ().has_result_record ()
            && (a_in.output ().result_record ().kind ()
                == Output::ResultRecord::DONE)
            && (a_in.output ().result_record ().has_frames_parameters ())) {
            LOG_D ("handler selected", NMV_DEFAULT_DOMAIN) ;
            return true ;
        }
        return false ;
    }

    void do_handle (CommandAndOutput &a_in)
    {
        m_engine->frames_params_listed_signal ().emit
            (a_in.output ().result_record ().frames_parameters ()) ;
    }
};//struct OnFramesParamsListedHandler

struct OnInfoProcHandler : OutputHandler {

    GDBEngine *m_engine ;

    OnInfoProcHandler (GDBEngine *a_engine) :
        m_engine (a_engine)
    {}

    bool can_handle (CommandAndOutput &a_in)
    {
        if (a_in.has_command ()
            && (a_in.command ().value ().find ("info proc")
                != Glib::ustring::npos)
            && (a_in.output ().has_out_of_band_record ())) {

            LOG_D ("handler selected", NMV_DEFAULT_DOMAIN) ;
            return true ;
        }
        return false ;
    }

    void do_handle (CommandAndOutput &a_in)
    {
        THROW_IF_FAIL (m_engine) ;

        int pid=0 ;
        if (!m_engine->extract_proc_info (a_in.output (), pid)) {
            LOG_D ("failed to extract proc info", NMV_DEFAULT_DOMAIN) ;
            return ;
        }
        THROW_IF_FAIL (pid) ;
        m_engine->got_proc_info_signal ().emit (pid) ;
    }
};//struct OnInfoProcHandler

struct OnLocalVariablesListedHandler : OutputHandler {

    GDBEngine *m_engine ;

    OnLocalVariablesListedHandler (GDBEngine *a_engine) :
        m_engine (a_engine)
    {}

    bool can_handle (CommandAndOutput &a_in)
    {
        if (a_in.output ().has_result_record ()
            && (a_in.output ().result_record ().kind ()
                == Output::ResultRecord::DONE)
            && (a_in.output ().result_record ().has_local_variables ())) {
            LOG_D ("handler selected", NMV_DEFAULT_DOMAIN) ;
            return true ;
        }
        return false ;
    }

    void do_handle (CommandAndOutput &a_in)
    {
        THROW_IF_FAIL (m_engine) ;

        m_engine->local_variables_listed_signal ().emit
            (a_in.output ().result_record ().local_variables ()) ;
    }
};//struct OnLocalVariablesListedHandler
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
        vector<UString> gdb_opts ;
        THROW_IF_FAIL (m_priv->launch_gdb_and_set_args
                                    (a_source_search_dirs, a_argv, gdb_opts));

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
        queue_command (command, a_run_event_loops) ;
        command.value ("info proc") ;
        queue_command (command, a_run_event_loops) ;
    }
}

void
GDBEngine::load_core_file (const UString &a_prog_path,
                           const UString &a_core_path,
                           bool a_run_event_loop)
{
    THROW_IF_FAIL (m_priv) ;
    if (m_priv->is_gdb_running ()) {
        m_priv->kill_gdb () ;
    }

    vector<UString> src_dirs, gdb_opts ;
    THROW_IF_FAIL (m_priv->launch_gdb_on_core_file (a_prog_path, a_core_path)) ;
}

bool
GDBEngine::attach_to_program (unsigned int a_pid)
{
    THROW_IF_FAIL (m_priv) ;
    vector<UString> args, source_search_dirs ;

    if (!m_priv->is_gdb_running ()) {
        vector<UString> gdb_opts ;
        THROW_IF_FAIL (m_priv->launch_gdb (source_search_dirs, gdb_opts)) ;

        IDebugger::Command command ;
        command.value ("set breakpoint pending auto") ;
        queue_command (command) ;
    }
    if (a_pid == (unsigned int)m_priv->gdb_pid) {
        return false ;
    }
    queue_command (Command ("attach " + UString::from_int (a_pid))) ;
    queue_command (Command ("info proc")) ;
    return true ;
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
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnFramesListedHandler (this))) ;
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnFramesParamsListedHandler (this))) ;
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnInfoProcHandler (this))) ;
    m_priv->output_handlers.push_back
            (OutputHandlerSafePtr (new OnLocalVariablesListedHandler (this))) ;
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

sigc::signal<void, const vector<IDebugger::Frame>& >&
GDBEngine::frames_listed_signal () const
{
    return m_priv->frames_listed_signal ;
}

sigc::signal<void, int>&
GDBEngine::got_proc_info_signal () const
{
    return m_priv->got_proc_info_signal ;
}

sigc::signal<void, const map< int, list<IDebugger::VariableSafePtr> >&>&
GDBEngine::frames_params_listed_signal () const
{
    return m_priv->frames_params_listed_signal ;
}

sigc::signal<void, int, IDebugger::Frame&> &
GDBEngine::current_frame_signal () const
{
    return m_priv->current_frame_signal ;
}

sigc::signal<void, const list<IDebugger::VariableSafePtr>& >&
GDBEngine::local_variables_listed_signal () const
{
    return m_priv->local_variables_listed_signal ;
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

void
GDBEngine::on_got_proc_info_signal (int a_pid)
{
    NEMIVER_TRY

    LOG_D ("target pid: '" << (int) a_pid << "'", NMV_DEFAULT_DOMAIN) ;
    m_priv->target_pid = a_pid ;

    NEMIVER_CATCH_NOX
}

//******************
//</signal handlers>
//******************
void
GDBEngine::init ()
{
    stdout_signal ().connect (sigc::mem_fun
            (*this, &GDBEngine::on_debugger_stdout_signal)) ;
    got_proc_info_signal ().connect (sigc::mem_fun
            (*this, &GDBEngine::on_got_proc_info_signal)) ;

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
    m_priv->run_loop_iterations_real (a_nb_iters) ;
}


void
GDBEngine::execute_command (const IDebugger::Command &a_command)
{
    THROW_IF_FAIL (m_priv && m_priv->is_gdb_running ()) ;
    queue_command (a_command) ;
}

bool
GDBEngine::queue_command (const IDebugger::Command &a_command,
                          bool a_run_event_loops)
{
    bool result (false) ;
    THROW_IF_FAIL (m_priv && m_priv->is_gdb_running ()) ;
    LOG_D ("queuing command: '" << a_command.value () << "'",
           NMV_DEFAULT_DOMAIN) ;
    m_priv->queued_commands.push_back (a_command) ;
    if (m_priv->started_commands.empty ()) {
        result = m_priv->issue_command (*m_priv->queued_commands.begin (),
                                        a_run_event_loops) ;
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
    LOG_D ("sending -exec-continue to gdb", NMV_DEFAULT_DOMAIN) ;
    queue_command (Command ("-exec-continue"), a_run_event_loops) ;
}

void
GDBEngine::run (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    LOG_D ("sending -exec-run to gdb", NMV_DEFAULT_DOMAIN) ;
    queue_command (Command ("-exec-run"), a_run_event_loops) ;
    queue_command (Command ("info proc"), a_run_event_loops) ;
}

bool
GDBEngine::stop (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    if (!m_priv->is_gdb_running ()) {
        LOG_ERROR_D ("GDB is not running", NMV_DEFAULT_DOMAIN) ;
        return false;
    }
    return  (kill (m_priv->target_pid, SIGINT) == 0) ;
}

void
GDBEngine::step_in (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-step"), a_run_event_loops) ;
}

void
GDBEngine::step_out (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-finish"), a_run_event_loops) ;
}

void
GDBEngine::step_over (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-exec-next"), a_run_event_loops) ;
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
    queue_command (Command ("-break-list "), a_run_event_loops) ;
}

void
GDBEngine::list_breakpoints (bool a_run_event_loops)
{
    THROW_IF_FAIL (m_priv) ;
    queue_command (Command ("-break-list"), a_run_event_loops) ;
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
    queue_command (Command ("-break-insert " + a_func_name), a_run_event_loops) ;
}

void
GDBEngine::enable_breakpoint (const UString &a_path,
                              gint a_line_num,
                              bool a_run_event_loops)
{
    //TODO: code this
}

void
GDBEngine::disable_breakpoint (const UString &a_path,
                               gint a_line_num,
                               bool a_run_event_loops)
{
    //TODO: code this
}

void
GDBEngine::delete_breakpoint (const UString &a_path,
                              gint a_line_num,
                              bool a_run_event_loops)
{
    queue_command (Command ("-break-delete "
                + a_path
                + ":"
                + UString::from_int (a_line_num)), a_run_event_loops) ;
}

void
GDBEngine::delete_breakpoint (gint a_break_num, bool a_run_event_loops)
{
    queue_command (Command ("-break-delete "
                + UString::from_int (a_break_num)), a_run_event_loops) ;
}

void
GDBEngine::list_frames (bool a_run_event_loops)
{
    queue_command (Command ("-stack-list-frames" ), a_run_event_loops) ;
}

void
GDBEngine::list_frames_arguments (int a_low_frame,
                                  int a_high_frame,
                                  bool a_run_event_loops)
{
    if (a_low_frame < 0 || a_high_frame < 0) {
        queue_command (Command ("-stack-list-arguments 1" ), a_run_event_loops) ;
    } else {
        queue_command (Command ("-stack-list-arguments 1 "
                                + UString::from_int (a_low_frame)
                                + " "
                                + UString::from_int (a_high_frame)),
                       a_run_event_loops) ;
    }
}

void
GDBEngine::list_local_variables (bool a_run_event_loops)
{
    Command command ("-stack-list-locals 2") ;
    queue_command (command, a_run_event_loops) ;
}

/// Extracts proc info from the out of band records
bool
GDBEngine::extract_proc_info (Output &a_output, int &a_pid)
{
    THROW_IF_FAIL (m_priv) ;

    if (!a_output.has_out_of_band_record ()) {
        LOG_ERROR_D ("output has no out of band record", NMV_DEFAULT_DOMAIN) ;
        return false ;
    }

    //********************************************
    //search the out of band record
    //that contains the debugger console
    //stream record with the string process <pid>
    //********************************************
    UString record ;
    UString::size_type index ;
    bool found=false ;
    list<Output::OutOfBandRecord>::const_iterator record_iter =
                                    a_output.out_of_band_records ().begin ();
    for (; record_iter != a_output.out_of_band_records ().end (); ++record_iter) {
        if (!record_iter->has_stream_record ()) {continue;}

        record = record_iter->stream_record ().debugger_console () ;
        if (record == "") {continue;}

        LOG_D ("found a debugger console stream record", NMV_DEFAULT_DOMAIN) ;

        index = record.find ("process ");
        if (index == Glib::ustring::npos) {continue;}
        found = true ;
        break ;
    }
    if (!found) {
        LOG_D ("output has no process info", NMV_DEFAULT_DOMAIN) ;
        return false;
    }
    index += 7 ;
    UString pid ;
    while (index < record.size () && isspace (record[index])) {++index;}
    while (index < record.size () && isdigit (record[index])) {
        pid += record[index];
        ++index ;
    }
    LOG_D ("extracted PID: '" << pid << "'", NMV_DEFAULT_DOMAIN) ;
    a_pid = atoi (pid.c_str ()) ;
    return true ;
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

