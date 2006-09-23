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
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NEMIVER_RUN_PROGRAM_DIALOG_H__
#define __NEMIVER_RUN_PROGRAM_DIALOG_H__

#include <map>
#include "nmv-dialog.h"
#include "nmv-safe-ptr-utils.h"
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

namespace nemiver {

namespace common {
class UString ;
}

using nemiver::common::UString ;
using nemiver::common::SafePtr ;

class RunProgramDialog : public Dialog {

    Gtk::TreeView* m_treeview_environment;

    struct EnvVarModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        // I tried using UString here, but it didn't want to compile... jmj
        Gtk::TreeModelColumn<Glib::ustring> varname;
        Gtk::TreeModelColumn<Glib::ustring> value;
        EnvVarModelColumns() { add (varname); add (value); }
    };
    EnvVarModelColumns m_env_columns;
    Glib::RefPtr<Gtk::ListStore> m_model;
    void on_add_new_variable ();
    void on_remove_variable ();
    void on_variable_selection_changed ();

public:

    RunProgramDialog (const UString &a_resource_root_path) ;

    virtual ~RunProgramDialog () ;

    UString program_name () const ;
    void program_name (const UString &a_name) ;

    UString arguments () const ;
    void arguments (const UString &a_args) ;

    UString working_directory () const ;
    void working_directory (const UString &) ;

    std::map<UString, UString> environment_variables () const;
    void environment_variables (const std::map<UString, UString> &);

};//end class nemiver

}//end namespace nemiver

#endif //__NEMIVER_RUN_PROGRAM_DIALOG_H__

