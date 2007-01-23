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
#include <algorithm>
#include "nmv-i-var-list.h"
#include "nmv-exception.h"

static const char* INSTANCE_NOT_INITIALIZED = "instance not initialized" ;

#ifndef CHECK_INIT
#define CHECK_INIT THROW_IF_FAIL2 (m_debugger, INSTANCE_NOT_INITIALIZED)
#endif

NEMIVER_BEGIN_NAMESPACE (nemiver)

using nemiver::common::DynamicModule ;
using nemiver::common::DynamicModuleSafePtr ;

class VarList : public IVarList {
    sigc::signal<void,IDebugger::VariableSafePtr&> m_variable_added_signal ;
    sigc::signal<void,IDebugger::VariableSafePtr&> m_variable_removed_signal ;

    typedef std::list<IDebugger::VariableSafePtr> VariableList ;
    VariableList m_raw_list ;
    IDebuggerSafePtr m_debugger ;

public:
    VarList (DynamicModule *a_dynmod) :
        IVarList (a_dynmod)
    {
    }

public:

    //*******************
    //<events getters>
    //*******************
    sigc::signal<void,IDebugger::VariableSafePtr&>& variable_added_signal ()
    {
        return m_variable_added_signal ;
    }

    sigc::signal<void,IDebugger::VariableSafePtr&>& variable_removed_signal ()
    {
        return m_variable_removed_signal ;
    }
    //*******************
    //</events getters>
    //*******************

    IDebugger& get_debugger () const
    {
        CHECK_INIT ;
        return *m_debugger ;
    }

    void initialize (IDebuggerSafePtr &a_debugger) ;

    const std::list<IDebugger::VariableSafePtr>& get_raw_list () const ;

    void append_variable (const IDebugger::VariableSafePtr &a_var) ;

    bool remove_variable (const IDebugger::VariableSafePtr &a_var);

    bool remove_variable (const UString &a_var_name) ;

    bool find_variable (const UString &a_var_name,
                        IDebugger::VariableSafePtr &a_var) ;

    void update_state () ;
};//end class VarList

void
VarList::initialize (IDebuggerSafePtr &a_debugger)
{
    m_debugger = a_debugger ;
}

const std::list<IDebugger::VariableSafePtr>&
VarList::get_raw_list () const
{
    CHECK_INIT ;
    return m_raw_list ;
}

void
VarList::append_variable (const IDebugger::VariableSafePtr &a_var)
{
    CHECK_INIT ;
    THROW_IF_FAIL (a_var) ;

    m_raw_list.push_back (a_var) ;
}

bool
VarList::remove_variable (const IDebugger::VariableSafePtr &a_var)
{
    CHECK_INIT ;
    VariableList::iterator result_iter = std::find (m_raw_list.begin (),
                                                    m_raw_list.end (),
                                                    a_var) ;
    if (result_iter == get_raw_list ().end ()) {
        return false;
    }
    m_raw_list.erase (result_iter) ;
    return true ;
}

bool
VarList::remove_variable (const UString &a_var_name)
{
    CHECK_INIT ;

    VariableList::iterator iter ;
    for (iter = m_raw_list.begin () ; iter != m_raw_list.end () ; ++iter) {
        if (!(*iter)) {
            continue ;
        }
        if ((*iter)->name () == a_var_name) {
            m_raw_list.erase (iter) ;
            return true;
        }
    }
    return false ;
}

bool
VarList::find_variable (const UString &a_var_name,
                        IDebugger::VariableSafePtr &a_var)
{
    CHECK_INIT ;
    VariableList::iterator iter ;
    for (iter = m_raw_list.begin () ; iter != m_raw_list.end () ; ++iter) {
        if (!(*iter)) {
            continue ;
        }
        if ((*iter)->name () == a_var_name) {
            a_var = *iter ;
            return true ;
        }
    }
    return false ;
}

void
VarList::update_state ()
{
    CHECK_INIT ;
    //TODO: finish this !
}

NEMIVER_END_NAMESPACE (nemiver)

//TODO: finish this, i.e, writing the dynmod

//the dynmod initial factory.
extern "C" {

bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    //*a_new_instance = new nemiver::VarListDynMod () ;
    return (*a_new_instance != 0) ;
}

}

