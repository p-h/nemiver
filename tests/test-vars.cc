#include <iostream>
#include <map>
#include <string>
#include <boost/test/minimal.hpp>
#include <glibmm.h>
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "nmv-i-var-list-walker.h"

using namespace nemiver ;
using namespace nemiver::common ;

Glib::RefPtr<Glib::MainLoop> s_loop =
    Glib::MainLoop::create (Glib::MainContext::get_default ()) ;
static int nb_vars_created;
static int nb_vars_created2;
static int nb_vars_deleted;
static int nb_vars_deleted2;
static int unfold_requests;
static IDebugger::VariableSafePtr var_to_delete;

void on_variable_deleted_signal (const IDebugger::VariableSafePtr a_var);
void on_variable_unfolded_signal (const IDebugger::VariableSafePtr a_var,
                                  IDebuggerSafePtr a_debugger);

void
on_command_done_signal (const UString&, const UString&)
{
}

void
on_engine_died_signal ()
{
    s_loop->quit ();
}

void
on_program_finished_signal ()
{
    s_loop->quit ();
}

void
on_running_signal ()
{
}

void
on_variable_created_signal (IDebugger::VariableSafePtr a_var)
{
    MESSAGE ("variable created: " << a_var->name ());
    nb_vars_created++;
    if (a_var->name () == "person") {
        // put the variable aside so that we
        // can delete it later
        var_to_delete = a_var;
    }
}

void
on_variable_created_signal2 (IDebugger::VariableSafePtr a_var,
                             const UString &,
                             IDebuggerSafePtr a_debugger)
{
    UString var_str;
    a_var->to_string (var_str, true /*show var name*/);
    MESSAGE ("variable created: " << var_str);
    nb_vars_created2++;

    unfold_requests++;
    a_debugger->unfold_variable (a_var,
                                 sigc::bind (&on_variable_unfolded_signal,
                                             a_debugger));
}

void
on_variable_unfolded_signal (const IDebugger::VariableSafePtr a_var,
                             IDebuggerSafePtr a_debugger)
{
    unfold_requests--;
    UString var_str;
    a_var->to_string (var_str, true /*show var name*/);
    MESSAGE ("variable unfolded: " << var_str);
    typedef list<IDebugger::VariableSafePtr>::const_iterator VarIter;
    for (VarIter it = a_var->members ().begin ();
         it != a_var->members ().end ();
         ++it) {
        if ((*it)->has_expected_children ()) {
            unfold_requests++;
            a_debugger->unfold_variable (*it,
                                         sigc::bind (&on_variable_unfolded_signal,
                                                     a_debugger));
        }
    }
    if (unfold_requests == 0 && var_to_delete)
        a_debugger->delete_variable (var_to_delete,
                                     &on_variable_deleted_signal);
}

void
on_variable_deleted_signal (const IDebugger::VariableSafePtr a_var)
{
    MESSAGE ("variable deleted. Name: "
             << a_var->name ()
             << ", Internal Name: "
             << a_var->internal_name ());
    nb_vars_deleted++;
}

void
on_variable_deleted_signal2 (const IDebugger::VariableSafePtr a_var,
                             const UString&)
{
    MESSAGE ("variable deleted. Name: "
             << a_var->name ()
             << ", Internal Name: "
             << a_var->internal_name ());
    nb_vars_deleted2++;
}

void
on_stopped_signal (IDebugger::StopReason /*a_reason*/,
                   bool,
                   const IDebugger::Frame&,
                   int,
                   int,
                   const UString&,
                   IDebuggerSafePtr &debugger)
{
    static int nb_stops = 0;

    nb_stops++;
    MESSAGE ("stopped " << nb_stops << " times");

    if (nb_stops == 2) {
        // Okay we stepped once, now we can now create the variable
        // for the person variable.
        debugger->create_variable ("person",
                                   &on_variable_created_signal);
        MESSAGE ("Requested creation of variable 'person'");
    }
    debugger->step_over ();
}

NEMIVER_API int
test_main (int argc __attribute__((unused)),
           char **argv __attribute__ ((unused)))
{
    NEMIVER_TRY

    Initializer::do_init ();

    //load the IDebugger interface
    IDebuggerSafePtr debugger =
        DynamicModuleManager::load_iface_with_default_manager<IDebugger>
                                                                ("gdbengine",
                                                                 "IDebugger");
    //setup the debugger with the glib mainloop
    debugger->set_event_loop_context (Glib::MainContext::get_default ());

    //*******************************
    //<connect to IDebugger events>
    //******************************
    debugger->command_done_signal ().connect (&on_command_done_signal);
    debugger->engine_died_signal ().connect (&on_engine_died_signal);
    debugger->program_finished_signal ().connect (&on_program_finished_signal);
    debugger->running_signal ().connect (&on_running_signal);
    debugger->stopped_signal ().connect (sigc::bind (&on_stopped_signal,
                                                      debugger));
    debugger->variable_created_signal ().connect (sigc::bind
                                              (&on_variable_created_signal2,
                                               debugger));
    debugger->variable_deleted_signal ().connect (&on_variable_deleted_signal2);
    //*******************************
    //</connect to IDebugger events>
    //******************************
    debugger->load_program ("fooprog", ".");
    debugger->set_breakpoint ("main");
    debugger->run ();

    //****************************************
    //run the event loop.
    //****************************************
    s_loop->run ();

    NEMIVER_CATCH_AND_RETURN_NOX (-1)

    BOOST_REQUIRE (nb_vars_created == nb_vars_created2);
    BOOST_REQUIRE (nb_vars_created > 0);
    BOOST_REQUIRE (nb_vars_deleted == nb_vars_deleted2);
    BOOST_REQUIRE (nb_vars_deleted > 0);

    return 0;
}

