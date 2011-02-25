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
#ifndef __NMV_LOG_STREAM_H__
#define __NMV_LOG_STREAM_H__
#include <string>
#include "nmv-api-macros.h"
#include "nmv-ustring.h"
#include "nmv-safe-ptr.h"
#include "nmv-api-macros.h"

#ifndef NMV_DEFAULT_DOMAIN
#define NMV_DEFAULT_DOMAIN __extension__        \
    ({                                          \
        const char* path = __FILE__;            \
        Glib::path_get_basename (path);         \
    })
#endif

#ifndef NMV_GENERAL_DOMAIN
#define NMV_GENERAL_DOMAIN "general-domain"
#endif

using std::string;
namespace nemiver {
namespace common {

/// \brief the log stream class
/// it features logs on cout, cerr, and files.
/// it also features log domains and log levels.
class NEMIVER_API LogStream
{
    friend LogStream& timestamp (LogStream &);
    friend LogStream& flush (LogStream &);
    friend LogStream& endl (LogStream &);
    struct Priv;
    SafePtr<Priv> m_priv;

    //forbid copy/assignation
    LogStream (LogStream const&);
    LogStream& operator= (LogStream const&);

protected:

public:

    enum StreamType {
        FILE_STREAM = 1,
        COUT_STREAM = 1 >> 1,
        CERR_STREAM = 1 >> 2,
        RFU0,//reserved for future usage
        RFU1,
        RFU2,
    };

    enum LogLevel {
        LOG_LEVEL_NORMAL=0,
        LOG_LEVEL_VERBOSE
    };

    /// \brief set the type of all the log streams that will be instanciated
    ///(either cout, cerr, or log file). By default, the type of stream is
    /// set to COUT_STREAM. All the logs are sent to stdout.
    /// \param a_type the type of the log stream
    static void set_stream_type (enum StreamType a_type);

    /// \brief gets the type of the instances of #LogStream
    /// \return the stream type as set by LogStream::set_stream_type().
    static enum StreamType get_stream_type ();

    /// in the case where the stream type is set to FILE_STREAM,
    /// this methods sets the path of the file to log into. By default,
    /// the log file is ./log.txt
    /// \param a_file_path the log file path.
    /// \param a_len the length of the file_path. If <0, it means that a_file_path
    /// is a zero terminated string.
    static void set_stream_file_path (const char* a_file_path, long a_len=-1);

    /// \brief gets the log file path, in case the stream type is set to
    /// FILE_STREAM
    /// \return the path to the log file.
    static const char* get_stream_file_path ();

    /// \brief sets the log level filter.
    /// if the filter is set to LOG_LEVEL_NORMAL, only the log streams that
    /// have a log level set to LOG_LEVEL_NORMAL will actually log data.
    /// If the filter is set to LOG_LEVEL_VERBOSE, streams set to
    /// LOG_LEVEL_NORMAL *and* streams set to LOG_LEVEL_VERBOSE will be
    /// logging data.
    ///
    /// \param a_level the level of verbosity you want your log streams to have.
    static void set_log_level_filter (enum LogLevel a_level);

    /// \brief sets a filter on the log domain
    /// only streams that have the same domain as the one set here will
    /// be logging data.
    /// \param a_domain the domain name.
    /// \a_len the length of the domain name. If <0, it means that a_domain
    /// is a zero terminated string.
    static void set_log_domain_filter (const char* a_domain, long a_len=-1);

    /// \brief activate/de-activate the logging.
    /// \param a_activate true to activate the logging, false to deactivate.
    static void activate (bool a_activate);

    /// \brief tests wether the logging is activated or not.
    /// \return true if the logging is activated, false otherwise.
    static bool is_active ();

    /// \brief gets the log stream instanciated by the system by default.
    /// the options of this log stream are the global options set before
    /// the first call to this method.
    /// \return the log stream instanciated by default.
    static LogStream& default_log_stream ();


