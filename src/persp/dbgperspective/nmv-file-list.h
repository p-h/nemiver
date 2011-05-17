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
#include "common/nmv-object.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

using nemiver::common::SafePtr;
using nemiver::common::UString;

/// display the list of source files that got compiled
/// to produce the executable being currently debugged.
/// When the widget is instanciated, it doesn't show anything.
/// The client code has to invoke FileList::update_content() to
/// have the FileList query the IDebugger interface for the source
///file list.
class NEMIVER_API FileList : public nemiver::common::Object {
    //non copyable
    FileList (const FileList&);
    FileList& operator= (const FileList&);

    struct Priv;
    SafePtr<Priv> m_priv;

public:

    FileList (IDebuggerSafePtr &a_debugger, const UString &a_starting_path);
    virtual ~FileList ();
    Gtk::Widget& widget () const;
    sigc::signal<void, const UString&>& file_activated_signal () const;
    sigc::signal<void>& files_selected_signal () const;
    void get_filenames (std::vector<std::string> &a_filenames) const;
    void update_content ();
    void expand_to_filename (const UString &a_filename);

};//end FileList

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_FILE_LIST_H__


