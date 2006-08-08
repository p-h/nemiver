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
#include <glibtop.h>
#include <glibtop/proclist.h>
#include <glibtop/procargs.h>

#include "nmv-proc-mgr.h"

namespace nemiver {

class ProcMgr : public IProcMgr {
    //non copyable
    ProcMgr (const ProcMgr &) ;
    ProcMgr& operator= (const ProcMgr &) ;

    list<Process> m_process_list ;
    friend class IProcMgr ;

protected:
    ProcMgr () ;

public:
    virtual ~ProcMgr () ;
    const list<Process>& get_all_process_list () const  ;
    list<Process>& get_all_process_list () ;
};//end class ProcMgr

struct LibgtopInit {
    LibgtopInit ()
    {
        glibtop_init () ;
    }

    ~LibgtopInit ()
    {
        glibtop_close () ;
    }
};//end struct LibgtopInit

ProcMgr::ProcMgr ()
{
    //init libgtop.
    static  LibgtopInit s_init ;
}

ProcMgr::~ProcMgr ()
{
}

list<ProcMgr::Process>&
ProcMgr::get_all_process_list ()
{
    glibtop_proclist buf_desc={0} ;
    unsigned int *pids=NULL;
    char **argv=NULL ;
    m_process_list.clear () ;

    try {
        pids = glibtop_get_proclist (&buf_desc, GLIBTOP_KERN_PROC_ALL, 0)  ;
        for (unsigned i=0 ; i < buf_desc.number ; ++i) {
            Process process (pids[i]) ;
            glibtop_proc_args process_args_desc = {0} ;
            argv = glibtop_get_proc_argv (&process_args_desc, pids[i], 30) ;
            char **cur_arg = argv ;
            while (*cur_arg) {
                process.args ().push_back
                    (UString (Glib::locale_to_utf8 (*cur_arg))) ;
                ++cur_arg ;
            }
            if (argv) {
                g_strfreev (argv) ;
                argv=NULL ;
            }
            m_process_list.push_back (process) ;
        }
    } catch (...) {
    }
    if (pids) {
        g_free (pids) ;
        pids=NULL ;
    }
    return m_process_list ;
}

IProcMgrSafePtr
IProcMgr::create_proc_mgr ()
{
    IProcMgrSafePtr result (new ProcMgr) ;
    return result ;
}

}//end namespace nemiver