    /// \brief default constructor of a log stream
    /// \param a_level the log level of the stream. This stream
    /// will log data if its log level is inferior or equal to
    /// the log level filter defined by LogStream::set_log_level_filter().
    /// \param a_domain the log domain. A stream will log data if its
    /// its log level is <= to the log level filter, *and* if its domain equals
    /// the domain filter.
    LogStream (enum LogLevel a_level=LOG_LEVEL_NORMAL,
               const string &a_default_domain=NMV_GENERAL_DOMAIN);

    /// \brief destructor of the log stream class
    virtual ~LogStream ();

    /// \brief enable or disable logging for a domain
    /// \param a_domain the domain to enable logging for
    /// \param a_do_enable when set to true, enables the logging for domain
    /// @a_domain, disable it otherwise.
    void enable_domain (const string &a_domain,
                        bool a_do_enable=true);

    /// \return true is logging is enabled for domain @a_domain
    bool is_domain_enabled (const string &a_domain);

    /// \brief writes a text string to the stream
    /// \param a_buf the buffer that contains the text string.
    /// \param a_buflen the length of the buffer. If <0, a_buf is
    /// considered as a zero terminated string.
    /// \param a_domain the domain the string has to be logged against.
    LogStream& write (const char *a_buf,
                      long a_buflen =-1,
                      const string &a_domain=NMV_GENERAL_DOMAIN);

    /// \brief log a message to the stream
    /// \param a_msg the message to log
    /// \param a_domain the domain to log against
    LogStream& write (const Glib::ustring &a_msg,
                      const string &a_domain=NMV_GENERAL_DOMAIN);

    LogStream& write (int a_msg,
                      const string &a_domain=NMV_GENERAL_DOMAIN);

    LogStream& write (double a_msg,
                      const string &a_domain=NMV_GENERAL_DOMAIN);

    LogStream& write (char a_msg,
                      const string &a_domain=NMV_GENERAL_DOMAIN);

    /// set the domain in against which all the coming
    /// messages will be logged.
    /// This is to be used in association with the << operators where
    /// we cannot specify the domain to log against, unlike LogStream::write() .
    /// \param a_domain the domain to log against.
    void push_domain (const string &a_domain);

    /// pops the last domain that has been pushed using LogStream::push_domain.
    void pop_domain ();

    /// \brief log zero teriminated strings
    /// \param a_string the string to log
    LogStream& operator<< (const char* a_c_string);

    /// \brief log a string
    /// \param a_string the string to log
    LogStream& operator<< (const std::string &a_string);

    /// \brief log a UTF-8 string
    /// \param a_string the string to log
    LogStream& operator<< (const Glib::ustring &a_string);

    /// \brief log an integer
    /// \param an_int the integer to log
    LogStream& operator<< (int an_int);

    /// \brief log a double
    /// \param the double to log
    LogStream& operator<< (double a_double);

    /// \brief log a character
    /// \param a_char the char to log
    LogStream& operator<< (char a_char);

    /// \brief  log a stream manipulator
    /// \param a_manipulator the LogStream manipulator to log
    LogStream& operator<< (LogStream& (*a_manipulator) (LogStream&));

    friend LogStream& level_normal (LogStream &a_stream);

    friend LogStream& level_verbose (LogStream &a_stream);

};//end class LogStream

/// \brief logs a timestamp. Basically the
/// the current date. You use it like:
/// nemiver::LogStream out; out << nemiver::timestamp ;
NEMIVER_API LogStream& timestamp (LogStream&);

/// \brief flushes the stream
/// Use it like: nemiver::LogStream out;
/// out << "Hello" << nemiver::flush;
NEMIVER_API LogStream& flush (LogStream &);

/// \brief log a '\\n' and flushes the stream
/// Use it like: nemiver::LogStream out;
/// out << "hello"<< nemiver::endl;
NEMIVER_API LogStream& endl (LogStream &);

/// \brief sets the log level to normal
/// Use it like nemiver::LogStream out;
/// out << nemiver::level_normal << "blabla";
NEMIVER_API LogStream& level_normal (LogStream &);

/// \brief sets the log level to verbose
/// Use it lik: nemiver::LogStream out;
/// out << nemiver::level_verbose << "bla bla bla";
NEMIVER_API LogStream& level_verbose (LogStream &);

}//end namespace common
}//end namespace nemiver

#endif //__NMV_LOG_STREAM_H__

