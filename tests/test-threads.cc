#include "config.h"
#include <iostream>
#include <boost/test/minimal.hpp>
#include "common/nmv-initializer.h"
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-exception.h"
#include "nmv-debugger-utils.h"

using namespace nemiver;
using namespace nemiver::common;

static Glib::RefPtr<Glib::MainLoop> loop =
    Glib::MainLoop::create (Glib::MainContext::get_default ());

static unsigned num_threads = 0;

static void
on_engine_died_signal ()
{
    MESSAGE ("engine died");
    loop->quit ();
}

static void
on_program_finished_signal ()
{
    MESSAGE ("program finished");
    BOOST_REQUIRE (num_threads == 5);
    loop->quit ();
}

static void
on_stopped_signal (IDebugger::StopReason a_reason,
                   bool /*a_has_frame*/,
                   const IDebugger::Frame &/*a_frame*/,
                   int /*a_thread_id*/,
                   const string &/*a_bp_num*/,
                   const UString &/*a_cookie*/,
                   IDebuggerSafePtr &a_debugger)
{
    if (a_reason == IDebugger::BREAKPOINT_HIT)
        a_debugger->print_variable_value ("tid");

    a_debugger->do_continue ();
}

static void
on_variable_value_signal (const UString&,
                          const IDebugger::VariableSafePtr a_expr,
                          const UString&)
{
    if (a_expr->name () == "tid")
        ++num_threads;
}

NEMIVER_API int
test_main (int, char *[])
{
    NEMIVER_TRY;

    Initializer::do_init ();

    THROW_IF_FAIL (loop);

    IDebuggerSafePtr debugger =
        debugger_utils::load_debugger_iface_with_confmgr ();

    debugger->set_event_loop_context (loop->get_context ());

    //*****************************
    //<connect to IDebugger events>
    //*****************************

    debugger->engine_died_signal ().connect (&on_engine_died_signal);

    debugger->program_finished_signal ().connect
        (&on_program_finished_signal);

    debugger->variable_value_signal ().connect
        (&on_variable_value_signal);

    debugger->stopped_signal ().connect
        (sigc::bind (&on_stopped_signal, debugger));

    //*****************************
    //</connect to IDebugger events>
    //*****************************

    std::vector<UString> args, source_search_dir;
    debugger->enable_pretty_printing (false);
    source_search_dir.push_back (".");
    debugger->load_program ("threads", args, ".",
                            source_search_dir, "",
                            false);
    debugger->set_breakpoint ("threads.cc", 32);

    debugger->run ();
    loop->run ();

    NEMIVER_CATCH_NOX;

    return 0;
}
