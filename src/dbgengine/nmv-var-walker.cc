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
#include "nmv-i-var-walker.h"
#include "nmv-gdb-engine.h"

using nemiver::common::DynamicModule ;
using nemiver::common::DynamicModuleSafePtr ;
using nemiver::common::DynModIface ;
using nemiver::common::DynModIfaceSafePtr ;
using nemiver::common::ObjectRef ;
using nemiver::common::ObjectUnref ;

typedef SafePtr<nemiver::GDBEngine, ObjectRef, ObjectUnref> GDBEngineSafePtr ;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VarWalker : public IVarWalker {
    sigc::signal<void,
                 const UString& /*cookie*/> m_error_signal ;
    sigc::signal<void,
                 VarFragment&> m_initialized_on_var_signal ;
    sigc::signal<void,
                 VarFragment&/*parent*/,
                 VarFragment&/*child*/> m_visited_var_child_signal;

    GDBEngineSafePtr m_debugger ;
    VarFragment m_var_fragment ;

public:

    VarWalker (DynamicModule *a_dynmod) :
        IVarWalker (a_dynmod)
    {
    }

    //********************
    //<event getters>
    //********************
    sigc::signal<void, const UString& /*cookie*/> error_signal () const;
    sigc::signal<void, VarFragment&> initialized_on_var_signal () const;
    sigc::signal<void, VarFragment&/*parent*/, VarFragment&/*child*/>
                                                visited_var_child_signal () const;
    //********************
    //</event getters>
    //********************

    void initialize (IDebuggerSafePtr &a_debugger,
                     const UString &a_var_name,
                     const UString &a_cookie="") ;

    void do_walk_var_children (const VarFragment &a_var,
                               bool a_recurse=false,
                               const UString &a_cookie="");
};//end class VarWalker

sigc::signal<void, const UString& /*cookie*/>
VarWalker::error_signal () const
{
    return m_error_signal ;
}

sigc::signal<void, VarFragment&>
VarWalker::initialized_on_var_signal () const
{
    return m_initialized_on_var_signal ;
}

sigc::signal<void, VarFragment&/*parent*/, VarFragment&/*child*/>
VarWalker::visited_var_child_signal () const
{
    return m_visited_var_child_signal ;
}

void
VarWalker::initialize (IDebuggerSafePtr &a_debugger,
                       const UString &a_var_name,
                       const UString &a_cookie)
{
    if (a_cookie == "") {}

    m_debugger = a_debugger.do_dynamic_cast<GDBEngine> ();
    THROW_IF_FAIL (m_debugger) ;
    m_var_fragment.set_name (a_var_name) ;
}

void
VarWalker::do_walk_var_children (const VarFragment &a_var,
                                 bool a_recurse,
                                 const UString &a_cookie)
{
    if (a_cookie == "" || a_var.get_name () == "" || a_recurse) {}
}


//the dynmod used to instanciate the VarWalker service object
//and return an interface on it.
struct VarWalkerDynMod : public  DynamicModule {

    void get_info (Info &a_info) const
    {
        const static Info s_info ("varWalker",
                                  "The Variable Walker dynmod. "
                                  "Implements the IVarWalker interface",
                                  "1.0") ;
        a_info = s_info ;
    }

    void do_init ()
    {
    }

    bool lookup_interface (const std::string &a_iface_name,
                           DynModIfaceSafePtr &a_iface)
    {
        if (a_iface_name == "IVarWalker") {
            a_iface.reset (new VarWalker (this)) ;
        } else {
            return false ;
        }
        return true ;
    }
};//end class varListDynMod

NEMIVER_END_NAMESPACE (nemiver)

//the dynmod initial factory.
extern "C" {

bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    *a_new_instance = new nemiver::VarWalkerDynMod () ;
    return (*a_new_instance != 0) ;
}

}
