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
#ifndef __NMV_I_VAR_WALKER_H__
#define __NMV_I_VAR_WALKER_H__

#include "nmv-i-debugger.h"
#include "common/nmv-safe-ptr-utils.h"
#include "nmv-var.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;
using nemiver::common::SafePtr;

class IVarWalker;
typedef SafePtr<IVarWalker,ObjectRef, ObjectUnref> IVarWalkerSafePtr;

/// Walker object. Queries recursively a variable and its members
/// for their type and contents.
/// When a member is queried for its content and type, the variable
/// is said have been "walked" or "visited".
/// The Walker then emits a "variable-walked" signal for each variable member
/// node that has been walked.
class NEMIVER_API IVarWalker : public DynModIface {
    IVarWalker ();
    IVarWalker (const IVarWalker &);

protected:
    IVarWalker (DynamicModule *a_dynmod) : DynModIface (a_dynmod)
    {
    }

public:
    /// \name signals
    ///@{

    /// emitted each time one of the members node of a variable is
    /// visited.
    virtual sigc::signal<void,
                         const IDebugger::VariableSafePtr>
                                    visited_variable_node_signal () const = 0;
    /// emitted when the root variable (the one the walker has been connected
    /// has been totally visited. That means when all its members nodes have
    /// been visited.
    virtual sigc::signal<void,
                         const IDebugger::VariableSafePtr>
                                        visited_variable_signal () const = 0;
    ///@}

    /// connect the walker to a variable and to a debugger
    /// that will be use to walk that variable
    virtual void connect (IDebugger *a_debugger,
                          const UString &a_var_name) = 0;

    virtual void connect (IDebugger *a_debugger,
                          const IDebugger::VariableSafePtr a_var) = 0;

    virtual void do_walk_variable (const UString &a_cookie = "") = 0;

    /// gets the root variable this walker is connected to.
    /// this will return a non null variable if and only if
    /// the visited_root_variabls_signal() has been emited already.
    virtual const IDebugger::VariableSafePtr get_variable () const = 0;

    /// gets the debugger the walker is connected to
    virtual IDebugger* get_debugger () const = 0;

    /// accessor of the maximum depth of variable members to explore.
    /// this can prevent inifite recursions.
    virtual void set_maximum_member_depth (unsigned a_max_depth) = 0;
    virtual unsigned get_maximum_member_depth () const = 0;
}; // end IVarWalker

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_I_VAR_WALKER_H__

