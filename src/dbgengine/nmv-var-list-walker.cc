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
#include <map>
#include "nmv-i-var-list-walker.h"

using namespace nemiver::common;

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct SafePtrCmp {
    bool operator() (const IVarWalkerSafePtr l,
                     const IVarWalkerSafePtr r) const
    {
        return (l.get () < r.get ());
    }
};
class VarListWalker : public IVarListWalker {
     mutable sigc::signal<void, const IVarWalkerSafePtr> m_variable_visited_signal;
     mutable sigc::signal<void> m_variable_list_visited_signal;
     list<IDebugger::VariableSafePtr> m_variables;
     list<IVarWalkerSafePtr> m_var_walkers;
     typedef std::map<IVarWalkerSafePtr, bool, SafePtrCmp>  WalkersMap;
     typedef std::deque<WalkersMap> WalkersQueue;
     WalkersQueue m_considered_walkers;
     WalkersMap m_walkers_map;
     IDebugger *m_debugger;

     IVarWalkerSafePtr create_variable_walker
                                 (const IDebugger::VariableSafePtr &a_var);

     void on_visited_variable_signal (const IDebugger::VariableSafePtr a_var,
                                      IVarWalkerSafePtr a_walker);

public:

     VarListWalker (DynamicModule *a_dynmod) :
         IVarListWalker (a_dynmod),
         m_debugger (0)
    {
    }
    //******************
    //<event getters>
    //******************
    sigc::signal<void, const IVarWalkerSafePtr>&
                                    variable_visited_signal () const;
    sigc::signal<void>& variable_list_visited_signal () const;
    //******************
    //</event getters>
    //******************
    void initialize (IDebugger *a_debugger);

    void append_variable (const IDebugger::VariableSafePtr a_var);

    void append_variables (const list<IDebugger::VariableSafePtr> a_vars);

    void remove_variables ();

    bool do_walk_variable (const UString &a_var_qname);

    void do_walk_variables ();

};//end class VarListWalker

void
VarListWalker::on_visited_variable_signal
                                (const IDebugger::VariableSafePtr a_var,
                                 IVarWalkerSafePtr a_walker)
{
    if (a_var) {}
    NEMIVER_TRY
    variable_visited_signal ().emit (a_walker);
    THROW_IF_FAIL (m_walkers_map.find (a_walker) != m_walkers_map.end ());
    m_walkers_map.erase (a_walker);
    if (m_walkers_map.empty ()) {
        variable_list_visited_signal ().emit ();
    }
    NEMIVER_CATCH_NOX
}

IVarWalkerSafePtr
VarListWalker::create_variable_walker (const IDebugger::VariableSafePtr &a_var)
{
    IVarWalkerSafePtr result;
    if (!a_var) {
        return result;
    }
    DynamicModule::Loader *loader =
        get_dynamic_module ().get_module_loader ();
    THROW_IF_FAIL (loader);
    DynamicModuleManager *module_manager = loader->get_dynamic_module_manager ();
    THROW_IF_FAIL (module_manager);
    result = module_manager->load_iface<IVarWalker> ("varwalker",
                                                     "IVarWalker");
    THROW_IF_FAIL (result);
    result->connect (m_debugger, a_var);
    return result;
}

sigc::signal<void, const IVarWalkerSafePtr>&
VarListWalker::variable_visited_signal () const
{
    return m_variable_visited_signal;
}

sigc::signal<void>&
VarListWalker::variable_list_visited_signal () const
{
    return m_variable_list_visited_signal;
}

void
VarListWalker::initialize (IDebugger *a_debugger)
{
    THROW_IF_FAIL (a_debugger);

    m_debugger = a_debugger;
}

void
VarListWalker::append_variable (const IDebugger::VariableSafePtr a_var)
{
    THROW_IF_FAIL (a_var);
    m_variables.push_back (a_var);
    IVarWalkerSafePtr var_walker = create_variable_walker (a_var);
    THROW_IF_FAIL (var_walker);
    var_walker->visited_variable_signal ().connect
        (sigc::bind
         (sigc::mem_fun (*this, &VarListWalker::on_visited_variable_signal),
          var_walker));
    m_var_walkers.push_back (var_walker);
    UString var_str;
    a_var->to_string (var_str, true);
    LOG_DD ("appended variable: " << var_str);
}

void
VarListWalker::append_variables
                    (const list<IDebugger::VariableSafePtr> a_vars)
{

    list<IDebugger::VariableSafePtr>::const_iterator it;
    for (it = a_vars.begin (); it != a_vars.end (); ++it) {
        append_variable (*it);
    }
}

void
VarListWalker::remove_variables ()
{
    m_variables.clear ();
    m_var_walkers.clear ();
}

bool
VarListWalker::do_walk_variable (const UString &a_var_qname)
{
    UString qname;
    list<IVarWalkerSafePtr>::iterator it;
    for (it = m_var_walkers.begin (); it != m_var_walkers.end (); ++it) {
        if (!(*it) || !(*it)->get_variable ())
            continue;
        (*it)->get_variable ()->build_qname (qname);
        if (qname == a_var_qname) {
            LOG_DD ("found variable of qname " << qname << " to walk");
            m_walkers_map[*it] = true;
            (*it)->do_walk_variable ();
            LOG_DD ("variable walking query sent");
            return true;
        }
    }
    LOG_ERROR ("did not find variable " << a_var_qname << " to walk");
    return false;
}

void
VarListWalker::do_walk_variables ()
{
    list<IVarWalkerSafePtr>::iterator it;
    for (it = m_var_walkers.begin (); it != m_var_walkers.end (); ++it) {
        m_walkers_map[*it] = true;
        (*it)->do_walk_variable ();
    }
}

//the dynmod used to instanciate the VarListWalker service object
//and return an interface on it.
struct VarListWalkerDynMod : public DynamicModule {
    void get_info (Info &a_info) const
    {
        const static Info s_info ("VarListWalker",
                                  "The list of variable walkers dynmod. "
                                  "Implements the IVarListWalker interface",
                                  "1.0");
        a_info = s_info;
    }

    void do_init ()
    {
    }

    bool lookup_interface (const std::string &a_iface_name,
                           DynModIfaceSafePtr &a_iface)
    {
        if (a_iface_name == "IVarListWalker") {
            a_iface.reset (new VarListWalker (this));
        } else {
            return false;
        }
        return true;
    }
};//end class VarListWalkerDynMod

NEMIVER_END_NAMESPACE (nemiver)
//the dynmod initial factory.
extern "C" {

bool
NEMIVER_API nemiver_common_create_dynamic_module_instance
                                                    (void **a_new_instance)
{
    *a_new_instance = new nemiver::VarListWalkerDynMod ();
    return (*a_new_instance != 0);
}

}

