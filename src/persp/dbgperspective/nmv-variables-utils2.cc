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

#include "config.h"

#ifdef WITH_VARIABLE_WALKER

#include "nmv-variables-utils2.h"
#include "common/nmv-exception.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (variables_utils2)

static void update_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                                    const Gtk::TreeView &a_tree_view,
                                    Gtk::TreeModel::iterator &a_row_it,
                                    bool a_handle_highlight,
                                    bool a_is_new_frame) ;

VariableColumns&
get_variable_columns ()
{
    static VariableColumns s_cols ;
    return s_cols ;
}

bool
is_type_a_pointer (const UString &a_type)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    LOG_DD ("type: '" << a_type << "'") ;

    UString type (a_type);
    type.chomp () ;
    if (type[type.size () - 1] == '*') {
        LOG_DD ("type is a pointer") ;
        return true ;
    }
    if (type.size () < 8) {
        LOG_DD ("type is not a pointer") ;
        return false ;
    }
    UString::size_type i = type.size () - 7 ;
    if (!a_type.compare (i, 7, "* const")) {
        LOG_DD ("type is a pointer") ;
        return true ;
    }
    LOG_DD ("type is not a pointer") ;
    return false ;
}

void
set_a_variable_node_type (Gtk::TreeModel::iterator &a_var_it,
                          const UString &a_type)
{
    THROW_IF_FAIL (a_var_it) ;
    a_var_it->set_value (get_variable_columns ().type,
                         (Glib::ustring)a_type) ;
    int nb_lines = a_type.get_number_of_lines () ;
    UString type_caption = a_type ;
    if (nb_lines) {--nb_lines;}

    UString::size_type truncation_index = 0 ;
    static const UString::size_type MAX_TYPE_STRING_LENGTH = 15 ;
    if (nb_lines) {
        truncation_index = a_type.find ('\n') ;
    } else if (a_type.size () > MAX_TYPE_STRING_LENGTH) {
        truncation_index = MAX_TYPE_STRING_LENGTH ;
    }
    if (truncation_index) {
        type_caption.erase (truncation_index) ;
        type_caption += "..." ;
    }

    a_var_it->set_value (get_variable_columns ().type_caption,
                         (Glib::ustring)type_caption) ;
    IDebugger::VariableSafePtr variable =
        (IDebugger::VariableSafePtr) a_var_it->get_value
                                        (get_variable_columns ().variable);
    THROW_IF_FAIL (variable) ;
    variable->type (a_type) ;
}

void
update_a_variable_node (const IDebugger::VariableSafePtr &a_var,
                        const Gtk::TreeView &a_tree_view,
                        Gtk::TreeModel::iterator &a_iter,
                        bool a_handle_highlight,
                        bool a_is_new_frame)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    if (a_var) {
        LOG_DD ("going to really update variable '" << a_var->name () << "'") ;
    } else {
        LOG_DD ("eek, got null variable") ;
        return ;
    }

    (*a_iter)[get_variable_columns ().variable] = a_var ;
    UString var_name = a_var->name ();
    var_name.chomp () ;
    UString prev_var_name =
            (Glib::ustring)(*a_iter)[get_variable_columns ().name] ;
    LOG_DD ("Prev variable name: " << prev_var_name) ;
    LOG_DD ("new variable name: " << var_name) ;
    LOG_DD ("Didn't update variable name") ;
    if (prev_var_name.raw () == "") {
        (*a_iter)[get_variable_columns ().name] = var_name;
    }
    (*a_iter)[get_variable_columns ().is_highlighted]=false ;
    bool do_highlight = false ;
    if (a_handle_highlight && !a_is_new_frame) {
        UString prev_value =
            (UString) (*a_iter)[get_variable_columns ().value] ;
        if (prev_value != a_var->value ()) {
            do_highlight = true ;
        }
    }

    if (do_highlight) {
        LOG_DD ("do highlight variable") ;
        (*a_iter)[get_variable_columns ().is_highlighted]=true;
        (*a_iter)[get_variable_columns ().fg_color] = Gdk::Color ("red");
    } else {
        LOG_DD ("remove highlight from variable") ;
        (*a_iter)[get_variable_columns ().is_highlighted]=false;
        (*a_iter)[get_variable_columns ().fg_color]  =
            a_tree_view.get_style ()->get_text (Gtk::STATE_NORMAL);
    }

    (*a_iter)[get_variable_columns ().value] = a_var->value () ;
    set_a_variable_node_type (a_iter,  a_var->type ()) ;
}

