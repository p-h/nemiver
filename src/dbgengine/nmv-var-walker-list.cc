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
#include "nmv-i-var-walker-list.h"

using namespace nemiver::common ;

NEMIVER_BEGIN_NAMESPACE (nemiver)

class VarWalkerList : public IVarWalkerList {
     mutable sigc::signal<void, const IVarWalkerSafePtr&> m_variable_visited_signal;
     list<IDebugger::VariableSafePtr> m_variables ;
     list<IVarWalkerSafePtr> m_var_walkers ;
     IDebuggerSafePtr m_debugger ;

     IVarWalkerSafePtr create_variable_walker (const IDebugger::VariableSafePtr &a_var) ;

     void on_visited_variable_signal (const IDebugger::VariableSafePtr &a_var,
                                      IVarWalkerSafePtr &a_walker) ;

public:

     VarWalkerList (DynamicModule *a_dynmod) :
         IVarWalkerList (a_dynmod)
    {
    }
    //******************
    //<event getters>
    //******************
    sigc::signal<void, const IVarWalkerSafePtr&>& variable_visited_signal () const ;
    //******************
    //</event getters>
    //******************
    void initialize (IDebuggerSafePtr &a_debugger) ;

    void append_variable (const IDebugger::VariableSafePtr a_var) ;
    void append_variables (const list<IDebugger::VariableSafePtr> a_vars) ;

    void remove_variables ();

    void do_walk_variables () ;

};//end class VarWalkerList

void
VarWalkerList::on_visited_variable_signal
                                (const IDebugger::VariableSafePtr &a_var,
                                 IVarWalkerSafePtr &a_walker)
{
    if (a_var) {}
    NEMIVER_TRY
    variable_visited_signal ().emit (a_walker) ;
    NEMIVER_CATCH_NOX
}

IVarWalkerSafePtr
VarWalkerList::create_variable_walker (const IDebugger::VariableSafePtr &a_var)
{
    IVarWalkerSafePtr result;
    if (!a_var) {
        return result ;
    }
    DynamicModule::Loader *loader =
        get_dynamic_module ().get_module_loader ();
    THROW_IF_FAIL (loader) ;
    DynamicModuleManager *module_manager = loader->get_dynamic_module_manager ();
    THROW_IF_FAIL (module_manager) ;
    result = module_manager->load_iface<IVarWalker> ("varwalker",
                                                     "IVarWalker");
    THROW_IF_FAIL (result) ;
    result->connect (m_debugger, a_var) ;
    return result ;
}

sigc::signal<void, const IVarWalkerSafePtr&>&
VarWalkerList::variable_visited_signal () const
{
    return m_variable_visited_signal ;
}

void
VarWalkerList::initialize (IDebuggerSafePtr &a_debugger)
{
    THROW_IF_FAIL (a_debugger) ;

    m_debugger = a_debugger ;
}

void
VarWalkerList::append_variable (const IDebugger::VariableSafePtr a_var)
{
    THROW_IF_FAIL (a_var) ;
    m_variables.push_back (a_var) ;
    IVarWalkerSafePtr var_walker = create_variable_walker (a_var) ;
    THROW_IF_FAIL (var_walker) ;
    var_walker->visited_variable_signal ().connect
        (sigc::bind
         (sigc::mem_fun (*this, &VarWalkerList::on_visited_variable_signal),
          var_walker)) ;
    m_var_walkers.push_back (var_walker) ;
}

void
VarWalkerList::append_variables (const list<IDebugger::VariableSafePtr> a_vars)
{
    list<IDebugger::VariableSafePtr>::const_iterator it;
    for (it = a_vars.begin (); it != a_vars.end (); ++it) {
        append_variable (*it) ;
    }
}

void
VarWalkerList::remove_variables ()
{
    m_variables.clear () ;
    m_var_walkers.clear () ;
}

void
VarWalkerList::do_walk_variables ()
{
    list<IVarWalkerSafePtr>::iterator it;
    for (it = m_var_walkers.begin (); it != m_var_walkers.end (); ++it) {
        (*it)->do_walk_variable () ;
    }
}

//the dynmod used to instanciate the VarWalkerList service object
//and return an interface on it.
struct VarWalkerListDynMod : public DynamicModule {
    void get_info (Info &a_info) const
    {
        const static Info s_info ("VarWalkerList",
                                  "The list of variable walkers dynmod. "
                                  "Implements the IVarWalkerList interface",
                                  "1.0") ;
        a_info = s_info ;
    }

    void do_init ()
    {
    }

    bool lookup_interface (const std::string &a_iface_name,
                           DynModIfaceSafePtr &a_iface)
    {
        if (a_iface_name == "IVarWalkerlist") {
            a_iface.reset (new VarWalkerList (this)) ;
        } else {
            return false;
        }
        return true ;
    }
};//end class VarWalkerListDynMod

NEMIVER_END_NAMESPACE (nemiver)
//the dynmod initial factory.
extern "C" {

bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    *a_new_instance = new nemiver::VarWalkerListDynMod () ;
    return (*a_new_instance != 0) ;
}

}

