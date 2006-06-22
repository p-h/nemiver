#include <iostream>
#include <glibmm.h>
#include "nmv-i-debugger.h"
#include "nmv-initializer.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-dynamic-module.h"

using namespace std ;
using namespace nemiver;
using namespace nemiver::common ;

void
on_pty_signal (IDebugger::Output &a_result)
{
    UString parsing_succeeded ;

    if (a_result.parsing_succeeded ()) {
        parsing_succeeded = "succeded";
    } else {
        parsing_succeeded = "failed" ;
    }

    cout <<"**********************************\n"
         <<"<pty parsing='" << parsing_succeeded << "'>\n"
         <<"**********************************\n"
         << a_result.raw_value () << "\n"
         <<"**********************************\n"
         <<"</pty>\n"
         <<"**********************************\n"
     ;
}

void
on_stdout_signal (IDebugger::CommandAndOutput &a_result)
{
    UString parsing_succeeded ;

    if (a_result.output ().parsing_succeeded ()) {
        parsing_succeeded = "succeded";
    } else {
        parsing_succeeded = "failed" ;
    }

    cout <<"**********************************\n"
         <<"<stdout parsing='" << parsing_succeeded << "'" ;
    if (a_result.has_command ()) {
        cout << " command='" << a_result.command ().value () << "'>\n" ;
    } else {
        cout << ">\n" ;
    }

    cout <<"**********************************\n"
         << a_result.output ().raw_value () << "\n"
         <<"**********************************\n"
         <<"</stdout>\n"
         <<"**********************************\n"
     ;
}
void
on_stderr_signal (IDebugger::Output &a_result)
{
    UString parsing_succeeded ;

    if (a_result.parsing_succeeded ()) {
        parsing_succeeded = "succeded";
    } else {
        parsing_succeeded = "failed" ;
    }

    cout <<"**********************************\n"
         <<"<stderr parsing='" << parsing_succeeded << "'>\n"
         <<"**********************************\n"
         << a_result.raw_value () << "\n"
         <<"**********************************\n"
         <<"</stderr>\n"
         <<"**********************************\n"
     ;
}

void
on_engine_died_signal ()
{
    cout << "!!!!!engine died!!!!\n" ;
}

void
display_help ()
{
    cout << "test-basic <prog-to-debug>\n" ;
}

int
main (int argc, char *argv[])
{
    if (argc != 2) {
        display_help () ;
        return -1 ;
    }

    UString prog_to_debug = argv[1] ;

    try {
        Initializer::do_init () ;
        Glib::RefPtr<Glib::MainLoop> loop =
            Glib::MainLoop::create (Glib::MainContext::get_default ()) ;

        THROW_IF_FAIL (loop) ;

        DynamicModuleManager module_manager ;
        IDebuggerSafePtr debugger =
                    module_manager.load<IDebugger> ("gdbengine") ;

        debugger->set_event_loop_context (loop->get_context ()) ;

        debugger->pty_signal ().connect (sigc::ptr_fun (&on_pty_signal)) ;

        debugger->stdout_signal ().connect
                (sigc::ptr_fun (&on_stdout_signal)) ;

        debugger->stderr_signal ().connect
                (sigc::ptr_fun (&on_stderr_signal)) ;

        debugger->engine_died_signal ().connect
                (sigc::ptr_fun (&on_engine_died_signal)) ;

        vector<UString> args, source_search_dir ;
        args.push_back (prog_to_debug) ;
        source_search_dir.push_back (".") ;

        debugger->load_program (args, source_search_dir);
        sleep (1) ;
        debugger->set_breakpoint ("main") ;
        sleep (1) ;
        debugger->run () ;
        sleep (1) ;
        debugger->set_breakpoint ("func1") ;
        sleep (1) ;
        debugger->set_breakpoint ("func2") ;
        sleep (1) ;
        debugger->list_breakpoints () ;
        sleep (1) ;
        debugger->do_continue () ;
        sleep (1) ;
        cout << "nb of breakpoints: "
             << debugger->get_cached_breakpoints ().size ()
             << "\n" ;
        debugger->do_continue () ;
        sleep (1) ;
        debugger->do_continue () ;
        sleep (1) ;
        debugger->run_loop_iterations (-1) ;
    } catch (Glib::Exception &e) {
        LOG_ERROR ("got error: " << e.what () << "\n") ;
        return -1 ;
    } catch (exception &e) {
        LOG_ERROR ("got error: " << e.what () << "\n") ;
        return -1 ;
    } catch (...) {
        LOG_ERROR ("got an unknown error\n") ;
        return -1 ;
    }
    return 0 ;
}
