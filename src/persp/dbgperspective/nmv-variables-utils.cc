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
#include "nmv-variables-utils.h"
#include "nmv-exception.h"
#include "nmv-ui-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (variables_utils)

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

bool
is_qname_a_pointer_member (const UString &a_qname)
{
    LOG_DD ("a_qname: " << a_qname) ;
    std::list<NameElement> name_elems ;

    if (!break_qname_into_name_elements (a_qname, name_elems)) {
        LOG_DD ("return false") ;
        return false ;
    }
    std::list<NameElement>::const_iterator end_it = name_elems.end () ;
    --end_it;
    if (end_it == name_elems.end ()) {
        LOG_DD ("return false") ;
        return false ;
    }

    LOG_DD ("result: " << (int) end_it->is_pointer_member ()) ;
    return end_it->is_pointer_member () ;
}

bool
break_qname_into_name_elements (const UString &a_qname,
                                std::list<NameElement> &a_name_elems)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD

    NEMIVER_TRY

    THROW_IF_FAIL (a_qname != "") ;
    LOG_D ("qname: '" << a_qname << "'", "break-qname-domain") ;

    UString::size_type len=a_qname.size (),cur=0, name_start=0, name_end=0 ;
    UString name_element ;
    bool is_pointer=false, is_pointer_member=false ;
    std::list<NameElement> name_elems ;

fetch_element:
    name_start = cur ;
    name_element="" ;
    gunichar c = 0 ;
    //okay, let's define what characters can be part of a variable name.
    //Note that variable names can be a template type -
    //in case a templated class extends another one, inline; in that
    //cases, the child templated class will have an anonymous member carring
    //the parent class's members.
    //this can looks funny sometimes.
    for (c = a_qname[cur];
         cur < len &&
         (c = a_qname[cur]) &&
         c != '-' &&
         (isalnum (c) ||
              c == '_' ||
              c == '<'/*anonymous names from templates*/ ||
              c == ':'/*fully qualified names*/ ||
              c == '>'/*templates again*/ ||
              c == '#'/*we can have names like '#unnamed#'*/ ||
              c == ','/*template parameters*/ ||
              c == '+' /*for arithmethic expressions*/ ||
              c == '*' /*ditto*/ ||
              c == '/' /*ditto*/ ||
              c == '(' /*ditto*/ ||
              c == ')' /*ditto*/ ||
              isspace (c)
         )
         ; ++cur) {
    }
    if (is_pointer) {
        is_pointer_member = true ;
    }
    if (cur == name_start) {
        name_element = "";
    } else if (cur >= len) {
        name_end = cur - 1 ;
        name_element = a_qname.substr (name_start, name_end - name_start + 1);
        is_pointer = false ;
    } else if (c == '.'
               || (cur + 1 < len
                   && c == '-'
                   && a_qname[cur+1] == '>')
               || cur >= len){
        name_end = cur - 1 ;
        name_element = a_qname.substr (name_start, name_end - name_start + 1);

        //update is_pointer state
        if (c != '.' || name_element[0] == '*') {
            is_pointer = true ;
        }

        //advance cur.
        if (c == '-') {
            //name_element = '*' + name_element ;
            cur += 2 ;
        } else if (cur < len){
            ++cur ;
        }
    } else {
        LOG_ERROR ("failed to parse name element. Index was: "
                   << (int) cur << " buf was '" << a_qname << "'") ;
        return false ;
    }
    name_element.chomp () ;
    LOG_D ("got name element: '" << name_element << "'", "break-qname-domain") ;
    LOG_D ("is_pointer: '" << (int) is_pointer << "'", "break-qname-domain") ;
    LOG_D ("is_pointer_member: '" << (int) is_pointer_member << "'",
           "break-qname-domain") ;
    name_elems.push_back (NameElement (name_element,
                                       is_pointer,
                                       is_pointer_member)) ;
    if (cur < len) {
        LOG_D ("go fetch next name element", "break-qname-domain") ;
        goto fetch_element ;
    }
    LOG_D ("getting out", "break-qname-domain") ;
    if (a_qname[0] == '*' && !name_elems.empty ()) {
        std::list<NameElement>::iterator it = name_elems.end () ;
        --it ;
        it->is_pointer (true) ;
    }
    a_name_elems = name_elems ;

    NEMIVER_CATCH_AND_RETURN (false)

    return true ;
}

