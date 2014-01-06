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

const UString VAR_WALKER_COOKIE="var-walker-cookie";

nemiver::common::Sequence&
get_sequence ()
{
    static nemiver::common::Sequence sequence;
    return sequence;
}

struct SafePtrCmp {
    bool operator() (const IDebugger::VariableSafePtr l,
                     const IDebugger::VariableSafePtr r) const
    {
        return (l.get () < r.get ());
    }
};

class VarWalker : public IVarWalker , public sigc::trackable {

    mutable sigc::signal<void,
                 const IDebugger::VariableSafePtr> m_visited_variable_node_signal;

    mutable sigc::signal<void,
                         const IDebugger::VariableSafePtr>
                                            m_visited_variable_signal;

    mutable GDBEngine *m_debugger;
    UString m_root_var_name;
    list<sigc::connection> m_connections;
    map<IDebugger::VariableSafePtr, bool, SafePtrCmp> m_vars_to_visit;
    UString m_cookie;
    IDebugger::VariableSafePtr m_root_var;

    void on_variable_value_signal (const UString &a_name,
                                   const IDebugger::VariableSafePtr a_var,
                                   const UString &a_cookie);

    void on_variable_value_set_signal (const IDebugger::VariableSafePtr a_var,
                                       const UString &a_cookie);

    void on_variable_type_set_signal (const IDebugger::VariableSafePtr a_var,
                                      const UString &a_cookie);

    void get_type_of_all_members (const IDebugger::VariableSafePtr a_from);

public:

    VarWalker (DynamicModule *a_dynmod) :
        IVarWalker (a_dynmod),
        m_debugger (0)
    {
    }

    //********************
    //<event getters>
    //********************
    sigc::signal<void, const IDebugger::VariableSafePtr>
                                        visited_variable_node_signal () const;
    sigc::signal<void, const IDebugger::VariableSafePtr>
                                        visited_variable_signal () const;
    //********************
    //</event getters>
    //********************

    void connect (IDebugger *a_debugger, const UString &a_var_name);

    void connect (IDebugger *a_debugger,
                  const IDebugger::VariableSafePtr a_var);

    void do_walk_variable (const UString &a_cookie="");

    const IDebugger::VariableSafePtr get_variable () const;

    IDebugger *get_debugger () const;

    void set_maximum_member_depth (unsigned a_max_depth);

    unsigned get_maximum_member_depth () const;
};//end class VarWalker

void
VarWalker::on_variable_value_signal (const UString &a_name,
                                     const IDebugger::VariableSafePtr a_var,
                                     const UString &a_cookie)
{
    if (a_name.raw () == "") {}
    if (a_cookie.raw () != m_cookie.raw ()) {
        return;
    }

    NEMIVER_TRY

    //now query for the type
    get_type_of_all_members (a_var);
    m_root_var = a_var;
    LOG_DD ("set m_root_var");

    NEMIVER_CATCH_NOX
}

void
VarWalker::on_variable_value_set_signal (const IDebugger::VariableSafePtr a_var,
                                         const UString &a_cookie)
{
    if (a_cookie.raw () != m_cookie.raw ()) {
        return;
    }

    NEMIVER_TRY

    //now query for the type
    get_type_of_all_members (a_var);

    LOG_DD ("m_vars_to_visit.size () = " << (int)m_vars_to_visit.size ());
    UString var_str;
    NEMIVER_CATCH_NOX
}

void
VarWalker::on_variable_type_set_signal (const IDebugger::VariableSafePtr a_var,
                                        const UString &a_cookie)
{
    if (a_cookie.raw () != m_cookie.raw ()) {
        return;
    }

    NEMIVER_TRY

    THROW_IF_FAIL (a_var);
    THROW_IF_FAIL (!m_vars_to_visit.empty ());

    UString parent_name;
    if (a_var->parent ()) {
        parent_name = a_var->parent ()->name ();
    } else {
        parent_name = "null";
    }
    LOG_DD ("var: " << a_var->name ()
            << " type: " << a_var->type ()
            << " parent: " << parent_name);

    visited_variable_node_signal ().emit (a_var);
    m_vars_to_visit.erase (a_var);
    if (m_vars_to_visit.size () == 0) {
        visited_variable_signal ().emit (m_root_var);
        LOG_DD ("visited var: " << m_root_var->name ()
                << ", cur node: " << a_var->name ());
    } else {
        LOG_DD ("m_vars_to_visit.size () = " << (int) m_vars_to_visit.size ());
    }
    map<IDebugger::VariableSafePtr, bool>::iterator it;
    for (it = m_vars_to_visit.begin ();
         it != m_vars_to_visit.end ();
         ++it) {
        LOG_DD ("m_vars_to_visit[x] = " << it->first->name ());
    }

    NEMIVER_CATCH_NOX
}

