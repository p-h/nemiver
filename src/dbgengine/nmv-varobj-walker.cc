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
#include <list>
#include <map>
#include "nmv-i-var-walker.h"
#include "nmv-gdb-engine.h"
#include "common/nmv-sequence.h"
#include "nmv-i-lang-trait.h"

using std::list;
using std::map;
using nemiver::common::DynamicModule;
using nemiver::common::DynamicModuleSafePtr;
using nemiver::common::DynModIface;
using nemiver::common::DynModIfaceSafePtr;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;

typedef SafePtr<nemiver::GDBEngine, ObjectRef, ObjectUnref> GDBEngineSafePtr;

NEMIVER_BEGIN_NAMESPACE (nemiver)

static const unsigned MAX_DEPTH = 256;

class VarobjWalker : public IVarWalker, public sigc::trackable
{
    mutable sigc::signal<void,
                         const IDebugger::VariableSafePtr>
                                         m_visited_variable_node_signal;
    mutable sigc::signal<void,
                         const IDebugger::VariableSafePtr>
                                        m_visited_variable_signal;
    IDebugger *m_debugger;
    IDebugger::VariableSafePtr m_variable;
    UString m_var_name;
    bool m_do_walk;
    // The count of on going variable unfolding
    int m_variable_unfolds;

    unsigned m_max_depth;

    VarobjWalker (); // Don't call this constructor.

public:

    VarobjWalker (DynamicModule *a_dynmod) :
        IVarWalker (a_dynmod),
        m_debugger (0),
        m_do_walk (false),
        m_variable_unfolds (0),
        m_max_depth (MAX_DEPTH)
    {
    }

    sigc::signal<void,
                 const IDebugger::VariableSafePtr>
                                    visited_variable_node_signal () const;
    sigc::signal<void,
                 const IDebugger::VariableSafePtr>
                                    visited_variable_signal () const;

    void connect (IDebugger *a_debugger,
                  const UString &a_var_name);

    void connect (IDebugger *a_debugger,
                  const IDebugger::VariableSafePtr a_var);

    void do_walk_variable (const UString &a_cookie="");

    const IDebugger::VariableSafePtr get_variable () const;

    IDebugger* get_debugger () const;

    void set_maximum_member_depth (unsigned a_max_depth);

    unsigned get_maximum_member_depth () const;

    void do_walk_variable_real (const IDebugger::VariableSafePtr,
                                unsigned a_max_depth);

    void on_variable_unfolded_signal (const IDebugger::VariableSafePtr a_var,
                                      unsigned max_depth);

    void on_variable_created_signal (const IDebugger::VariableSafePtr a_var);
}; // end class VarobjWalker.

sigc::signal<void,
             const IDebugger::VariableSafePtr>
VarobjWalker::visited_variable_node_signal () const
{
    return m_visited_variable_node_signal;
}

sigc::signal<void,
             const IDebugger::VariableSafePtr>
VarobjWalker::visited_variable_signal () const
{
    return m_visited_variable_signal;
}

void
VarobjWalker::connect (IDebugger *a_debugger,
                       const UString &a_var_name)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    THROW_IF_FAIL (a_debugger);
    THROW_IF_FAIL (!a_var_name.empty ());

    m_debugger = a_debugger;
    m_var_name = a_var_name;
    m_debugger->create_variable
        (a_var_name,
         sigc::mem_fun (*this, &VarobjWalker::on_variable_created_signal));

}

void
VarobjWalker::connect (IDebugger *a_debugger,
                       const IDebugger::VariableSafePtr a_var)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    THROW_IF_FAIL (a_debugger);
    THROW_IF_FAIL (a_var);
    // The variable must be backed by variable objects.
    THROW_IF_FAIL (!a_var->internal_name ().empty ());

    m_debugger = a_debugger;
    m_variable = a_var;
}

