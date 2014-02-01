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
#include <algorithm>
#include <list>
#include "nmv-i-var-list.h"
#include "common/nmv-exception.h"

static const char* INSTANCE_NOT_INITIALIZED = "instance not initialized";
static const char* VAR_LIST_COOKIE = "var-list-cookie";

#ifndef CHECK_INIT
#define CHECK_INIT THROW_IF_FAIL2 (m_debugger, INSTANCE_NOT_INITIALIZED)
#endif

NEMIVER_BEGIN_NAMESPACE (nemiver)

using nemiver::common::DynamicModule;
using nemiver::common::DynamicModuleSafePtr;
using nemiver::common::DynModIface;
using nemiver::common::DynModIfaceSafePtr;

class NameElement {
    UString m_name;
    bool m_is_pointer;
    bool m_is_pointer_member;

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
};//end class NameElement
typedef std::list<NameElement> NameElementList;

//******************************
//<static function declarations>
//******************************

static bool break_qname_into_name_elements (const UString &a_qname,
                                            NameElementList &a_name_elems);


//******************************
//</static function declarations>
//******************************


//******************************
//<static function definitions>
//******************************

/*
static bool
is_qname_a_pointer_member (const UString &a_qname)
{
    LOG_DD ("a_qname: " << a_qname);
    std::list<NameElement> name_elems;

    if (!break_qname_into_name_elements (a_qname, name_elems)) {
        LOG_DD ("return false");
        return false;
    }
    std::list<NameElement>::const_iterator end_it = name_elems.end ();
    --end_it;
    if (end_it == name_elems.end ()) {
        LOG_DD ("return false");
        return false;
    }

    LOG_DD ("result: " << (int) end_it->is_pointer_member ());
    return end_it->is_pointer_member ();
}
*/

static bool
break_qname_into_name_elements (const UString &a_qname,
                                NameElementList &a_name_elems)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD

    NEMIVER_TRY

    THROW_IF_FAIL (a_qname != "");
    LOG_D ("qname: '" << a_qname << "'", "break-qname-domain");

    UString::size_type len = a_qname.size (),cur=0, name_start=0, name_end=0;
    UString name_element;
    bool is_pointer=false, is_pointer_member=false;
    NameElementList name_elems;

fetch_element:
    name_start = cur;
    name_element="";
    gunichar c = 0;
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
        is_pointer_member = true;
    }
    if (cur == name_start) {
        name_element = "";
    } else if (cur >= len) {
        name_end = cur - 1;
        name_element = a_qname.substr (name_start, name_end - name_start + 1);
        is_pointer = false;
    } else if (c == '.'
               || (cur + 1 < len
                   && c == '-'
                   && a_qname[cur+1] == '>')
               || cur >= len){
        name_end = cur - 1;
        name_element = a_qname.substr (name_start, name_end - name_start + 1);

        //update is_pointer state
        if (c != '.' || name_element[0] == '*') {
            is_pointer = true;
        }

        //advance cur.
        if (c == '-') {
            //name_element = '*' + name_element;
            cur += 2;
        } else if (cur < len){
            ++cur;
        }
    } else {
        LOG_ERROR ("failed to parse name element. Index was: "
                   << (int) cur << " buf was '" << a_qname << "'");
        return false;
    }
    name_element.chomp ();
    LOG_D ("got name element: '" << name_element << "'", "break-qname-domain");
    LOG_D ("is_pointer: '" << (int) is_pointer << "'", "break-qname-domain");
    LOG_D ("is_pointer_member: '" << (int) is_pointer_member << "'",
           "break-qname-domain");
    name_elems.push_back (NameElement (name_element,
                                       is_pointer,
                                       is_pointer_member));
    if (cur < len) {
        LOG_D ("go fetch next name element", "break-qname-domain");
        goto fetch_element;
    }
    LOG_D ("getting out", "break-qname-domain");
    if (a_qname[0] == '*' && !name_elems.empty ()) {
        NameElementList::iterator it = name_elems.end ();
        --it;
        it->is_pointer (true);
    }
    a_name_elems = name_elems;

    NEMIVER_CATCH_AND_RETURN_NOX (false)

    return true;
}
//******************************
//</static function definitions>
//******************************

class VarList : public IVarList {
    sigc::signal<void, const IDebugger::VariableSafePtr&>
                                                    m_variable_added_signal;
    sigc::signal<void, const IDebugger::VariableSafePtr>
                                                m_variable_value_set_signal;
    sigc::signal<void, const IDebugger::VariableSafePtr&>
                                                m_variable_type_set_signal;
    sigc::signal<void, const IDebugger::VariableSafePtr&>
                                                m_variable_updated_signal;
    sigc::signal<void, const IDebugger::VariableSafePtr&>
                                                    m_variable_removed_signal;

