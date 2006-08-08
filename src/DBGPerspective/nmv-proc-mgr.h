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

#ifndef __NMV_PROC_MGR_H__
#define __NMV_PROC_MGR_H__

#include <list>
#include "nmv-object.h"
#include "nmv-ustring.h"
#include "nmv-safe-ptr-utils.h"

using std::list ;
using nemiver::common::UString ;
using nemiver::common::Object ;
using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;
using nemiver::common::SafePtr ;

namespace nemiver {

class IProcMgr ;
typedef SafePtr<IProcMgr, ObjectRef, ObjectUnref> IProcMgrSafePtr ;

class IProcMgr : public Object {
    //non copyable
    IProcMgr (const IProcMgr &) ;
    IProcMgr& operator= (const IProcMgr &) ;

protected:
    IProcMgr () ;

public:

    class Process {
        unsigned int m_pid ;
        list<UString> m_args ;

    public:

        Process () :
            m_pid (0)
        {}

        Process (unsigned int a_pid, const list<UString> a_args) :
            m_pid (a_pid),
            m_args (a_args)
        {}

        Process (unsigned int a_pid) :
            m_pid (a_pid)
        {}

        unsigned int pid () const {return m_pid;}
        void pid (unsigned int a_pid) {m_pid = a_pid;}

        const list<UString>& args () const {return m_args;}
        list<UString>& args () {return m_args;}
        void args (const list<UString> &a_in) {m_args = a_in;}
    };//end class Process

    virtual ~IProcMgr () {}
    static IProcMgrSafePtr create_proc_mgr () ;

    virtual list<Process>& get_all_process_list () = 0 ;
};//end IProcMgr

}//end namespace nemiver

#endif //__NMV_PROC_MGR_H__

