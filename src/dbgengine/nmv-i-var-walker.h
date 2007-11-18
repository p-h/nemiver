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

using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;
using nemiver::common::SafePtr ;

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
    virtual sigc::signal<void, const UString& /*cookie*/> error_signal () const = 0;
    virtual sigc::signal<void, VarFragment&> initialized_on_var_signal () const = 0;
    virtual sigc::signal<void, VarFragment&/*parent*/, VarFragment&/*child*/>
                                                visited_var_child_signal () const = 0;
    ///@}
    virtual void initialize (IDebuggerSafePtr &a_debugger,
                             const UString &a_var_name,
                             const UString &a_cookie="") = 0;
    virtual void do_walk_var_children (const VarFragment &a_var,
                                       bool a_recurse=false,
                                       const UString &a_cookie="") = 0;
};//end IVarWalker

NEMIVER_END_NAMESPACE (nemiver)

#endif //__NMV_I_VAR_WALKER_H__

