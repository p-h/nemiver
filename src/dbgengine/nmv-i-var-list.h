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
#ifndef __NMV_I_VAR_LIST_H__
#define __NMV_I_VAR_LIST_H__

#include <list>
#include "nmv-i-debugger.h"
#include "common/nmv-safe-ptr-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::SafePtr;
using nemiver::common::DynModIface;
using nemiver::common::DynModIfaceSafePtr;

typedef std::list<IDebugger::VariableSafePtr> DebuggerVariableList;
class IVarList;
typedef SafePtr<IVarList, ObjectRef, ObjectUnref> IVarListSafePtr;
class NEMIVER_API IVarList : public DynModIface {
    IVarList ();
    IVarList (const IVarList &);

protected:
    IVarList (DynamicModule *a_dynmod) :
        DynModIface (a_dynmod)
    {
    }

public:

    /// \name signals
    ///@{
    virtual sigc::signal<void, const IDebugger::VariableSafePtr&>&
                                                variable_added_signal () = 0;
    virtual sigc::signal<void, const IDebugger::VariableSafePtr>&
                                                variable_value_set_signal ()=0;
    virtual sigc::signal<void, const IDebugger::VariableSafePtr&>&
                                                variable_type_set_signal () = 0;
    virtual sigc::signal<void, const IDebugger::VariableSafePtr&>&
                                                variable_removed_signal () = 0;
    ///@}

    /// \brief intialize the interface with a given IDebugger pointer.
    /// This must be the function called on the interface before using it.
    /// Failing to initialize the interface with a valid non null IDebugger
    /// make subsequent invocation of the interface methods throw instances
    /// of nemiver::common::Exception.
    /// \param a_debugger the non null debugger interface used to query
    //// the variables.
    virtual void initialize (IDebuggerSafePtr &a_debugger) = 0;

    virtual IDebugger& get_debugger () const = 0;

    /// \return the raw list of variables maintained internally.
    virtual const DebuggerVariableList& get_raw_list() const = 0;

    /// \brief append a variable to the list
    ///
    /// \param a_var the new variable to append
    /// \param a_update_type if true, update the type field of the
    ///  the variable. This will trigger an invocation of the IDebugger
    ///  interface.
    virtual void append_variable (const IDebugger::VariableSafePtr &a_var,
                                  bool a_update_type=true) = 0;

    /// \brief append a set list of variables to the list
    ///
    /// \param a_vars the new variable list to append
    /// \param a_update_type if true, update the type field of the
    ///  the variable. This will trigger an invocation of the IDebugger
    ///  interface.
    virtual void append_variables (const DebuggerVariableList& a_vars,
                                   bool a_update_type=true) = 0;

    /// \brief remove a variable from the list
    ///
    /// \param a_var the variable to remove from the list
    /// \return true if the variable has been found and removed
    virtual bool remove_variable (const IDebugger::VariableSafePtr &a_var) = 0;

    /// \brief remove a variable from the list
    ///
    /// \param a_var_name the name of the variable to remove
    /// \return true if the variable has been found and removed
    virtual bool remove_variable (const UString &a_var_name) = 0;

    /// \brief remove all variables from the list
    virtual void remove_variables () = 0;

    /// \brief lookup a variable from its name
    ///
    /// \param a_var_name the name of the variable to look for
    /// \param a_var where to put the variable, if found.
    /// \return true if the variable were found, false otherwise.
    virtual bool find_variable (const UString &a_var_name,
                                IDebugger::VariableSafePtr &a_var) = 0;

    /// \brief get the variable addressed by a qualified variable name.
    ///
    /// a qualified variable name has a form similar to: foo.bar.baz
    /// \param a_qname the qualified variable name
    /// \param a_var out parameters. The resulting variable, if found.
    /// \return true if the a variable matching that the qname a_qname
    /// has been found, false otherwise.
    virtual bool find_variable_from_qname (const UString &a_qname,
                                           IDebugger::VariableSafePtr &a_var)=0;

    /// \brief update the state of the all the variables of the list
    ///
    /// issue many calls to the underlying IDebugger interface to query
    /// the new values of variables contained in the list
    virtual void update_state () = 0;
};//end class IVarList

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_I_VAR_LIST_H__

