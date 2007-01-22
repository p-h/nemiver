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
on_command_done_signal (const UString &a_command,
                        const UString &a_cookie)
{
    std::cout << "command done: '"
              << a_command
              << "', cookie: '"
              << a_cookie
              << "'\n" ;
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
                   const IDebugger::Frame &a_frame,
                   int a_thread_id)
{
    std::cout << "stopped, reason is '" << a_command << "'\n" ;
    if (a_has_frame) {
        std::cout << "in frame: " << a_frame.function_name () << "\n";
    }
    std::cout << "thread-id is '" << a_thread_id << "'\n" ;
}

void
on_variable_type_signal (const UString &a_variable_name,
                         const UString &a_variable_type)
{
    std::cout << "type of variable '" << a_variable_name
              << "' is '" << a_variable_type << "'\n" ;
}

void
on_threads_listed_signal (const std::list<int> &a_thread_ids)
{

    std::cout << "number of threads: '" << a_thread_ids.size () << "'\n";
    std::list<int>::const_iterator it ;
    for (it = a_thread_ids.begin () ; it != a_thread_ids.end () ; ++it) {
        std::cout << "thread-id: '" << *it << "'\n";
    }
}

void
on_thread_selected_signal (int a_thread_id,
                           const IDebugger::Frame &a_frame)
{
    std::cout << "thread selected: '" << a_thread_id << "'\n" ;
    std::cout << "frame in thread : '" << a_frame.level () << "'\n" ;
    std::cout << "frame.function: '" << a_frame.function_name () << "'\n" ;
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
                module_manager.load_iface<IDebugger> ("gdbengine", "IDebugger");

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

        debugger->got_target_info_signal ().connect
            (sigc::ptr_fun (&on_got_proc_info_signal)) ;

        debugger->stopped_signal ().connect
            (sigc::ptr_fun (&on_stopped_signal)) ;

        debugger->variable_type_signal ().connect
            (sigc::ptr_fun (&on_variable_type_signal)) ;

        debugger->threads_listed_signal ().connect
            (sigc::ptr_fun (&on_threads_listed_signal)) ;

        debugger->thread_selected_signal ().connect
            (sigc::ptr_fun (&on_thread_selected_signal)) ;

        //*****************************
        //</connect to IDebugger events>
        //*****************************

        std::vector<UString> args, source_search_dir ;
        args.push_back (prog_to_debug) ;
        source_search_dir.push_back (".") ;

        debugger->load_program (args, "", source_search_dir);
        debugger->set_breakpoint ("main") ;
        debugger->run () ;
        debugger->list_threads () ;
        debugger->select_thread (1) ;
        debugger->set_breakpoint ("func1") ;
        debugger->set_breakpoint ("func2") ;
        debugger->list_breakpoints () ;
        debugger->do_continue () ;
        std::cout << "nb of breakpoints: "
                  << debugger->get_cached_breakpoints ().size ()
                  << "\n" ;
        debugger->step_over () ;
        debugger->print_variable_type ("i") ;
        debugger->step_out () ;
        debugger->print_variable_type ("person") ;
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
