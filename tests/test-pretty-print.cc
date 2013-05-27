#include "config.h"
#include <iostream>
#include <map>
#include <string>
#include <boost/test/minimal.hpp>
#include <glibmm.h>
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "nmv-debugger-utils.h"

using namespace nemiver;
using namespace nemiver::common;

Glib::RefPtr<Glib::MainLoop> s_loop =
    Glib::MainLoop::create (Glib::MainContext::get_default ());

IDebugger::VariableSafePtr v_var;

static void
on_engine_died_signal ()
{
    s_loop->quit ();
}

static void
on_program_finished_signal ()
{
    s_loop->quit ();
}

static void
on_running_signal ()
{
}

static void
on_variable_unfolded_with_n_children (const IDebugger::VariableSafePtr a_var,
                                      unsigned n,
                                      IDebuggerSafePtr a_debugger)
{
    NEMIVER_TRY;

    BOOST_REQUIRE (a_var->name () == "v");
    BOOST_REQUIRE (a_var->members ().size () == n);
    a_debugger->step_over ();

    NEMIVER_CATCH_NOX;
}

static void
on_variable_v_created_empty (const IDebugger::VariableSafePtr a_var,
                             IDebuggerSafePtr a_debugger)
{
    NEMIVER_TRY;

    BOOST_REQUIRE (a_var->name () == "v");
    BOOST_REQUIRE (a_var->is_dynamic ());
    BOOST_REQUIRE (a_var->members ().empty ());
    v_var = a_var;
    a_debugger->step_over ();

    NEMIVER_CATCH_NOX;
}

static void
on_variable_v_has_n_children (const std::list<IDebugger::VariableSafePtr> &a_vars,
                              unsigned n, IDebuggerSafePtr a_debugger)
{
    NEMIVER_TRY;
    
    MESSAGE ("Number of children v is supposed to have: " << (int) n);
    MESSAGE ("a_vars.size(): " << (int) a_vars.size ());

    if (a_vars.size () == 1) {
        // It means that a_vars only contains "v" itself, and that it
        // needs unfolding.
        IDebugger::VariableSafePtr var = a_vars.front ();
        BOOST_REQUIRE (var->name () == "v");
        if (var->needs_unfolding ())
            a_debugger->unfold_variable
                (var, sigc::bind (&on_variable_unfolded_with_n_children,
                                  1, a_debugger));
        else
            a_debugger->step_over ();
        
    } else {
        // A vars should contain the last updated child of "v", as
        // well as v itself.
        BOOST_REQUIRE (a_vars.size () == 2);

        // And the first variable of a_vars should be "v".
        std::list<IDebugger::VariableSafePtr>::const_iterator it =
            a_vars.begin ();
        IDebugger::VariableSafePtr var = *it;
        BOOST_REQUIRE (var->name () == "v");
        // And v should contain n children.
        BOOST_REQUIRE (var->members ().size () == n);
        a_debugger->step_over ();
    }

    NEMIVER_CATCH_NOX;
}

static void
on_stopped_signal (IDebugger::StopReason /*a_reason*/,
                   bool a_has_frame,
                   const IDebugger::Frame &a_frame,
                   int, const string &,
                   const UString &/*a_cookie*/,
                   IDebuggerSafePtr &a_debugger)
{
    static int nb_stops_in_main = 0;

    NEMIVER_TRY;

    if (a_has_frame) {
        MESSAGE ("frame name: " << a_frame.function_name ());
        if (a_frame.function_name () == "main") {
            ++nb_stops_in_main;
            MESSAGE ("stopped " << nb_stops_in_main << " times in main");
            if (nb_stops_in_main < 3) {
                a_debugger->step_over ();
            } else if (nb_stops_in_main == 3) {
                // We just stopped right after the creation of the v
                // vector, which is empty for now.
                a_debugger->create_variable
                    ("v", sigc::bind (&on_variable_v_created_empty, a_debugger));
            } else if (nb_stops_in_main == 4) {
                // We should be stopped right after s1 has been
                // added to the v vector.
                BOOST_REQUIRE (v_var);
                a_debugger->list_changed_variables
                    (v_var,
                     sigc::bind (&on_variable_v_has_n_children,
                                 1,
                                 a_debugger));
            } else if (nb_stops_in_main == 5) {
                // We should be stopped right after s2 has been
                // added to the v vector.
                BOOST_REQUIRE (v_var);
                a_debugger->list_changed_variables
                    (v_var,
                     sigc::bind (&on_variable_v_has_n_children,
                                 2,
                                 a_debugger));
            } else if (nb_stops_in_main == 6) {
                // We should be stopped right after s3 has been
                // added to the v vector.
                BOOST_REQUIRE (v_var);
                a_debugger->list_changed_variables
                    (v_var,
                     sigc::bind (&on_variable_v_has_n_children,
                                 3,
                                 a_debugger));
            } else if (nb_stops_in_main){
                a_debugger->do_continue ();
            }
        }
    } else {
        a_debugger->step_over ();
    }

    NEMIVER_CATCH_NOX;
}

NEMIVER_API int
test_main (int, char **)
{
    NEMIVER_TRY;

    Initializer::do_init ();

    IDebuggerSafePtr debugger =
        debugger_utils::load_debugger_iface_with_confmgr ();

    // setup the debugger with the glib mainloop
    debugger->set_event_loop_context (Glib::MainContext::get_default ());

    //*******************************
    //<connect to IDebugger events>
    //******************************
    debugger->engine_died_signal ().connect (&on_engine_died_signal);
    debugger->program_finished_signal ().connect (&on_program_finished_signal);
    debugger->running_signal ().connect (&on_running_signal);
    debugger->stopped_signal ().connect (sigc::bind (&on_stopped_signal,
                                                     debugger));
    //*******************************
    //</connect to IDebugger events>
    //******************************

    debugger->enable_pretty_printing ();
    debugger->load_program ("prettyprint");
    debugger->set_breakpoint ("main");

    debugger->run ();

    //****************************************
    //run the event loop.
    //****************************************
    s_loop->run ();

    v_var.reset ();
    NEMIVER_CATCH_AND_RETURN_NOX (-1);

    return 0;
}