    DebuggerVariableList m_raw_list;
    IDebuggerSafePtr m_debugger;

public:
    VarList (DynamicModule *a_dynmod) :
        IVarList (a_dynmod)
    {
    }

public:

    //*******************
    //<events getters>
    //*******************
    sigc::signal<void, const IDebugger::VariableSafePtr&>&
                                                    variable_added_signal ()
    {
        return m_variable_added_signal;
    }

    sigc::signal<void, const IDebugger::VariableSafePtr>&
                                                variable_value_set_signal ()
    {
        return m_variable_value_set_signal;
    }

    sigc::signal<void, const IDebugger::VariableSafePtr&>&
                                                variable_type_set_signal ()
    {
        return m_variable_type_set_signal;
    }

    sigc::signal<void, const IDebugger::VariableSafePtr&>&
                                                variable_updated_signal ()
    {
        return m_variable_updated_signal;
    }

    sigc::signal<void, const IDebugger::VariableSafePtr&>&
                                                    variable_removed_signal ()
    {
        return m_variable_removed_signal;
    }
    //*******************
    //</events getters>
    //*******************

    //***************
    //<signal slots>
    //***************
    void on_variable_type_set_signal (const IDebugger::VariableSafePtr &a_var,
                                      const UString &a_cookie);

    void on_variable_value_set_signal (const IDebugger::VariableSafePtr &a_var,
                                       const UString &a_cookie);
    //***************
    //</signal slots>
    //***************

    void initialize (IDebuggerSafePtr &a_debugger);

    IDebugger& get_debugger () const
    {
        CHECK_INIT;
        return *m_debugger;
    }

    const DebuggerVariableList& get_raw_list () const;

    void append_variable (const IDebugger::VariableSafePtr &a_var,
                          bool a_update_type);

    void append_variables (const DebuggerVariableList& a_vars,
                           bool a_update_type);

    bool remove_variable (const IDebugger::VariableSafePtr &a_var);

    bool remove_variable (const UString &a_var_name);

    void remove_variables ();

    bool find_variable (const UString &a_var_name,
                        IDebugger::VariableSafePtr &a_var);

    bool find_variable_from_qname
                        (const NameElementList &a_name_elems,
                         const NameElementList::const_iterator &a_cur_elem_it,
                         const DebuggerVariableList::iterator &a_from_it,
                         IDebugger::VariableSafePtr &a_result);

    bool find_variable_in_tree
                        (const NameElementList &a_name_elems,
                         const NameElementList::const_iterator &a_cur_elem_it,
                         const IDebugger::VariableSafePtr &a_from_it,
                         IDebugger::VariableSafePtr &a_result);

    bool find_variable_from_qname
                    (const UString &a_qname,
                     const DebuggerVariableList::iterator &a_from,
                     IDebugger::VariableSafePtr &a_var);


    bool find_variable_from_qname (const UString &a_qname,
                                   IDebugger::VariableSafePtr &a_var);

    bool update_variable (const UString &a_var_name,
                          const IDebugger::VariableSafePtr &a_new_var_value);

    void update_state ();
};//end class VarList

void
VarList::on_variable_type_set_signal (const IDebugger::VariableSafePtr &a_var,
                                      const UString &a_cookie)
{
    if (a_cookie != VAR_LIST_COOKIE) {return;}

    NEMIVER_TRY

    THROW_IF_FAIL (a_var && a_var->name () != "" && a_var->type () != "");

    IDebugger::VariableSafePtr variable;
    THROW_IF_FAIL (find_variable (a_var->name (), variable));
    THROW_IF_FAIL (variable == a_var);
    THROW_IF_FAIL (variable->type () != "");

    variable_type_set_signal ().emit (a_var);

    NEMIVER_CATCH_NOX
}

void
VarList::on_variable_value_set_signal (const IDebugger::VariableSafePtr &a_var,
                                       const UString &a_cookie)
{
    NEMIVER_TRY

    if (a_cookie != VAR_LIST_COOKIE) {return;}

    THROW_IF_FAIL (update_variable (a_var->name (), a_var));

    variable_value_set_signal ().emit (a_var);
    variable_updated_signal ().emit (a_var);

    NEMIVER_CATCH_NOX
}

void
VarList::initialize (IDebuggerSafePtr &a_debugger)
{
    m_debugger = a_debugger;

    THROW_IF_FAIL (m_debugger);
    m_debugger->variable_type_set_signal ().connect (sigc::mem_fun
                                (*this, &VarList::on_variable_type_set_signal));
    m_debugger->variable_value_set_signal ().connect (sigc::mem_fun
                            (*this, &VarList::on_variable_value_set_signal));
}

const DebuggerVariableList&
VarList::get_raw_list () const
{
    CHECK_INIT;
    return m_raw_list;
}