void
update_a_variable (const IDebugger::VariableSafePtr &a_var,
                   const Gtk::TreeView &a_tree_view,
                   Gtk::TreeModel::iterator &a_parent_row_it,
                   bool a_handle_highlight,
                   bool a_is_new_frame)
{
        LOG_FUNCTION_SCOPE_NORMAL_DD ;
        THROW_IF_FAIL (a_parent_row_it) ;

        Gtk::TreeModel::iterator row_it ;
        bool found=false;
        for (row_it = a_parent_row_it->children ().begin ();
             row_it != a_parent_row_it->children ().end ();
             ++row_it) {
            IDebugger::VariableSafePtr var =
                row_it->get_value (get_variable_columns ().variable);
            if (!var) {
                LOG_ERROR ("hit a null variable") ;
                continue ;
            }
            LOG_DD ("reading var: " << var->name ()) ;
            if (var->name ().raw () == a_var->name ().raw () &&
                var->type ().raw () == a_var->type ().raw ()) {
                found = true ;
                break ;
            }
        }
        if (!found) {
            THROW ("could not find variable in inspector: " + a_var->name ()) ;
        }
        update_a_variable_real (a_var,
                                a_tree_view,
                                row_it,
                                a_handle_highlight,
                                a_is_new_frame) ;
}

static void
update_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                        const Gtk::TreeView &a_tree_view,
                        Gtk::TreeModel::iterator &a_row_it,
                        bool a_handle_highlight,
                        bool a_is_new_frame)
{
    update_a_variable_node (a_var,
                            a_tree_view,
                            a_row_it,
                            a_handle_highlight,
                            a_is_new_frame) ;
    Gtk::TreeModel::iterator row_it;
    list<IDebugger::VariableSafePtr>::const_iterator var_it;
    Gtk::TreeModel::Children rows = a_row_it->children ();
    //TODO: change this to handle dereferencing
    for (row_it = rows.begin (), var_it = a_var->members ().begin ();
         row_it != rows.end ();
         ++row_it, ++var_it) {
        update_a_variable_real (*var_it, a_tree_view,
                                row_it, a_handle_highlight,
                                a_is_new_frame) ;
    }
}

void
append_a_variable (const IDebugger::VariableSafePtr &a_var,
                   const Gtk::TreeView &a_tree_view,
                   Glib::RefPtr<Gtk::TreeStore> &a_tree_store,
                   Gtk::TreeModel::iterator &a_parent_row_it)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    Gtk::TreeModel::iterator row_it ;
    append_a_variable (a_var, a_tree_view, a_tree_store,
                       a_parent_row_it, row_it) ;
}

void
append_a_variable (const IDebugger::VariableSafePtr &a_var,
                   const Gtk::TreeView &a_tree_view,
                   Glib::RefPtr<Gtk::TreeStore> &a_tree_store,
                   Gtk::TreeModel::iterator &a_parent_row_it,
                   Gtk::TreeModel::iterator &a_result)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;
    THROW_IF_FAIL (a_tree_store) ;

    Gtk::TreeModel::iterator row_it ;
    if (!a_parent_row_it) {
        row_it = a_tree_store->append () ;
    } else {
        row_it = a_tree_store->append (a_parent_row_it->children ()) ;
    }
    update_a_variable_node (a_var, a_tree_view, row_it, true, true) ;
    list<IDebugger::VariableSafePtr>::const_iterator it;
    for (it = a_var->members ().begin (); it != a_var->members ().end (); ++it) {
        append_a_variable (*it, a_tree_view, a_tree_store, row_it) ;
    }
    a_result = row_it ;
}


NEMIVER_END_NAMESPACE (variables_utils2)
NEMIVER_END_NAMESPACE (nemiver)

#endif //WITH_VARIABLE_WALKER