void
VarWalker::get_type_of_all_members (const IDebugger::VariableSafePtr a_from)
{
    RETURN_IF_FAIL (a_from);

    LOG_DD ("member: " << a_from->name ());
    if (a_from->parent ()) {
        LOG_DD ("parent: " << a_from->parent ()->name ());
    } else {
        LOG_DD ("parent: null");
    }

    UString qname;
    a_from->build_qname (qname);
    LOG_DD ("qname: " << qname);
    qname.chomp ();
    if (qname.raw ()[0] == '<' || a_from->name ().raw ()[0] == '<') {
        //this is a hack to detect c++ templated unamed members
        //usually, their name have the form "<blablah>"
        LOG_DD ("templated unnamed member, don't query for its type");
        LOG_DD ("member was: " << a_from->name ());
    } else if (qname.get_number_of_words () != 1) {
        LOG_DD ("variable name is weird, don't query for its type");
        LOG_DD ("member was: " << a_from->name ());
        return;
    } else {
        m_vars_to_visit[a_from] = true;
        m_debugger->get_variable_type (a_from, m_cookie);
        LOG_DD ("member : " << a_from->name () << "to added to m_vars_to_visit");
        return;
    }
    list<IDebugger::VariableSafePtr>::const_iterator it;
    for (it = a_from->members ().begin ();
         it != a_from->members ().end ();
         ++it) {
        get_type_of_all_members (*it);
    }
    LOG_DD ("m_vars_to_visit.size () = " << (int)m_vars_to_visit.size ());
}

sigc::signal<void, const IDebugger::VariableSafePtr>
VarWalker::visited_variable_node_signal () const
{
    return m_visited_variable_node_signal;
}

sigc::signal<void, const IDebugger::VariableSafePtr>
VarWalker::visited_variable_signal () const
{
    return m_visited_variable_signal;
}

void
VarWalker::connect (IDebugger *a_debugger,
                    const UString &a_var_name)
{
    THROW_IF_FAIL (a_debugger);

    m_debugger = dynamic_cast<GDBEngine*> (a_debugger);
    THROW_IF_FAIL (m_debugger);

    m_root_var_name = a_var_name;

    list<sigc::connection>::iterator it;
    for (it = m_connections.begin (); it != m_connections.end (); ++it) {
        it->disconnect ();
    }
    m_connections.push_back (m_debugger->variable_value_signal ().connect
            (sigc::mem_fun (*this, &VarWalker::on_variable_value_signal)));
    m_connections.push_back (m_debugger->variable_type_set_signal ().connect
                    (sigc::mem_fun (*this, &VarWalker::on_variable_type_set_signal)));
}

void
VarWalker::connect (IDebugger *a_debugger,
                    const IDebugger::VariableSafePtr a_var)
{
    THROW_IF_FAIL (a_debugger);

    m_debugger = dynamic_cast<GDBEngine *> (a_debugger);
    THROW_IF_FAIL (m_debugger);

    m_root_var = a_var;

    list<sigc::connection>::iterator it;
    for (it = m_connections.begin (); it != m_connections.end (); ++it) {
        it->disconnect ();
    }
    m_connections.clear ();
    m_connections.push_back (m_debugger->variable_value_set_signal ().connect
            (sigc::mem_fun (*this,
                            &VarWalker::on_variable_value_set_signal)));
    m_connections.push_back (m_debugger->variable_type_set_signal ().connect
                (sigc::mem_fun (*this,
                                &VarWalker::on_variable_type_set_signal)));
}

void
VarWalker::do_walk_variable (const UString &a_cookie)
{
    if (a_cookie.raw () == "") {
        m_cookie =
            UString::from_int (get_sequence ().create_next_integer ()) +
            "-" + VAR_WALKER_COOKIE;
    } else {
        m_cookie = a_cookie;
    }

    if (m_root_var_name.raw () != "") {
        m_debugger->print_variable_value (m_root_var_name,
                                          m_cookie);
    } else if (m_root_var){
        m_debugger->get_variable_value (m_root_var, m_cookie);
    }
}

const IDebugger::VariableSafePtr
VarWalker::get_variable () const
{
    return m_root_var;
}

IDebugger*
VarWalker::get_debugger () const
{
    return dynamic_cast<IDebugger*> (m_debugger);
}

void
VarWalker::set_maximum_member_depth (unsigned)
{
}

unsigned
VarWalker::get_maximum_member_depth () const
{
    return 0;
}

//the dynmod used to instanciate the VarWalker service object
//and return an interface on it.
struct VarWalkerDynMod : public  DynamicModule {

    void get_info (Info &a_info) const
    {
        const static Info s_info ("varWalker",
                                  "The Variable Walker dynmod. "
                                  "Implements the IVarWalker interface",
                                  "1.0");
        a_info = s_info;
    }

    void do_init ()
    {
    }

    bool lookup_interface (const std::string &a_iface_name,
                           DynModIfaceSafePtr &a_iface)
    {
        if (a_iface_name == "IVarWalker") {
            a_iface.reset (new VarWalker (this));
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
    *a_new_instance = new nemiver::VarWalkerDynMod ();
    return (*a_new_instance != 0);
}

}
