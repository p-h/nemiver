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
 *GNU General Public License along with Goupil;
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
#include "nmv-object.h"
#include "nmv-ustring.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"
#include "nmv-transaction.h"

using namespace std ;
using nemiver::common::Object ;
using nemiver::common::UString ;
using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;
using nemiver::common::SafePtr ;
using nemiver::common::Transaction ;

namespace nemiver {

class ISessMgr ;
typedef SafePtr<ISessMgr, ObjectRef, ObjectUnref> ISessMgrSafePtr  ;

class NEMIVER_API ISessMgr : public Object {
    //non copyable
    ISessMgr (const ISessMgr&) ;
    ISessMgr& operator= (const ISessMgr&) ;

protected:
    ISessMgr () {};

public:
    class BreakPoint {
        UString m_file_name ;
        UString m_file_full_name ;
        int m_line_number ;

    public:
        BreakPoint (const UString &a_file_name,
                    const UString &a_file_full_name,
                    const UString &a_line_number) :
            m_file_name (a_file_name),
            m_file_full_name (a_file_full_name),
            m_line_number (atoi (a_line_number.c_str ()))
        {}

        BreakPoint (const UString &a_file_name,
                    const UString &a_file_full_name,
                    int a_line_number) :
            m_file_name (a_file_name),
            m_file_full_name (a_file_full_name),
            m_line_number (a_line_number)
        {}

        BreakPoint () :
            m_line_number (0)
        {}

        const UString& file_name () const {return m_file_name;}
        void file_name (const UString &a_in) {m_file_name = a_in;}

        const UString& file_full_name () const {return m_file_full_name;}
        void file_full_name (const UString &a_in) {m_file_full_name = a_in;}

        int line_number () const {return m_line_number;}
        void line_number (int a_in) {m_line_number = a_in;}
    };

    class Session {
        gint64 m_session_id ;
        map<UString, UString> m_properties ;
        map<UString, UString> m_env_variables ;
        list<BreakPoint> m_breakpoints ;
        list<UString> m_opened_files ;

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

        const map<UString, UString>& env_variables ()  const {return m_env_variables;}
        map<UString, UString>& env_variables () {return m_env_variables;}

        list<BreakPoint>& breakpoints () {return m_breakpoints;}
        const list<BreakPoint>& breakpoints () const
        {
            return m_breakpoints;
        }
        list<UString>& opened_files () {return m_opened_files;}
        const list<UString>& opened_files () const {return m_opened_files;}
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
    static ISessMgrSafePtr create (const UString &a_root_dir) ;
};//end class SessMgr


}//end namespace nemiver

#endif //__NMV_SESS_MGR_H__