bool
get_variable_iter_from_qname
                    (const std::list<NameElement> &a_name_elems,
                     const std::list<NameElement>::const_iterator &a_cur_elem_it,
                     const Gtk::TreeModel::iterator &a_from_it,
                     Gtk::TreeModel::iterator &a_result)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    THROW_IF_FAIL (!a_name_elems.empty ()) ;
    if (a_cur_elem_it != a_name_elems.end ()) {
        LOG_DD ("a_cur_elem_it: " << a_cur_elem_it->get_name ()) ;
        LOG_DD ("a_cur_elem_it->is_pointer: "
                << (int) a_cur_elem_it->is_pointer ()) ;
    } else {
        LOG_DD ("a_cur_elem_it: end") ;
    }

    if (!a_from_it) {
        LOG_ERROR ("got null a_from iterator") ;
        return false ;
    }
    std::list<NameElement>::const_iterator cur_elem_it = a_cur_elem_it;
    if (cur_elem_it == a_name_elems.end ()) {
        a_result = a_from_it ;
        LOG_DD ("found iter") ;
        return true;
    }

    Gtk::TreeModel::const_iterator row_it ;
    for (row_it = a_from_it->children ().begin () ;
         row_it != a_from_it->children ().end () ;
         ++row_it) {
        UString iter_name_col =
            (Glib::ustring)((*row_it)[get_variable_columns ().name]) ;
        LOG_DD ("current row_it: "
                << iter_name_col) ;

        if (cur_elem_it->is_pointer () == (iter_name_col[0] == '*')
            && iter_name_col == cur_elem_it->get_name ()) {
            LOG_DD ("walked to path element: '" << cur_elem_it->get_name ()) ;
            return get_variable_iter_from_qname (a_name_elems,
                                                 ++cur_elem_it,
                                                 row_it,
                                                 a_result) ;
        } else if (row_it->children ()
                   && row_it->children ().begin ()) {
            Gtk::TreeModel::iterator it = row_it->children ().begin () ;
            UString name_col =
                (Glib::ustring) (*it)[get_variable_columns ().name] ;
            std::list<NameElement> elems ;
            if (name_col != ""
                && break_qname_into_name_elements (name_col, elems)
                && !elems.empty ()) {
                std::list<NameElement>::const_iterator iter = elems.end () ;
                --iter ;
                name_col = iter->get_name () ;
            }
            LOG_DD ("testing against child row: " << name_col) ;
            if (name_col == ("*" + cur_elem_it->get_name ())
                || name_col == cur_elem_it->get_name ()) {
                LOG_DD ("walked to path element: '"
                         << name_col) ;
                return get_variable_iter_from_qname (a_name_elems,
                                                     ++cur_elem_it,
                                                     it,
                                                     a_result) ;
            }
        }
    }
    LOG_DD ("iter not found") ;
    return false ;
}

bool
get_variable_iter_from_qname (const UString &a_var_qname,
                              const Gtk::TreeModel::iterator &a_from,
                              Gtk::TreeModel::iterator &a_result)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    THROW_IF_FAIL (a_var_qname != "") ;
    LOG_DD ("a_var_qname: '" << a_var_qname << "'") ;

    if (!a_from) {
        LOG_ERROR ("got null a_from iterator") ;
        return false ;
    }
    std::list<NameElement> name_elems ;
    bool is_ok =
        variables_utils::break_qname_into_name_elements (a_var_qname,
                                                         name_elems) ;
    if (!is_ok) {
        LOG_ERROR ("failed to break qname into path elements") ;
        return false ;
    }

    bool ret = get_variable_iter_from_qname (name_elems,
                                             name_elems.begin (),
                                             a_from,
                                             a_result) ;
    if (!ret) {
        name_elems.clear () ;
        name_elems.push_back (a_var_qname) ;
        ret = get_variable_iter_from_qname (name_elems,
                                            name_elems.begin (),
                                            a_from,
                                            a_result) ;
    }
    return ret ;
}

void
set_a_variable_type_real (Gtk::TreeModel::iterator &a_var_it,
                          const UString &a_type)
{
    THROW_IF_FAIL (a_var_it) ;
    a_var_it->set_value (variables_utils::get_variable_columns ().type,
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

    a_var_it->set_value (variables_utils::get_variable_columns ().type_caption,
                         (Glib::ustring)type_caption) ;
    IDebugger::VariableSafePtr variable =
        (IDebugger::VariableSafePtr) a_var_it->get_value
                                        (get_variable_columns ().variable);
    THROW_IF_FAIL (variable) ;
    variable->type (a_type) ;
}

void
update_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                        Gtk::TreeModel::iterator &a_iter,
                        Gtk::TreeView &a_tree_view,
                        bool a_handle_highlight,
                        bool a_is_new_frame)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    if (a_var) {
        LOG_DD ("going to really update variable '" << a_var->name () << "'") ;
    } else {
        LOG_DD ("eek, got null variable") ;
    }

    (*a_iter)[get_variable_columns ().variable] = a_var ;
    UString var_name = a_var->name ();
    var_name.chomp () ;
    UString prev_var_name =
            (Glib::ustring)(*a_iter)[get_variable_columns ().name] ;
    LOG_DD ("Prev variable name: " << prev_var_name) ;
    LOG_DD ("new variable name: " << var_name) ;
    LOG_DD ("Didn't update variable name") ;
    if (prev_var_name == "") {
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
        (*a_iter)[get_variable_columns ().is_highlighted]  = true;
        (*a_iter)[get_variable_columns ().fg_color]  = Gdk::Color ("red");
    } else {
        LOG_DD ("remove highlight from variable") ;
        (*a_iter)[get_variable_columns ().is_highlighted]  = false ;
        (*a_iter)[get_variable_columns ().fg_color]  =
            a_tree_view.get_style ()->get_fg (Gtk::STATE_NORMAL);
    }

    (*a_iter)[get_variable_columns ().value] = a_var->value () ;
    (*a_iter)[get_variable_columns ().type] = a_var->type () ;
}

