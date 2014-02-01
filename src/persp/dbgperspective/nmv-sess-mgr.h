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
#ifndef __NMV_SESS_MGR_H__
#define __NMV_SESS_MGR_H__

#include <list>
#include <map>
#include "common/nmv-object.h"
#include "common/nmv-ustring.h"
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-transaction.h"
#include "nmv-i-debugger.h"

using namespace std;
using nemiver::common::Object;
using nemiver::common::UString;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::SafePtr;
using nemiver::common::Transaction;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class ISessMgr;
typedef SafePtr<ISessMgr, ObjectRef, ObjectUnref> ISessMgrSafePtr;

class NEMIVER_API ISessMgr : public Object {
    //non copyable
    ISessMgr (const ISessMgr&);
    ISessMgr& operator= (const ISessMgr&);

protected:
    ISessMgr () {};

public:
    class Breakpoint {
        UString m_file_name;
        UString m_file_full_name;
        int m_line_number;
        bool m_enabled;
        UString m_condition;
        int m_ignore_count;
	bool m_is_countpoint;

    public:
        Breakpoint (const UString &a_file_name,
                    const UString &a_file_full_name,
                    const UString &a_line_number,
                    const UString &a_enabled,
                    const UString &a_condition,
                    const UString &a_ignore_count,
                    const UString &a_is_countpoint) :
	m_file_name (a_file_name),
            m_file_full_name (a_file_full_name),
            m_line_number (atoi (a_line_number.c_str ())),
            m_enabled (atoi (a_enabled.c_str ())),
            m_condition (a_condition),
            m_ignore_count (atoi (a_ignore_count.c_str ())),
            m_is_countpoint (atoi (a_is_countpoint.c_str ()))
        {}

        Breakpoint (const UString &a_file_name,
                    const UString &a_file_full_name,
                    int a_line_number,
                    bool a_enabled,
                    const UString &a_condition,
                    unsigned a_ignore_count,
		    bool a_is_countpoint) :
	m_file_name (a_file_name),
            m_file_full_name (a_file_full_name),
            m_line_number (a_line_number),
            m_enabled (a_enabled),
            m_condition (a_condition),
            m_ignore_count (a_ignore_count),
            m_is_countpoint (a_is_countpoint)
        {}

        Breakpoint () :
        m_line_number (0),
            m_enabled (true),
            m_ignore_count (0),            
            m_is_countpoint (false)
        {}

        const UString& file_name () const {return m_file_name;}
        void file_name (const UString &a_in) {m_file_name = a_in;}

        const UString& file_full_name () const {return m_file_full_name;}
        void file_full_name (const UString &a_in) {m_file_full_name = a_in;}

        int line_number () const {return m_line_number;}
        void line_number (int a_in) {m_line_number = a_in;}

        bool enabled () const { return m_enabled; }
        void enabled (bool a_in) { m_enabled = a_in;}

        const UString& condition () const {return m_condition;}
        void condition (const UString &a_cond) {m_condition = a_cond;}

        int ignore_count () const {return m_ignore_count;}
        void ignore_count (int a_c) {m_ignore_count = a_c;}

        bool is_countpoint () const {return m_is_countpoint;}
        void is_countpoint (bool a) {m_is_countpoint = a;}
    };

    class WatchPoint {
        UString m_expression;
        bool m_is_write;
        bool m_is_read;

    public:
        WatchPoint (const UString &a_expression,
                    bool a_is_write,
                    bool a_is_read) :
            m_expression (a_expression),
            m_is_write (a_is_write),
            m_is_read (a_is_read)
        {
        }

        WatchPoint (const UString &a_expression,
                    UString &a_is_write,
                    UString &a_is_read) :
            m_expression (a_expression),
            m_is_write (a_is_write == "true" ? true : false),
            m_is_read (a_is_read == "true" ? true : false)
        {
        }

        const UString& expression () const {return m_expression;}
        void expression (const UString &a_expr) {m_expression = a_expr;}

        bool is_write () const {return m_is_write;}
        void is_write (bool f) {m_is_write = f;}

        bool is_read () const {return m_is_read;}
        void is_read (bool f)  {m_is_read = f;}
    }; // end class WatchPoint

    class Session {
        gint64 m_session_id;
        map<UString, UString> m_properties;
        map<UString, UString> m_env_variables;
        list<Breakpoint> m_breakpoints;
        list<WatchPoint> m_watchpoints;
        list<UString> m_opened_files;
        list<UString> m_search_paths;

    public:
        Session () :
            m_session_id (0)
        {}

        Session (gint64 a_session_id) :
            m_session_id (a_session_id)
        {}

        gint64 session_id () const {return m_session_id;}
        void session_id (gint64 a_in) {m_session_id = a_in;}

        const map<UString, UString>& properties ()  const {return m_properties;}
        map<UString, UString>& properties () {return m_properties;}

        const map<UString, UString>& env_variables ()  const
        {
            return m_env_variables;
        }
        map<UString, UString>& env_variables () {return m_env_variables;}

        list<Breakpoint>& breakpoints () {return m_breakpoints;}
        const list<Breakpoint>& breakpoints () const { return m_breakpoints;}

        list<WatchPoint>& watchpoints () {return m_watchpoints;}
        const list<WatchPoint>& watchpoints () const {return m_watchpoints;}

        list<UString>& opened_files () {return m_opened_files;}
        const list<UString>& opened_files () const {return m_opened_files;}

        list<UString>& search_paths () {return m_search_paths;}
        const list<UString>& search_paths () const {return m_search_paths;}
    };//end class Session

    virtual ~ISessMgr () {}
    virtual Transaction& default_transaction () = 0;
    virtual list<Session>& sessions () = 0;
    virtual const list<Session>& sessions () const = 0;
    virtual void store_session (Session &a_session,
                                Transaction &a_trans) = 0;
    virtual void store_sessions (Transaction &a_trans) = 0;
    virtual void load_session (Session &a_session,
                               Transaction &a_trans) = 0;
    virtual void load_sessions (Transaction &a_trans) = 0;
    virtual void load_sessions () = 0;
    virtual void delete_session (gint64 a_id,
                                 Transaction &a_trans) = 0;
    virtual void delete_session (gint64 a_id) = 0;
    virtual void delete_sessions (Transaction &a_trans)  = 0;
    virtual void delete_sessions () = 0;

    virtual void clear_session (gint64 a_id, Transaction &a_trans) = 0;
    virtual void clear_session (gint64 a_id) = 0;
    static ISessMgrSafePtr create (const UString &a_root_dir);
};//end class SessMgr

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_SESS_MGR_H__

