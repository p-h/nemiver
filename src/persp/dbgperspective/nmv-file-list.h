//Author: Jonathon Jongsma
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
#ifndef __NMV_FILE_LIST_H__
#define __NMV_FILE_LIST_H__

#include <list>
#include <gtkmm/widget.h>
#include "nmv-object.h"
#include "nmv-i-debugger.h"
#include "nmv-safe-ptr-utils.h"

namespace nemiver {

class IWorkbench;
class IPerspective;

class NEMIVER_API FileList : public nemiver::common::Object {
    //non copyable
    FileList (const FileList&) ;
    FileList& operator= (const FileList&) ;

    struct Priv ;
    SafePtr<Priv> m_priv ;

public:

    FileList (IDebuggerSafePtr& a_debugger, IWorkbench& a_workbench, IPerspective& a_perspective) ;
    virtual ~FileList () ;
    Gtk::Widget& widget () const ;
    sigc::signal<void,
                 const UString&>& signal_file_selected () const ;

};//end FileList

}//end namespace nemiver

#endif //__NMV_FILE_LIST_H__