void
VarobjWalker::do_walk_variable (const UString &)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    if (!m_variable) {
        if (!m_var_name.empty ()) {
            LOG_DD ("setting m_do_walk to true");
            m_do_walk = true;
            return;
        } else {
            THROW ("expecting a non null m_variable!");
        }
    }
    do_walk_variable_real (m_variable, m_max_depth);

}

const IDebugger::VariableSafePtr
VarobjWalker::get_variable () const
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    return m_variable;
}

IDebugger*
VarobjWalker::get_debugger () const
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    return m_debugger;
}

void
VarobjWalker::set_maximum_member_depth (unsigned a_max_depth)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    m_max_depth = a_max_depth;
}

unsigned
VarobjWalker::get_maximum_member_depth () const
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    return m_max_depth;
}

void
VarobjWalker::do_walk_variable_real (const IDebugger::VariableSafePtr a_var,
                                     unsigned a_max_depth)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    THROW_IF_FAIL (a_var);

    LOG_DD ("internal var name: " << a_var->internal_name ()
            << "depth: " << (int) a_max_depth);

    if (a_max_depth == 0)
        return;

    if (a_var->needs_unfolding ()
        && m_debugger->get_language_trait ().is_variable_compound (a_var)) {
        LOG_DD ("needs unfolding");
        m_variable_unfolds++;
        m_debugger->unfold_variable
            (a_var,
             sigc::bind (sigc::mem_fun (*this,
                                   &VarobjWalker::on_variable_unfolded_signal),
                         a_max_depth - 1));
    } else if (!a_var->members ().empty ()) {
        LOG_DD ("children need visiting");
        visited_variable_node_signal ().emit (a_var);
        IDebugger::VariableList::const_iterator it;
        for (it = a_var->members ().begin ();
             it != a_var->members ().end ();
             ++ it) {
            do_walk_variable_real (*it, a_max_depth - 1);
        }
    } else {
        LOG_DD ("else finished ?. m_variable_unfolds: "
                << (int) m_variable_unfolds);
        visited_variable_node_signal ().emit (a_var);
        if (m_variable_unfolds == 0)
            visited_variable_signal ().emit (m_variable);
    }

}

void
VarobjWalker::on_variable_unfolded_signal (const IDebugger::VariableSafePtr a_var,
                                           unsigned a_max_depth)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    NEMIVER_TRY

    m_variable_unfolds--;
    visited_variable_node_signal ().emit (a_var);
    do_walk_variable_real (a_var, a_max_depth);
    if (m_variable_unfolds == 0) {
        THROW_IF_FAIL (m_variable);
        visited_variable_signal ().emit (m_variable);
    }

    NEMIVER_CATCH_NOX
}

void
VarobjWalker::on_variable_created_signal (const IDebugger::VariableSafePtr a_var)
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;

    NEMIVER_TRY

    THROW_IF_FAIL (a_var);

    m_variable = a_var;

    if (m_do_walk) {
        do_walk_variable ();
        m_do_walk = false;
    } else {
        LOG_DD ("m_do_walk is false");
    }

    NEMIVER_CATCH_NOX
}

// the dynmod used to instanciate the VarWalker service object
// and return an interface on it.
struct VarobjWalkerDynMod : public  DynamicModule
{

    void
    get_info (Info &a_info) const
    {
        const static Info s_info ("VarobjWalker",
                                  "The Variable Object Walker dynmod. "
                                  "Implements the IVarWalker interface",
                                  "1.0");
        a_info = s_info;
    }

    void
    do_init ()
    {
    }

    bool
    lookup_interface (const std::string &a_iface_name,
                      DynModIfaceSafePtr &a_iface)
    {
        if (a_iface_name == "IVarWalker") {
            a_iface.reset (new VarobjWalker (this));
        } else {
            return false;
        }
        return true;
    }
}; //end class VarobjListDynMod

//the dynmod initial factory.
extern "C"
{

bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    *a_new_instance = new nemiver::VarobjWalkerDynMod ();
    return (*a_new_instance != 0);
}

}

NEMIVER_END_NAMESPACE(nemiver)


