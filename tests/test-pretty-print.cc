#include "config.h"
#include <iostream>
#include <map>
#include <string>
#include <boost/test/minimal.hpp>
#include <glibmm.h>
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "nmv-debugger-utils.h"
#include "test-utils.h"

using namespace nemiver;
using namespace nemiver::common;

Glib::RefPtr<Glib::MainLoop> s_loop =
    Glib::MainLoop::create (Glib::MainContext::get_default ());

namespace {

/// TestObject will contain the test data
/// as sigc::bind () is limited in args count
/// It is a non copyable object.
struct TestObject {
  IDebuggerSafePtr debugger;
  IDebugger::VariableSafePtr v_var;

  TestObject () { };

  /// Prevent copying.
  TestObject (const TestObject &) = delete;
};

}

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
                                      TestObject & test_object)
{
    NEMIVER_TRY;

    BOOST_REQUIRE (a_var->name () == "v");
    BOOST_REQUIRE (a_var->members ().size () == n);
    test_object.debugger->step_over ();

    NEMIVER_CATCH_NOX;
}

static void
on_variable_v_created_empty (const IDebugger::VariableSafePtr a_var,
                             TestObject & test_object)
{
    NEMIVER_TRY;

    BOOST_REQUIRE (a_var->name () == "v");
    BOOST_REQUIRE (a_var->is_dynamic ());
    BOOST_REQUIRE (a_var->members ().empty ());
    test_object.v_var = a_var;
    test_object.debugger->step_over ();

    NEMIVER_CATCH_NOX;
}

static void
on_variable_v_has_n_children (const std::list<IDebugger::VariableSafePtr> &a_vars,
                              unsigned n, TestObject & test_object)
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
            test_object.debugger->unfold_variable
                (var, sigc::bind (&on_variable_unfolded_with_n_children,
                                  1, sigc::ref (test_object)));
        else
            test_object.debugger->step_over ();
        
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
        test_object.debugger->step_over ();
    }

    NEMIVER_CATCH_NOX;
}

static void
on_stopped_signal (IDebugger::StopReason /*a_reason*/,
                   bool a_has_frame,
                   const IDebugger::Frame &a_frame,
                   int, const string &,
                   const UString &/*a_cookie*/,
                   TestObject & test_object)
{
    static int nb_stops_in_main = 0;

    NEMIVER_TRY;

    if (a_has_frame) {
        MESSAGE ("frame name: " << a_frame.function_name ());
        if (a_frame.function_name () == "main") {
            ++nb_stops_in_main;
            MESSAGE ("stopped " << nb_stops_in_main << " times in main");
            if (nb_stops_in_main < 3) {
                test_object.debugger->step_over ();
            } else if (nb_stops_in_main == 3) {
                // We just stopped right after the creation of the v
                // vector, which is empty for now.
                test_object.debugger->create_variable
                  ("v", sigc::bind (&on_variable_v_created_empty, sigc::ref (test_object)));
            } else if (nb_stops_in_main == 4) {
                // We should be stopped right after s1 has been
                // added to the v vector.
                BOOST_REQUIRE (test_object.v_var);
                test_object.debugger->list_changed_variables
                    (test_object.v_var,
                     sigc::bind (&on_variable_v_has_n_children,
                                 1,
                                 sigc::ref (test_object)));
            } else if (nb_stops_in_main == 5) {
                // We should be stopped right after s2 has been
                // added to the v vector.
                BOOST_REQUIRE (test_object.v_var);
                test_object.debugger->list_changed_variables
                    (test_object.v_var,
                     sigc::bind (&on_variable_v_has_n_children,
                                 2,
                                 sigc::ref (test_object)));
            } else if (nb_stops_in_main == 6) {
                // We should be stopped right after s3 has been
                // added to the v vector.
                BOOST_REQUIRE (test_object.v_var);
                test_object.debugger->list_changed_variables
                    (test_object.v_var,
                     sigc::bind (&on_variable_v_has_n_children,
                                 3,
                                 sigc::ref (test_object)));
            } else if (nb_stops_in_main){
                test_object.debugger->do_continue ();
            }
        }
    } else {
        test_object.debugger->step_over ();
    }

    NEMIVER_CATCH_NOX;
}

NEMIVER_API int
test_main (int, char **)
{
    NEMIVER_TRY;

    Initializer::do_init ();

    TestObject test_object;

    test_object.debugger =
        debugger_utils::load_debugger_iface_with_confmgr ();

    // setup the debugger with the glib mainloop
    test_object.debugger->set_event_loop_context (Glib::MainContext::get_default ());

    //*******************************
    //<connect to IDebugger events>
    //******************************
    test_object.debugger->engine_died_signal ().connect (&on_engine_died_signal);
    test_object.debugger->program_finished_signal ().connect (&on_program_finished_signal);
    test_object.debugger->running_signal ().connect (&on_running_signal);
    test_object.debugger->stopped_signal ().connect (sigc::bind (&on_stopped_signal,
                                                     sigc::ref (test_object)));
    //*******************************
    //</connect to IDebugger events>
    //******************************

    test_object.debugger->enable_pretty_printing ();
    test_object.debugger->load_program ("prettyprint");
    test_object.debugger->set_breakpoint ("main");

    test_object.debugger->run ();

    //****************************************
    //run the event loop.
    //****************************************
    NEMIVER_SETUP_TIMEOUT (s_loop, 10);
    s_loop->run ();

    // we shouldn't have timeout.
    NEMIVER_CHECK_NO_TIMEOUT;

    test_object.v_var.reset ();
    NEMIVER_CATCH_AND_RETURN_NOX (-1);

    return 0;
}
