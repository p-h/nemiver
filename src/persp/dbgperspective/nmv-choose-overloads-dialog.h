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
#ifndef __NMV_CHOOSE_OVERLOADS_DIALOG_H__
#define __NMV_CHOOSE_OVERLOADS_DIALOG_H__

#include <vector>
#include "nmv-dialog.h"
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

class ChooseOverloadsDialog : public Dialog {
    struct Priv;
    SafePtr<Priv> m_priv;

public:
    ChooseOverloadsDialog
      (Gtk::Window &a_parent,
       const UString &a_res_root_path,
       const vector<IDebugger::OverloadsChoiceEntry> &a_entries);
    virtual ~ChooseOverloadsDialog ();

    void set_overloads_choice_entries
                        (const vector<IDebugger::OverloadsChoiceEntry> &a_in);
    const std::vector<IDebugger::OverloadsChoiceEntry>&
                                            overloaded_functions () const;
    void overloaded_function (int a_in) const;
    void clear ();

};//end ChooserOverloadsDialog

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_CHOOSE_OVERLOADS_DIALOG_H__

