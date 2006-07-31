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
#ifndef __NEMIVER_I_PERSPECTIVE_H__
#define __NEMIVER_I_PERSPECTIVE_H__

#include <gtkmm.h>
#include <list>
#include "nmv-api-macros.h"
#include "nmv-plugin.h"
#include "nmv-i-workbench.h"

namespace nemiver {

using nemiver::common::Plugin ;
using std::list ;
using nemiver::common::UString ;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::SafePtr;
using nemiver::IWorkbenchSafePtr ;

class IPerspective ;

typedef SafePtr<IPerspective, ObjectRef, ObjectUnref> IPerspectiveSafePtr ;

class NEMIVER_API IPerspective : public Plugin::EntryPoint {
    //non copyable
    IPerspective (const IPerspective&) ;
    IPerspective& operator= (const IPerspective&) ;

protected:
    IPerspective () {}

public:

    /// initialize the perspective within the context of
    /// of the workbench that loads it.
    /// \param a_workbench, the workbench that loaded the
    /// current perspective.
    virtual void do_init (IWorkbenchSafePtr &a_workbench) = 0;

    /// Get a unique identifier of the perspective.
    /// It is a good practice that this remains legible.
    /// \return the unique identifier of the of the perspective.
    virtual const UString& get_perspective_identifier () = 0;

    /// this method is called by the Workbench when
    /// the perspective is first set in it.
    /// \params a_tbs the list of toolbars. The implementation of this method
    /// must fill this parameter with the list of toolbars it wants the workbench
    /// to display when this perspective becomes active.
    virtual void get_toolbars (list<Gtk::Toolbar*> &a_tbs) = 0 ;

    /// \returns the body of the perspective.
    virtual Gtk::Widget* get_body () = 0 ;

    /// This method is only called once, during the
    /// perspective's initialisation time,
    /// by the workbench.
    virtual void edit_workbench_menu () = 0;

    /// \brief open a source file from a url
    /// \param a_uri the uri of the file to open
    /// \param a_cur_line the line to flag as being the current exceution line
    /// if set to -1, this parameter is ignored.
    virtual bool open_file (const UString &a_uri, int a_cur_line=-1) = 0;

    /// \brief open a source file
    ///
    /// Let the user choose the set of files to open
    /// via a file chooser dialog and open them.
    virtual void open_file () = 0 ;

    /// \brief close the currently selected file
    virtual void close_current_file () = 0;

    /// \brief closes a file
    /// \param a_uri the uri that identifies the file to close
    virtual void close_file (const UString &a_uri) = 0;

    /// \name signals

    /// @{

    /// This signal is emited to notify the perspective
    /// about its activation state (whether it is activated or not).
    virtual sigc::signal<void, bool>& activated_signal () = 0;

    /// @}

};//end class IPerspective

}//end namespace nemiver

#endif //__NEMIVER_I_PERSPECTIVE_H__

