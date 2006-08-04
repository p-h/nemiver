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
#include "nmv-object.h"
#include "nmv-ustring.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"

using namespace std ;
using nemiver::common::Object ;
using nemiver::common::UString ;
using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;
using nemiver::common::SafePtr ;

namespace nemiver {

class SessMgr ;
typedef SafePtr<SessMgr, ObjectRef, ObjectUnref> SessMgrSafePtr  ;

class SessMgr : public Object {
    //non copyable
    SessMgr (const SessMgr&) ;
    SessMgr& operator= (const SessMgr&) ;
    struct Priv ;
    SafePtr<Priv> m_priv ;

protected:
    SessMgr () ;

public:

    class Session {
        UString m_name ;
        list<IDebugger::BreakPoint> m_breakpoints ;
        list<UString> m_opened_files ;

    public:
        const UString& name ()  const {return m_name;}
        void name (const UString &a_in) {m_name = a_in;}
        list<IDebugger::BreakPoint>& breakpoints () {return m_breakpoints;}
        const list<IDebugger::BreakPoint>& breakpoints () const {return m_breakpoints;}
        list<UString>& opened_files () {return m_opened_files;}
        const list<UString>& opened_files () const {return m_opened_files;}
    };//end class Session

    SessMgr (const UString &root_dir) ;
    virtual ~SessMgr () {}
    list<Session>& sessions () ;
    const list<Session>& sessions () const ;
    void store_session (const Session &a_session) ;
    void store_sessions () ;
    void load_sessions ();
};//end class SessMgr

}//end namespace nemiver

#endif //__NMV_SESS_MGR_H__

