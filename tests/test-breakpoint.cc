#include <iostream>
#include <glibmm.h>
#include "nmv-i-debugger.h"
#include "nmv-initializer.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-dynamic-module.h"

using namespace nemiver;
using namespace nemiver::common ;

Glib::RefPtr<Glib::MainLoop> loop =
    Glib::MainLoop::create (Glib::MainContext::get_default ()) ;

void
on_engine_died_signal ()
{
    std::cout << "engine died\n" ;
    loop->quit () ;
}

void
on_program_finished_signal ()
{
    std::cout << "program finished\n" ;
    loop->quit () ;
}

void
on_command_done_signal (const UString &a_command)
{
    std::cout << "command done: '" << a_command << "'\n" ;
}

void
on_breakpoints_set_signal (const std::map<int, IDebugger::BreakPoint> &a_breaks)
{
    std::cout  << "breakpoints set: \n" ;
    std::map<int, IDebugger::BreakPoint>::const_iterator it ;
    for (it = a_breaks.begin () ; it != a_breaks.end () ; ++it) {
        std::cout << "<break><num>" << it->first <<"</num><line>"
             << it->second.file_name () << ":" << it->second.line ()
             << "</line></break>"
             ;
    }
    std::cout << "\n" ;
}

void
on_running_signal ()
{
    std::cout << "debugger running ...\n" ;
}

void
on_got_proc_info_signal (int a_pid, const UString &a_exe_path)
{
    std::cout << "debugging program '"<< a_exe_path
              << "' of pid: '" << a_pid << "'\n" ;
}

void
on_stopped_signal (const UString &a_command,
                   bool a_has_frame,
                   const IDebugger::Frame &a_frame)
{
    std::cout << "stopped, reason is '" << a_command << "'\n" ;
    if (a_has_frame) {
        std::cout << "in frame: " << a_frame.function () ;
    }
    std::cout << "\n" ;
}

void
display_help ()
{
    std::cout << "test-basic <prog-to-debug>\n" ;
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

        THROW_IF_FAIL (loop) ;

        DynamicModuleManager module_manager ;
        IDebuggerSafePtr debugger =
                    module_manager.load<IDebugger> ("gdbengine") ;

        debugger->set_event_loop_context (loop->get_context ()) ;

        //*****************************
        //<connect to IDebugger events>
        //*****************************
        debugger->engine_died_signal ().connect
                (sigc::ptr_fun (&on_engine_died_signal)) ;

        debugger->program_finished_signal ().connect
                (sigc::ptr_fun (&on_program_finished_signal)) ;

        debugger->command_done_signal ().connect
                (sigc::ptr_fun (&on_command_done_signal)) ;

        debugger->breakpoints_set_signal ().connect
            (sigc::ptr_fun (&on_breakpoints_set_signal)) ;

        debugger->running_signal ().connect (sigc::ptr_fun (&on_running_signal));

        debugger->got_proc_info_signal ().connect
            (sigc::ptr_fun (&on_got_proc_info_signal)) ;

        debugger->stopped_signal ().connect
            (sigc::ptr_fun (&on_stopped_signal)) ;

        //*****************************
        //</connect to IDebugger events>
        //*****************************

        std::vector<UString> args, source_search_dir ;
        args.push_back (prog_to_debug) ;
        source_search_dir.push_back (".") ;

        debugger->load_program (args, source_search_dir);
        debugger->set_breakpoint ("main") ;
        debugger->run () ;
        debugger->set_breakpoint ("func1") ;
        debugger->set_breakpoint ("func2") ;
        debugger->list_breakpoints () ;
        debugger->do_continue () ;
        std::cout << "nb of breakpoints: "
                  << debugger->get_cached_breakpoints ().size ()
                  << "\n" ;
        debugger->do_continue () ;
        debugger->do_continue () ;
        loop->run () ;
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
