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

IDebugger::Frame s_current_frame ;

typedef std::list<IDebugger::VariableSafePtr> DebuggerVariableList ;
typedef std::map<std::string, IVarListWalkerSafePtr> VarListWalkerMap ;

VarListWalkerMap s_var_list_walker ;

IVarListWalkerSafePtr
create_var_list_walker ()
{
    IVarListWalkerSafePtr result  =
        DynamicModuleManager::load_iface_with_default_manager<IVarListWalker>
                                                                ("varwalkerlist",
                                                                 "IVarListWalker") ;
    return result ;
}

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

    a_debugger->list_local_variables () ;
    a_debugger->list_frames_arguments () ;

    if (a_frame.function_name () == "main") {
        if (s_var_list_walker.find ("main") == s_var_list_walker.end ()) {
            s_var_list_walker["main"] = create_var_list_walker () ;
        }
        a_debugger->do_continue() ;
    } else if (a_frame.function_name () == "func1") {
        if (s_var_list_walker.find ("func1") == s_var_list_walker.end ()) {
            s_var_list_walker["func1"] = create_var_list_walker () ;
        }
        a_debugger->do_continue() ;
    } else if (a_frame.function_name () == "func2") {
        if (s_var_list_walker.find ("func2") == s_var_list_walker.end ()) {
            s_var_list_walker["func2"] = create_var_list_walker () ;
        }
        a_debugger->do_continue() ;
    } else if (a_frame.function_name () == "func3") {
        if (s_var_list_walker.find ("func3") == s_var_list_walker.end ()) {
            s_var_list_walker["func3"] = create_var_list_walker () ;
        }
        a_debugger->do_continue() ;
    }
}

void
on_frames_arguments_listed_signal
        (const map<int, list<IDebugger::VariableSafePtr> > &a_frames_params,
         const UString &a_cookie)
{
    if (a_cookie.empty ()) {/*keep compiler happy*/}
    map<int, list<IDebugger::VariableSafePtr> >::const_iterator it ;
    it = a_frames_params.find (0) ;
    if (it == a_frames_params.end ()) {
        LOG_ERROR ("Could not find current frame") ;
        return ;
    }
    BOOST_REQUIRE (s_var_list_walker.find (s_current_frame.function_name ())
                   != s_var_list_walker.end ()) ;
    IVarListWalkerSafePtr walker = s_var_list_walker[s_current_frame.function_name ()];
    BOOST_REQUIRE (walker) ;
    walker->remove_variables () ;
    walker->append_variables (it->second) ;
    walker->do_walk_variables () ;
}

void
on_local_variables_listed_signal (const DebuggerVariableList &a_variables,
                                  const UString &a_cookie)
{
    if (a_variables.empty () || a_cookie.empty ()) {
    }
    BOOST_REQUIRE (s_var_list_walker.find (s_current_frame.function_name ())
                   != s_var_list_walker.end ()) ;
    IVarListWalkerSafePtr walker = s_var_list_walker[s_current_frame.function_name ()];
    BOOST_REQUIRE (walker) ;
    walker->remove_variables () ;
    walker->append_variables (a_variables) ;
    walker->do_walk_variables () ;
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
    debugger->frames_arguments_listed_signal ().connect
                            (&on_frames_arguments_listed_signal) ;
    debugger->local_variables_listed_signal ().connect
                            (&on_local_variables_listed_signal) ;
    //*******************************
    //</connect to IDebugger events>
    //******************************

    debugger->load_program (".libs/fooprog", ".") ;
    debugger->set_breakpoint ("main") ;
    debugger->set_breakpoint ("func1") ;
    debugger->set_breakpoint ("func2") ;
    debugger->set_breakpoint ("func3") ;
    s_var_list_walker["main"] = create_var_list_walker () ;
    s_var_list_walker["func1"] = create_var_list_walker () ;
    s_var_list_walker["func2"] = create_var_list_walker () ;
    s_var_list_walker["func3"] = create_var_list_walker () ;
    debugger->run () ;

    //****************************************
    //run the event loop.
    //****************************************
    s_loop->run () ;

    NEMIVER_CATCH_AND_RETURN_NOX (-1)

    return 0 ;
}
