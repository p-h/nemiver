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
#ifndef __NMV_I_VAR_LIST_H__
#define __NMV_I_VAR_LIST_H__

#include <list>
#include "nmv-i-debugger.h"
#include "nmv-safe-ptr-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;
using nemiver::common::SafePtr ;
using nemiver::common::DynamicModule ;

class IVarList ;
typedef SafePtr<IVarList, ObjectRef, ObjectUnref> VarListSafePtr ;
class NEMIVER_API IVarList : public DynamicModule {
    IVarList () ;
    IVarList (const IVarList &) ;

public:

    /// \name signals
    ///@{
    virtual sigc::signal<void,IDebugger::VariableSafePtr&>& variable_added ()=0;
    virtual sigc::signal<void,IDebugger::VariableSafePtr&>& variable_removed ()=0;
    ///@}

    /// \brief intialize the interface with a given IDebugger pointer.
    /// This must be the function called on the interface before using it.
    /// Failing to initialize the interface with a valid non null IDebugger
    /// make subsequent invocation of the interface methods throw instances
    /// of nemiver::common::Exception.
    /// \param a_debugger the non null debugger interface used to query
    //// the variables.
    virtual void initialize (IDebuggerSafePtr &a_debugger) = 0;

    /// \return the raw list of variables maintained internally.
    virtual const std::list<IDebugger::VariableSafePtr>& get_raw_list () = 0 ;

    /// \brief append a variable to the list
    ///
    /// \param a_var the new variable to append
    virtual void append_variable (const IDebugger::VariableSafePtr &a_var) = 0 ;

    /// \brief remove a variable from the list
    ///
    /// \param a_var the variable to remove from the list
    virtual void remove_variable (const IDebugger::VariableSafePtr &a_var) = 0 ;

    /// \brief remove a variable from the list
    ///
    /// \param a_str the name of the variable to remove
    virtual void remove_variable (const UString &a_str) = 0 ;

    /// \brief update the state of the all the variables of the list
    ///
    /// issue many calls to the underlying IDebugger interface to query
    /// the new values of variables contained in the list
    virtual void update_state () = 0 ;
};//end class IVarList

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_I_VAR_LIST_H__

