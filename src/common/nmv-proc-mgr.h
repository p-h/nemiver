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

#ifndef __NMV_PROC_MGR_H__
#define __NMV_PROC_MGR_H__

#include <sys/types.h>
#include <pwd.h>
#include <list>
#include "nmv-object.h"
#include "nmv-ustring.h"
#include "nmv-safe-ptr-utils.h"

using std::list;
using nemiver::common::UString;
using nemiver::common::Object;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::SafePtr;

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

class IProcMgr;
typedef SafePtr<IProcMgr, ObjectRef, ObjectUnref> IProcMgrSafePtr;

class NEMIVER_API IProcMgr : public Object {
    //non copyable
    IProcMgr (const IProcMgr &);
    IProcMgr& operator= (const IProcMgr &);

protected:
    IProcMgr () {}

public:

    class Process {
        unsigned int m_pid;
        unsigned int m_ppid;
        unsigned int m_uid;
        unsigned int m_euid;
        UString m_user_name;
        list<UString> m_args;

    public:

        Process () :
            m_pid (0),
            m_ppid (0),
            m_uid (0),
            m_euid (0)
        {}

        Process (unsigned int a_pid, const list<UString> a_args) :
            m_pid (a_pid),
            m_ppid (0),
            m_uid (0),
            m_euid (0),
            m_args (a_args)
        {}

        Process (unsigned int a_pid) :
            m_pid (a_pid),
            m_ppid (0),
            m_uid (0),
            m_euid (0)
        {}

        unsigned int pid () const {return m_pid;}
        void pid (unsigned int a_pid) {m_pid = a_pid;}

        unsigned int ppid () const {return m_ppid;}
        void ppid (unsigned int a_ppid) {m_ppid = a_ppid;}

        unsigned int uid () const {return m_uid;}
        void uid (unsigned int a_uid) {m_uid = a_uid;}

        unsigned int euid () const {return m_euid;}
        void euid (unsigned int a_euid) {m_euid = a_euid;}

        const UString& user_name () const {return m_user_name;}
        void user_name (const UString &a_name) {m_user_name = a_name;}

        const list<UString>& args () const {return m_args;}
        list<UString>& args () {return m_args;}
        void args (const list<UString> &a_in) {m_args = a_in;}

        bool operator== (const Process &an_other)
        {
            if (pid () == an_other.pid ()
                && ppid () == an_other.ppid ()
                && uid () == an_other.uid ()
                && euid () == an_other.euid ()) {
                return true;
            }
            return false;
        }
    };//end class Process

    virtual ~IProcMgr () {}
    static IProcMgrSafePtr create ();
    virtual const list<Process>& get_all_process_list () const = 0;
    virtual bool get_process_from_pid (pid_t a_pid,
                                       Process &a_process) const = 0;
    virtual bool get_process_from_name
                                    (const UString &a_pname,
                                     Process &a_process,
                                     bool a_fuzzy_search=false) const = 0;
};//end IProcMgr

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_PROC_MGR_H__