void
VarList::append_variable (const IDebugger::VariableSafePtr &a_var,
                          bool a_update_type)
{
    CHECK_INIT;
    THROW_IF_FAIL (a_var);
    THROW_IF_FAIL (a_var->name () != "");

    m_raw_list.push_back (a_var);
    if (a_update_type) {
        get_debugger ().get_variable_type (a_var, VAR_LIST_COOKIE);
    }
    variable_added_signal ().emit (a_var);
}

void
VarList::append_variables (const DebuggerVariableList& a_vars,
                           bool a_update_type)
{
    CHECK_INIT;

    DebuggerVariableList::const_iterator var_iter;
    for (var_iter = a_vars.begin (); var_iter != a_vars.end (); ++ var_iter) {
        append_variable (*var_iter, a_update_type);
    }
}

bool
VarList::remove_variable (const IDebugger::VariableSafePtr &a_var)
{
    CHECK_INIT;
    DebuggerVariableList::iterator result_iter =
                    std::find (m_raw_list.begin (), m_raw_list.end (), a_var);
    if (result_iter == get_raw_list ().end ()) {
        return false;
    }

    IDebugger::VariableSafePtr variable = *result_iter;
    m_raw_list.erase (result_iter);
    variable_removed_signal ().emit (variable);

    return true;
}

bool
VarList::remove_variable (const UString &a_var_name)
{
    CHECK_INIT;

    DebuggerVariableList::iterator iter;
    for (iter = m_raw_list.begin (); iter != m_raw_list.end () ; ++iter) {
        if (!(*iter)) {
            continue;
        }
        if ((*iter)->name () == a_var_name) {
            IDebugger::VariableSafePtr variable = *iter;
            m_raw_list.erase (iter);
            variable_removed_signal ().emit (variable);
            return true;
        }
    }
    return false;
}

void
VarList::remove_variables ()
{
    CHECK_INIT;

    DebuggerVariableList::iterator var_iter;

    if (m_raw_list.empty ()) {return;}

    for (var_iter = m_raw_list.begin ();
         var_iter != m_raw_list.end ();
         var_iter = m_raw_list.begin ()) {
        remove_variable (*var_iter);
    }
}

bool
VarList::find_variable (const UString &a_var_name,
                        IDebugger::VariableSafePtr &a_var)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    CHECK_INIT;
    return VarList::find_variable_from_qname (a_var_name,
                                              m_raw_list.begin (),
                                              a_var);
}

bool
VarList::find_variable_from_qname
                (const UString &a_var_qname,
                 const DebuggerVariableList::iterator &a_from,
                 IDebugger::VariableSafePtr &a_result)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    CHECK_INIT;

    THROW_IF_FAIL (a_var_qname != "");
    LOG_DD ("a_var_qname: '" << a_var_qname << "'");

    if (a_from == m_raw_list.end ()) {
        LOG_ERROR ("got null a_from iterator");
        return false;
    }
    std::list<NameElement> name_elems;
    bool is_ok = break_qname_into_name_elements (a_var_qname, name_elems);
    if (!is_ok) {
        LOG_ERROR ("failed to break qname into path elements");
        return false;
    }
    bool ret = find_variable_from_qname (name_elems,
                                         name_elems.begin (),
                                         a_from,
                                         a_result);
    if (!ret) {
        name_elems.clear ();
        name_elems.push_back (a_var_qname);
        ret = find_variable_from_qname (name_elems,
                                        name_elems.begin (),
                                        a_from,
                                        a_result);
    }
    return ret;
}

bool
VarList::find_variable_from_qname
                        (const NameElementList &a_name_elems,
                         const NameElementList::const_iterator &a_cur_elem_it,
                         const DebuggerVariableList::iterator &a_from_it,
                         IDebugger::VariableSafePtr &a_result)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    CHECK_INIT;

    THROW_IF_FAIL (!a_name_elems.empty ());
    if (a_cur_elem_it != a_name_elems.end ()) {
        LOG_DD ("a_cur_elem_it: " << a_cur_elem_it->get_name ());
        LOG_DD ("a_cur_elem_it->is_pointer: "
                << (int) a_cur_elem_it->is_pointer ());
    } else {
        LOG_DD ("a_cur_elem_it: end");
    }

    if (a_from_it == m_raw_list.end ()) {
        LOG_ERROR ("got null a_from iterator");
        return false;
    }
    std::list<NameElement>::const_iterator cur_elem_it = a_cur_elem_it;
    if (cur_elem_it == a_name_elems.end ()) {
        a_result = *a_from_it;
        LOG_DD ("found iter");
        return true;
    }

    DebuggerVariableList::iterator var_iter;
    for (var_iter = a_from_it;
         var_iter != m_raw_list.end ();
         ++var_iter) {
        UString var_name = (*var_iter)->name ();
        LOG_DD ("current var list iter name: " << var_name);
        if (var_name == cur_elem_it->get_name ()) {
            LOG_DD ("walked to path element: '" << cur_elem_it->get_name ());
            bool res = find_variable_in_tree (a_name_elems,
                                              cur_elem_it,
                                              *var_iter,
                                              a_result);
            if (res) {
                return true;
            }
        }
    }
    LOG_DD ("iter not found");
    return false;
}

