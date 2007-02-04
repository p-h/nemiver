#include <iostream>
#include <glibmm.h>
#include "common/nmv-initializer.h"
#include "common/nmv-safe-ptr-utils.h"
#include "common/nmv-dynamic-module.h"
#include "common/nmv-exception.h"
#include "nmv-i-var-list.h"

using namespace std ;
using namespace nemiver ;
using namespace nemiver::common ;


void
display_help ()
{
    cout << "usage: runtestvarlist <program-containing variables>" << endl;
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
                   const UString &a_cookie)
{
    if (a_reason.empty () || a_has_frame || a_frame.level () || a_thread_id ||
        a_cookie.empty ()) {
        /*keeps compiler happy*/
    }
}

void
on_variable_value_signal (const UString &a_variable_name,
                          const IDebugger::VariableSafePtr &a_variable,
                          const UString &a_cookie)
{
    if (a_variable_name.empty () || a_variable || a_cookie.empty ()) {
        /*keep compiler happy*/
    }
}

void
on_variable_type_signal (const UString &a_variable_name,
                         const UString &a_variable_type,
                         const UString &a_cookie)
{
    if (a_variable_name.empty () || a_variable_type.empty () ||
        a_cookie.empty ()) {
    }
}

int
main (int argc, char **argv)
{
    NEMIVER_TRY

    if (argc != 2 || !argv) {
        display_help () ;
        return -1 ;
    }

    Initializer::do_init () ;

    /*
    Glib::RefPtr<Glib::MainLoop> event_loop =
        Glib::MainLoop::create (Glib::MainContext::get_default ()) ;
    */

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
    debugger->engine_died_signal ().connect (&on_engine_died_signal) ;
    debugger->program_finished_signal ().connect (&on_program_finished_signal);
    debugger->running_signal ().connect (&on_program_finished_signal) ;
    debugger->stopped_signal ().connect (&on_stopped_signal) ;
    debugger->variable_value_signal ().connect (&on_variable_value_signal) ;
    debugger->variable_type_signal ().connect (&on_variable_type_signal) ;

    //*******************************
    //</connect to IDebugger events>
    //******************************

    IVarListSafePtr var_list =
        DynamicModuleManager::load_iface_with_default_manager<IVarList>
                                                                ("varlist",
                                                                 "IVarList");
    var_list->initialize (debugger) ;


    NEMIVER_CATCH_AND_RETURN_NOX (-1)
    return 0 ;
}

