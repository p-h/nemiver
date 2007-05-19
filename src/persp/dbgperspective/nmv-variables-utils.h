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
#ifndef __NMV_VARIABLES_UTILS_H__
#define __NMV_VARIABLES_UTILS_H__

#include <list>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gdkmm/color.h>
#include "common/nmv-ustring.h"
#include "nmv-i-debugger.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (variables_utils)

struct VariableColumns : public Gtk::TreeModelColumnRecord {
    enum Offset {
        NAME_OFFSET=0,
        VALUE_OFFSET,
        TYPE_OFFSET,
        TYPE_CAPTION_OFFSET,
        VARIABLE_OFFSET,
        IS_HIGHLIGHTED_OFFSET,
        FG_COLOR_OFFSET
    };

    Gtk::TreeModelColumn<Glib::ustring> name ;
    Gtk::TreeModelColumn<Glib::ustring> value ;
    Gtk::TreeModelColumn<Glib::ustring> type ;
    Gtk::TreeModelColumn<Glib::ustring> type_caption ;
    Gtk::TreeModelColumn<IDebugger::VariableSafePtr> variable ;
    Gtk::TreeModelColumn<bool> is_highlighted;
    Gtk::TreeModelColumn<Gdk::Color> fg_color;

    VariableColumns ()
    {
        add (name) ;
        add (value) ;
        add (type) ;
        add (type_caption) ;
        add (variable) ;
        add (is_highlighted) ;
        add (fg_color) ;
    }
};//end VariableColumns

VariableColumns& get_variable_columns () ;
bool is_type_a_pointer (const UString &a_type) ;
bool is_qname_a_pointer_member (const UString &a_qname) ;

class NameElement {
    UString m_name ;
    bool m_is_pointer ;
    bool m_is_pointer_member ;

public:

    NameElement () :
        m_is_pointer (false),
        m_is_pointer_member (false)
    {
    }

    NameElement (const UString &a_name) :
        m_name (a_name),
        m_is_pointer (false),
        m_is_pointer_member (false)
    {
    }

    NameElement (const UString &a_name,
                 bool a_is_pointer,
                 bool a_is_pointer_member) :
        m_name (a_name) ,
        m_is_pointer (a_is_pointer),
        m_is_pointer_member (a_is_pointer_member)
    {
    }

    ~NameElement ()
    {
    }

    const UString& get_name () const {return m_name;}
    void set_name (const UString &a_name) {m_name = a_name;}

    bool is_pointer () const {return m_is_pointer;}
    void is_pointer (bool a_flag) {m_is_pointer = a_flag;}

    bool is_pointer_member () const {return m_is_pointer_member;}
    void is_pointer_member (bool a_flag) {m_is_pointer_member = a_flag;}
};//end NameElement

bool break_qname_into_name_elements (const UString &a_qname,
                                     std::list<NameElement> &a_name_elems) ;

bool get_variable_iter_from_qname
                    (const std::list<NameElement> &a_name_elems,
                     const std::list<NameElement>::const_iterator &a_cur_elem_it,
                     const Gtk::TreeModel::iterator &a_from_it,
                     Gtk::TreeModel::iterator &a_result) ;

bool get_variable_iter_from_qname (const UString &a_var_qname,
                                   const Gtk::TreeModel::iterator &a_from,
                                   Gtk::TreeModel::iterator &a_result) ;

void set_a_variable_type_real (Gtk::TreeModel::iterator &a_var_it,
                               const UString &a_type) ;

void update_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                             Gtk::TreeModel::iterator &a_iter,
                             Gtk::TreeView &a_tree_view,
                             bool a_handle_highlight,
                             bool a_is_new_frame) ;

void append_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                             const Gtk::TreeModel::iterator &a_parent,
                             Glib::RefPtr<Gtk::TreeStore> &a_tree_store,
                             Gtk::TreeView &a_tree_view,
                             IDebugger &a_debugger,
                             bool a_do_highlight,
                             bool a_is_new_frame,
                             Gtk::TreeModel::iterator &a_result) ;

bool update_a_variable (const IDebugger::VariableSafePtr &a_var,
                        Gtk::TreeView &a_tree_view,
                        bool a_is_new_frame,
                        Gtk::TreeModel::iterator &a_iter) ;

void append_a_variable (const IDebugger::VariableSafePtr &a_var,
                        const Gtk::TreeModel::iterator &a_parent,
                        Glib::RefPtr<Gtk::TreeStore> &a_tree_sore,
                        Gtk::TreeView &a_tree_view,
                        IDebugger &a_debugger,
                        bool a_do_highlight,
                        bool a_is_new_frame,
                        Gtk::TreeModel::iterator &a_result) ;

NEMIVER_END_NAMESPACE (variables_utils)
NEMIVER_END_NAMESPACE (nemiver)


#endif //__NMV_VARIABLES_UTILS_H__