void
append_a_variable_real (const IDebugger::VariableSafePtr &a_var,
                        const Gtk::TreeModel::iterator &a_parent,
                        Glib::RefPtr<Gtk::TreeStore> &a_tree_store,
                        Gtk::TreeView &a_tree_view,
                        IDebugger &a_debugger,
                        bool a_do_highlight,
                        bool a_is_new_frame,
                        Gtk::TreeModel::iterator &a_result)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    THROW_IF_FAIL (a_var) ;
    LOG_DD ("going to append variable: '" << a_var->name () << "'") ;

    Gtk::TreeModel::iterator cur_row_it;
    if (a_parent) {
        cur_row_it = a_parent->children ().begin () ;
    } else {
        cur_row_it = a_tree_store->append () ;
    }

    if (!cur_row_it
        ||(Glib::ustring)
            cur_row_it->get_value (get_variable_columns ().name) != "") {
        cur_row_it = a_tree_store->append (a_parent->children ()) ;
    }
    THROW_IF_FAIL (cur_row_it) ;
    update_a_variable_real (a_var,
                            cur_row_it,
                            a_tree_view,
                            a_do_highlight,
                            a_is_new_frame) ;
    UString qname ;
    a_var->build_qname (qname) ;
    LOG_DD ("set variable with qname '" << qname << "'") ;
    a_debugger.print_variable_type (qname) ;
    LOG_DD ("did query type of variable '" << qname << "'") ;
    a_result = cur_row_it ;
}

void
update_a_variable (const IDebugger::VariableSafePtr &a_var,
                   Gtk::TreeView &a_tree_view,
                   bool a_is_new_frame,
                   Gtk::TreeModel::iterator &a_iter)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    NEMIVER_TRY

    if (a_var) {
        LOG_DD ("going to update variable: '" << a_var->name () << "'") ;
    }
    update_a_variable_real (a_var, a_iter, a_tree_view, true, a_is_new_frame) ;

    if (a_var->members ().empty ()) {return;}

    std::list<IDebugger::VariableSafePtr>::const_iterator member ;
    for (member = a_var->members ().begin ();
         member != a_var->members ().end ();
         ++member) {

        THROW_IF_FAIL (*member) ;

        //hmmh, this makes the current function have an O(n!) exec time.
        //Doing it in less time is too difficult for the time being.
        Gtk::TreeModel::iterator tree_node_iter ;
        if (!get_variable_iter_from_qname ((*member)->name (),
                                           a_iter,
                                           tree_node_iter)) {
            THROW (UString ("Could not find tree node for variable '")
                   + (*member)->name ()
                   + "'. Parent var was: '"
                   + a_var->name ()
                   + "'") ;
        }
        THROW_IF_FAIL (tree_node_iter) ;

        update_a_variable (*member, a_tree_view, a_is_new_frame, tree_node_iter) ;
    }//end for.

    NEMIVER_CATCH
}

void
append_a_variable (const IDebugger::VariableSafePtr &a_var,
                   const Gtk::TreeModel::iterator &a_parent,
                   Glib::RefPtr<Gtk::TreeStore> &a_tree_store,
                   Gtk::TreeView &a_tree_view,
                   IDebugger &a_debugger,
                   bool a_do_highlight,
                   bool a_is_new_frame,
                   Gtk::TreeModel::iterator &a_result)
{

    LOG_FUNCTION_SCOPE_NORMAL_DD ;

    NEMIVER_TRY

    if (a_result) {}
    Gtk::TreeModel::iterator result_iter, tmp_iter;

    append_a_variable_real (a_var,
                            a_parent,
                            a_tree_store,
                            a_tree_view,
                            a_debugger,
                            a_do_highlight,
                            a_is_new_frame,
                            result_iter);

    if (a_var->members ().empty ()) {
        a_result = result_iter ;
        return;
    }

    std::list<IDebugger::VariableSafePtr>::const_iterator member_iter ;
    for (member_iter = a_var->members ().begin ();
         member_iter != a_var->members ().end ();
         ++member_iter) {
        append_a_variable (*member_iter, result_iter,
                           a_tree_store, a_tree_view, a_debugger,
                           a_do_highlight, a_is_new_frame, tmp_iter) ;
    }
    a_result = result_iter ;

    NEMIVER_CATCH
}

NEMIVER_END_NAMESPACE (variables_utils)
NEMIVER_END_NAMESPACE (nemiver)

