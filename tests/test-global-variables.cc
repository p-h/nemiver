#include <iostream>
#include <map>
#include <string>
#include <boost/test/test_tools.hpp>
#include <boost/test/minimal.hpp>
#include <glibmm.h>
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "nmv-i-var-list-walker.h"

using namespace nemiver ;
using namespace nemiver::common ;

Glib::RefPtr<Glib::MainLoop> s_loop =
    Glib::MainLoop::create (Glib::MainContext::get_default ()) ;

IDebugger::Frame s_current_frame ;

typedef std::list<IDebugger::VariableSafePtr> DebuggerVariableList ;
typedef std::map<std::string, IVarListWalkerSafePtr> VarListWalkerMap ;
typedef std::map<std::string, string> StringMap ;

void
on_command_done_signal (const UString &a_name,
                        const UString &a_cookie)
{
    if (a_cookie.empty ()) {
    }
    MESSAGE ("command " << a_name << " done") ;
}

void
on_engine_died_signal ()
{
}

void
on_program_finished_signal ()
{
}

void
on_running_signal ()
{
}

void
on_stopped_signal (const UString &a_reason,
                   bool a_has_frame,
                   const IDebugger::Frame &a_frame,
                   int a_thread_id,
                   const UString &a_cookie,
                   IDebuggerSafePtr &a_debugger)
{
    BOOST_REQUIRE (a_debugger) ;

    if (a_reason.empty () || a_has_frame || a_frame.level () || a_thread_id ||
        a_cookie.empty () || a_debugger) {
        /*keeps compiler happy*/
    }

    if (a_reason == "exited-normally") {
        MESSAGE ("program exited normally") ;
        s_loop->quit () ;
        return ;
    }

    if (!a_has_frame) {
        MESSAGE ("stopped, but not in a frame, reason: " << a_reason) ;
        return ;
    }

    s_current_frame = a_frame ;

    MESSAGE ("stopped in function: '"
             << a_frame.function_name ()
             << "()'") ;

    a_debugger->list_global_variables () ;
}

void
on_global_variables_listed_signal (const std::list<IDebugger::VariableSafePtr> &a_vars,
                                   const UString &a_cookie,
                                   IDebuggerSafePtr &a_debugger)
{
    if (a_cookie.empty ()) {}

    NEMIVER_TRY

    if (a_vars.size () == 0) {
        MESSAGE ("got zero global variables") ;
        return ;
    }
    MESSAGE ("got global variables") ;
    std::list<IDebugger::VariableSafePtr>::const_iterator it ;
    for (it = a_vars.begin (); it != a_vars.end (); ++it) {
        cout << "==============================\n";
        cout << "name: " << (*it)->name () << "\n";
        cout << "type: " << (*it)->type () << "\n";
        cout << "==============================\n";
    }

    a_debugger->do_continue () ;

    NEMIVER_CATCH_NOX
}

NEMIVER_API int
test_main (int argc, char **argv)
{
    if (argc || argv) {/*keep compiler happy*/}

    NEMIVER_TRY

    Initializer::do_init () ;


    //load the IDebugger interface
    IDebuggerSafePtr debugger =
        DynamicModuleManager::load_iface_with_default_manager<IDebugger>
                                                                ("gdbengine",
                                                                 "IDebugger") ;
    //setup the debugger with the glib mainloop
    debugger->set_event_loop_context (Glib::MainContext::get_default ()) ;

    //*******************************
    //<connect to IDebugger events>
    //******************************
    debugger->command_done_signal ().connect (&on_command_done_signal) ;
    debugger->engine_died_signal ().connect (&on_engine_died_signal) ;
    debugger->program_finished_signal ().connect (&on_program_finished_signal);
    debugger->running_signal ().connect (&on_running_signal) ;
    debugger->stopped_signal ().connect (sigc::bind (&on_stopped_signal,
                                                     debugger)) ;
    debugger->global_variables_listed_signal ().connect
                            (sigc::bind (&on_global_variables_listed_signal, debugger)) ;
    //*******************************
    //</connect to IDebugger events>
    //******************************

    debugger->load_program (".libs/fooprog", ".") ;
    debugger->set_breakpoint ("main") ;
    debugger->run () ;

    //********************
    //run the event loop.
    //********************
    s_loop->run () ;

    NEMIVER_CATCH_AND_RETURN_NOX (-1)

    return 0 ;
}


