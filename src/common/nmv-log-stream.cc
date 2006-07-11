/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset: 4-*- */

/*Copyright (c) 2005-2006 Dodji Seketeli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <glibmm.h>
#include <glibmm/thread.h>
#include <glib/gstdio.h>
#include "nmv-log-stream.h"
#include "nmv-ustring.h"
#include "nmv-exception.h"
#include "nmv-date-utils.h"
#include "nmv-safe-ptr-utils.h"

namespace nemiver {
namespace common {

using namespace std ;

static enum LogStream::StreamType s_stream_type = LogStream::COUT_STREAM ;
static enum LogStream::LogLevel s_level_filter = LogStream::LOG_LEVEL_NORMAL ;
static bool s_is_active = true ;

/// the base class of the destination
/// of the messages send to a stream.
/// each log stream uses a particular
/// Log Sink, e.g, a sink that sends messages
/// to stdout, of a sink that sends messages to
/// to a file etc...

class LogSink : public Object {
protected:
    mutable Glib::Mutex m_ostream_mutex ;
    ostream *m_out ;

    //non copyable
    LogSink (const LogSink &) ;
    LogSink& operator= (const LogSink &) ;

    LogSink () ;

public:

    LogSink (ostream *a_out) : m_out (a_out) {}

    virtual ~LogSink () {}

    bool bad () const
    {
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        return m_out->bad () ;
    }

    bool good () const
    {
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        return m_out->good () ;
    }

    void flush ()
    {
        if (!m_out) throw runtime_error ("underlying ostream not initialized") ;
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        m_out->flush () ;
    }

    LogSink& write (const char *a_buf, long a_buflen)
    {
        if (!m_out) throw runtime_error ("underlying ostream not initialized") ;
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        m_out->write (a_buf, a_buflen) ;
        return *this ;
    }

    LogSink& operator<< (const UString &a_string)
    {
        if (!m_out) throw runtime_error ("underlying ostream not initialized") ;
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        *m_out << a_string ;
        return *this ;
    }

    LogSink& operator<< (int an_int)
    {
        if (!m_out) throw runtime_error ("underlying ostream not initialized") ;
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        *m_out << an_int ;
        return *this ;
    }

    LogSink& operator<< (unsigned long int an_int)
    {
        if (!m_out) throw runtime_error ("underlying ostream not initialized") ;
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        *m_out << an_int ;
        return *this ;
    }

    LogSink& operator<< (double a_double)
    {
        if (!m_out) throw runtime_error ("underlying ostream not initialized") ;
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        *m_out << a_double ;
        return *this ;
    }

    LogSink& operator<< (char a_char)
    {
        if (!m_out) throw runtime_error ("underlying ostream not initialized") ;
        Glib::Mutex::Lock lock (m_ostream_mutex) ;
        *m_out << a_char;
        return *this ;
    }
};//end class LogSink

class CoutLogSink : public LogSink {
public:
    CoutLogSink () : LogSink (&cout) {}
    virtual ~CoutLogSink () {}
};//end class CoutLogSink

class CerrLogSink : public LogSink {
public:
    CerrLogSink () : LogSink (&cerr) {}
    virtual ~CerrLogSink () {}
};//end class OStreamSink

class OfstreamLogSink : public LogSink {
    SafePtr<ofstream> m_ofstream ;

    void init_from_path (const UString &a_file_path)
    {
        GCharSafePtr dir (g_path_get_dirname (a_file_path.c_str ())) ;

        if (!Glib::file_test (dir.get (),  Glib::FILE_TEST_IS_DIR)) {
            if (g_mkdir_with_parents (dir, S_IRWXU)) {
                throw Exception (UString ("failed to create '")
                                 + static_cast<gchar*> (dir) + "'") ;
            }
        }
        m_ofstream = new ofstream (a_file_path.c_str ()) ;
        THROW_IF_FAIL (m_ofstream) ;
        if (!m_ofstream->good ()) {
            THROW ("Could not open file " + a_file_path) ;
        }
        m_out = m_ofstream.get () ;
    }

public:
    OfstreamLogSink (const UString &a_file_path) : LogSink (NULL)
    {
        init_from_path (a_file_path);
    }

    OfstreamLogSink () : LogSink (NULL)
    {
        vector<string> path_elems ;
        path_elems.push_back (Glib::get_current_dir ()) ;
        path_elems.push_back (string ("log.txt"));
        init_from_path (Glib::build_filename (path_elems).c_str ()) ;
    }

    virtual ~OfstreamLogSink ()
    {
        if (m_ofstream) {
            m_ofstream->flush ();
            m_ofstream->close ();
            m_ofstream = NULL ;
        }
    }
};//end class OfstreamLogSink

struct LogStream::Priv
{
    enum LogStream::StreamType stream_type ;
    SafePtr<LogSink, ObjectRef, ObjectUnref> sink ;

    //the domain of this log stream
    UString domain ;

    //the log level of this log stream
    enum LogStream::LogLevel level ;

    Priv ():
            stream_type (LogStream::COUT_STREAM),
            domain ("general"),
            level (LogStream::LOG_LEVEL_NORMAL)
    {}

    static UString&
    get_stream_file_path_private ()
    {
        static UString s_stream_file_path ;
        if (s_stream_file_path == "") {
            vector<string> path_elems ;
            path_elems.push_back (Glib::get_current_dir ()) ;
            path_elems.push_back (string ("log.txt"));
            s_stream_file_path = Glib::build_filename (path_elems).c_str () ;
        }
        return s_stream_file_path ;
    }

    static UString&
    get_domain_filter_private ()
    {
        static UString s_domain_filter ("all") ;
        return s_domain_filter ;
    }

    static bool
    is_logging_allowed (LogStream &a_this)
    {
        if (!a_this.m_priv) {
            return false ;
        }
        if (!LogStream::is_active ())
            return false ;

        //check domain
        UString domain_filter ("general");
        if (get_domain_filter_private () != "") {
            domain_filter = get_domain_filter_private ();
        }
        if (domain_filter != "all"
            && a_this.m_priv->domain != domain_filter) {
            return false ;
        }
        //check log level
        if (a_this.m_priv->level > s_level_filter) {
            return false;
        }
        return true ;
    }
}
;//end LogStream::Priv

void
LogStream::set_stream_type (enum StreamType a_type)
{
    s_stream_type = a_type ;
}

enum LogStream::StreamType
LogStream::get_stream_type ()
{
    return s_stream_type ;
}

void
LogStream::set_stream_file_path (const char* a_file_path, long a_len)
{
    UString &file_path = LogStream::Priv::get_stream_file_path_private () ;
    file_path.assign (a_file_path, a_len) ;
}

const char*
LogStream::get_stream_file_path ()
{
    return LogStream::Priv::get_stream_file_path_private ().c_str () ;
}

void
LogStream::set_log_level_filter (enum LogLevel a_level)
{
    s_level_filter = a_level ;
}

void
LogStream::set_log_domain_filter (const char* a_domain, long a_len)
{
    UString &domain_filter = LogStream::Priv::get_domain_filter_private ();
    domain_filter.assign (a_domain, a_len) ;
}

void
LogStream::activate (bool a_activate)
{
    s_is_active = a_activate ;
}

bool
LogStream::is_active ()
{
    return s_is_active ;
}

LogStream&
LogStream::default_log_stream ()
{
    static LogStream s_default_stream (LOG_LEVEL_NORMAL, "general") ;
    return s_default_stream ;
}

LogStream::LogStream (enum LogLevel a_level,
                      const char* a_domain)
{
    m_priv = new LogStream::Priv ;
    std::string file_path ;
    if (get_stream_type () == FILE_STREAM) {
        m_priv->sink = new OfstreamLogSink (get_stream_file_path ()) ;
    } else if (get_stream_type () == COUT_STREAM) {
        m_priv->sink = new CoutLogSink ;
    } else if (get_stream_type () == CERR_STREAM) {
        m_priv->sink = new CerrLogSink ;
    } else {
        g_critical ("LogStream type not supported") ;
        throw Exception ("LogStream type not supported") ;
    }
    m_priv->stream_type = get_stream_type ();
    m_priv->level = a_level ;
    m_priv->domain = a_domain ;
}

LogStream::~LogStream ()
{
    if (!m_priv) throw runtime_error ("double free in LogStrea::~LogStream") ;

    m_priv = NULL ;
}

LogStream&
LogStream::write (const char* a_buf, long a_buflen)
{
    if (!LogStream::Priv::is_logging_allowed (*this))
        return *this ;

    long len = 0 ;
    if (a_buflen > 0) {
        len = a_buflen ;
    } else {
        if (!a_buf)
            len = 0 ;
        else
            len = strlen (a_buf) ;
    }
    m_priv->sink->write (a_buf, len) ;
    if (m_priv->sink->bad ()) {
        cerr << "write failed\n" ;
        throw Exception ("write failed") ;
    }
    return *this ;
}

LogStream&
LogStream::operator<< (const UString &a_string)
{
    if (!m_priv || !m_priv->sink)
        return *this ;

    if (!LogStream::Priv::is_logging_allowed (*this))
        return *this ;

    *m_priv->sink << a_string ;
    if (m_priv->sink->bad ()) {
        cout << "write failed" ;
        throw Exception ("write failed") ;
    }
    return *this ;
}

LogStream&
LogStream::operator<< (int an_int)
{
    if (!m_priv || !m_priv->sink)
        return *this ;

    if (!LogStream::Priv::is_logging_allowed (*this))
        return *this ;

    *m_priv->sink << an_int;
    if (m_priv->sink->bad ()) {
        cout << "write failed" ;
        throw Exception ("write failed") ;
    }
    return *this ;
}

LogStream&
LogStream::operator<< (unsigned long an_int)
{
    if (!m_priv || !m_priv->sink)
        return *this ;

    if (!LogStream::Priv::is_logging_allowed (*this))
        return *this ;

    *m_priv->sink << an_int;
    if (m_priv->sink->bad ()) {
        cout << "write failed" ;
        throw Exception ("write failed") ;
    }
    return *this ;
}

LogStream&
LogStream::operator<< (double a_double)
{
    if (!m_priv || !m_priv->sink)
        return *this ;

    if (!LogStream::Priv::is_logging_allowed (*this))
        return *this ;

    *m_priv->sink << a_double;
    if (m_priv->sink->bad ()) {
        cout << "write failed" ;
        throw Exception ("write failed") ;
    }
    return *this ;
}

LogStream&
LogStream::operator<< (char a_char)
{
    if (!m_priv || !m_priv->sink)
        return *this ;

    if (!LogStream::Priv::is_logging_allowed (*this))
        return *this ;

    *m_priv->sink << a_char ;
    if (m_priv->sink->bad ()) {
        cout << "write failed" ;
        throw Exception ("write failed") ;
    }
    return *this ;
}

LogStream&
LogStream::operator<< (LogStream& (*a_manipulator) (LogStream &))
{
    a_manipulator (*this) ;
    return *this ;
}

LogStream&
timestamp (LogStream &a_stream)
{
    if (!LogStream::Priv::is_logging_allowed (a_stream))
        return a_stream ;
    UString now_str ;
    dateutils::get_current_datetime (now_str) ;
    a_stream << now_str ;
    return a_stream ;
}

LogStream&
flush (LogStream &a_stream)
{
    if (!LogStream::Priv::is_logging_allowed (a_stream))
        return a_stream ;

    a_stream.m_priv->sink->flush () ;
    return a_stream ;
}

LogStream&
endl (LogStream &a_stream)
{
    if (!LogStream::Priv::is_logging_allowed (a_stream))
        return a_stream;

    a_stream  << '\n' ;
    a_stream << flush  ;
    return a_stream ;
}

LogStream&
level_normal (LogStream &a_stream)
{
    a_stream.m_priv->level = LogStream::LOG_LEVEL_NORMAL ;
    return a_stream ;
}

LogStream&
level_verbose (LogStream &a_stream)
{
    a_stream.m_priv->level = LogStream::LOG_LEVEL_VERBOSE;
    return a_stream ;
}

}//end namespace common
}//end namespace nemiver

