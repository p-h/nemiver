/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4; -*- */

/*Copyright (c) 2005-2006 Nemiver Seketeli
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
#include <glibmm.h>
#include "nmv-exception.h"
#include "nmv-ustring.h"
#include "nmv-scope-logger.h"

namespace nemiver {
namespace common {

struct ScopeLoggerPriv
{
    Glib::Timer timer ;
    LogStream *out ;
    UString name ;

    ScopeLoggerPriv (): out (NULL)
    {}
}
;

ScopeLogger::ScopeLogger (const char*a_scope_name)
{
    m_priv = new ScopeLoggerPriv () ;
    LogStream *out  = new LogStream () ;
    m_priv->name = a_scope_name ;

    *out << timestamp << "|" << m_priv->name << ":{\n" ;
    m_priv->timer.start () ;
    m_priv->out = out ;
}

ScopeLogger::ScopeLogger (const char*a_scope_name,
                          enum LogStream::LogLevel a_level)
{
    m_priv = new ScopeLoggerPriv () ;
    LogStream *out = new LogStream (a_level);
    m_priv->name = a_scope_name ;

    *out << timestamp << "|" << m_priv->name << ": {\n" ;
    m_priv->timer.start () ;
    m_priv->out = out ;
}

ScopeLogger::ScopeLogger (const char*a_scope_name,
                          enum LogStream::LogLevel a_level,
                          const char* a_log_domain)
{
    m_priv = new ScopeLoggerPriv () ;
    LogStream *out = new LogStream (a_level, a_log_domain);
    m_priv->name = a_scope_name ;

    *out << timestamp << "|" << m_priv->name << ": {\n" ;
    m_priv->timer.start () ;
    m_priv->out = out ;
}

ScopeLogger::~ScopeLogger ()
{
    THROW_IF_FAIL (m_priv && m_priv->out) ;
    if (m_priv) {
        m_priv->timer.stop () ;
        *m_priv->out << timestamp << "|" << m_priv->name <<": } elapsed: "
        << m_priv->timer.elapsed () << "secs \n" ;
        delete m_priv->out ;
        m_priv->out = NULL ;
        delete m_priv ;
        m_priv = NULL ;
    }
}

}//end namespace common
}//end namespace nemiver

