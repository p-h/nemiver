//Author: Dodji Seketeli
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
#ifndef __NMV_FIND_TEXT_DIALOG_H__
#define __NMV_FIND_TEXT_DIALOG_H__

#include "nmv-dialog.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class SourceEditor;
class FindTextDialog;
using nemiver::common::SafePtr;

typedef SafePtr<FindTextDialog, common::ObjectRef, common::ObjectUnref>  FindTextDialogSafePtr;

class FindTextDialog : public Dialog {
    class Priv;
    SafePtr<Priv> m_priv;

public:

    FindTextDialog (Gtk::Window &a_parent,
                    const UString &a_resource_root_path);
    virtual ~FindTextDialog ();

    Gtk::TextIter& get_search_match_start () const;
    Gtk::TextIter& get_search_match_end () const;

    void get_search_string (UString &a_search_str) const;
    void set_search_string (const UString &a_search_str);

    bool get_match_case () const;
    void set_match_case (bool a_flag);

    bool get_match_entire_word () const;
    void set_match_entire_word (bool a_flag);

    bool get_wrap_around () const;
    void set_wrap_around (bool a_flag);

    bool get_search_backward () const;
    void set_search_backward (bool a_flag);

    bool clear_selection_before_search () const;
    void clear_selection_before_search (bool);
};//end FindTextDialog

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_FIND_TEXT_DIALOG_H__