bool
VarList::find_variable_in_tree
                    (const NameElementList &a_name_elems,
                     const NameElementList::const_iterator &a_cur_elem_it,
                     const IDebugger::VariableSafePtr &a_from,
                     IDebugger::VariableSafePtr &a_result)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    CHECK_INIT;

    THROW_IF_FAIL (a_from);

    if (a_cur_elem_it == a_name_elems.end ()) {
        LOG_DD ("reached end of name elements' list, returning false");
        return false;
    }

    IDebugger::VariableSafePtr variable = a_from;
    NameElementList::const_iterator name_elem_it = a_cur_elem_it;
    if (variable->name () != name_elem_it->get_name ()) {
        LOG_DD ("variable name doesn't match name element, returning false");
        return false;
    }

    UString str; variable->to_string (str) ;
    LOG_DD ("inspecting variable: '"
            << variable->name ()
            << "' |content->|"
            << str);

    ++name_elem_it;
    if (name_elem_it == a_name_elems.end ()) {
        a_result = variable;
        LOG_DD ("Found variable, returning true");
        return true;
    } else if (variable->members ().empty ()) {
        LOG_DD ("reached end of variable tree without finding a matching one, "
                "returning false");
        return false;
    } else {
        std::list<IDebugger::VariableSafePtr>::const_iterator var_it;
        for (var_it = variable->members ().begin ();
             var_it != variable->members ().end ();
             ++var_it) {
            if ((*var_it)->name () == name_elem_it->get_name ()) {
                bool res = find_variable_in_tree (a_name_elems,
                                                  name_elem_it,
                                                  *var_it,
                                                  a_result);
                if (res) {
                    LOG_DD ("Found the variable, returning true");
                    return true;
                }
            }
        }
    }
    LOG_DD ("Didn't found the variable, returning false");
    return false;
}

bool
VarList::find_variable_from_qname (const UString &a_qname,
                                   IDebugger::VariableSafePtr &a_var)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    CHECK_INIT;

    THROW_IF_FAIL (a_qname != "");
    LOG_DD ("a_qname: '" << a_qname << "'");

    std::list<NameElement> name_elems;
    bool ret = break_qname_into_name_elements (a_qname, name_elems);
    if (!ret) {
        LOG_ERROR ("failed to break qname into path elements");
        return false;
    }
    ret = find_variable_from_qname (name_elems,
                                    name_elems.begin (),
                                    m_raw_list.begin (),
                                    a_var);
    return ret;
}

bool
VarList::update_variable (const UString &a_var_name,
                          const IDebugger::VariableSafePtr &a_new_var_value)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    CHECK_INIT;

    DebuggerVariableList::iterator var_it;
    for (var_it = m_raw_list.begin ();
         var_it != m_raw_list.end ();
         ++var_it) {
        if (!(*var_it)) {continue;}
        if (a_var_name == (*var_it)->name ()) {
            *var_it = a_new_var_value;
            return true;
        }
    }
    return false;
}

void
VarList::update_state ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    CHECK_INIT;

    DebuggerVariableList::iterator var_it;
    for (var_it = m_raw_list.begin ();
         var_it != m_raw_list.end ();
         ++var_it) {
        if (!(*var_it) || (*var_it)->name () == "") {
            continue;
        }
        get_debugger ().get_variable_value (*var_it, VAR_LIST_COOKIE);
    }
}

//the dynmod used to instanciate the VarList service object
//and return an interface on it.
class VarListDynMod : public  DynamicModule {

    void get_info (Info &a_info) const
    {
        const static Info s_info ("variablelist",
                                  "The Variable Model dynmod. "
                                  "Implements the IVarList interface",
                                  "1.0");
        a_info = s_info;
    }

    void do_init ()
    {
    }

    bool lookup_interface (const std::string &a_iface_name,
                           DynModIfaceSafePtr &a_iface)
    {
        if (a_iface_name == "IVarList") {
            a_iface.reset (new VarList (this));
        } else {
            return false;
        }
        return true;
    }
};//end class varListDynMod

NEMIVER_END_NAMESPACE (nemiver)


//the dynmod initial factory.
extern "C" {

bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    *a_new_instance = new nemiver::VarListDynMod ();
    return (*a_new_instance != 0);
}

}

