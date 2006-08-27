/*
 *This file is part of the Nemiver project
 *
 *Nemiver is free software; you can redistribute
 *it and/or modify it under the terms of
 *the GNU General Public License as published by the
 *Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Nemiver is distributed in the hope that it will
 *be useful, but WITHOUT ANY WARRANTY;
 *without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include <iostream>
#include <gtkmm.h>
#include <libglademm.h>
#include "nmv-exception.h"
#include "nmv-initializer.h"
#include "nmv-i-workbench.h"
#include "nmv-ui-utils.h"
#include "nmv-env.h"
#include "nmv-dbg-perspective.h"

using namespace std ;
using nemiver::common::DynamicModuleManager ;
using nemiver::common::Initializer ;
using nemiver::IWorkbench ;
using nemiver::IWorkbenchSafePtr ;
using nemiver::IDBGPerspective ;
using nemiver::common::UString ;
using nemiver::ISessMgr ;

static gchar *gv_prog_arg=NULL ;
static bool gv_list_sessions=false ;
static bool gv_purge_sessions=false ;
static int gv_execute_session=0;
static gchar *gv_log_domains=0;
static bool gv_log_debugger_output=false ;

static GOptionEntry entries[] =
{
    { "debug",
      'd',
      0,
      G_OPTION_ARG_STRING,
      &gv_prog_arg,
      "debug a prog",
      "<prog-name-and-args>"
    },
    { "listsessions",
      0,
      0,
      G_OPTION_ARG_NONE,
      &gv_list_sessions,
      "list the saved debugging sessions",
      NULL
    },
    { "purgesessions",
      0,
      0,
      G_OPTION_ARG_NONE,
      &gv_purge_sessions,
      "erase the saved debugging sessions",
      NULL
    },
    { "executesession",
      0,
      0,
      G_OPTION_ARG_INT,
      &gv_execute_session,
      "debug the program that was of session number N",
      "N"
    },
    { "log-domains",
      0,
      0,
      G_OPTION_ARG_STRING,
      &gv_log_domains,
      "Enable logging domains DOMAINS",
      "DOMAINS"
    },
    { "logdebuggeroutput",
      0,
      0,
      G_OPTION_ARG_NONE,
      &gv_log_debugger_output,
      "log the debugger output",
      NULL
    },
    {NULL}
};

int
main (int a_argc, char *a_argv[])
{
    Initializer::do_init () ;
    Gtk::Main main_loop (a_argc, a_argv);
    GOptionContext *context=NULL ;

    //***************************
    //parse command line options
    //***************************
    context = g_option_context_new ("- a C/C++ debugger for GNOME") ;
    g_option_context_add_main_entries (context, entries, "") ;
    g_option_context_add_group (context, gtk_get_option_group (TRUE)) ;
    g_option_context_set_ignore_unknown_options (context, FALSE) ;
    g_option_context_parse (context, &a_argc, &a_argv, NULL) ;

    NEMIVER_TRY

    //**********************************
    //load the workbench dynamic module
    //**********************************
    DynamicModuleManager module_manager ;

    IWorkbenchSafePtr workbench = module_manager.load<IWorkbench> ("workbench");
    workbench->do_init (main_loop) ;

    //********************************
    //<process command line arguments>
    //********************************
    if (gv_log_debugger_output) {
        workbench->get_properties ()["log-debugger-output"] = "yes" ;
        LOG_STREAM.enable_domain ("gdbmi-output-domain") ;
    }

    if (gv_log_domains) {
        UString log_domains (gv_log_domains) ;
        vector<UString> domains = log_domains.split (" ") ;
        for (vector<UString>::const_iterator iter = domains.begin () ;
             iter != domains.end ();
             ++iter) {
            LOG_STREAM.enable_domain (*iter) ;
        }
    }

    if (gv_list_sessions) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (workbench->get_perspective
                                                            ("DBGPerspective")) ;
        if (debug_persp) {
            debug_persp->session_manager ().load_sessions () ;
            list<ISessMgr::Session>::iterator session_iter ;
            list<ISessMgr::Session>& sessions =
                            debug_persp->session_manager ().sessions () ;
            for (session_iter = sessions.begin ();
                 session_iter != sessions.end ();
                 ++session_iter) {
                cout << session_iter->session_id ()
                     << " "
                     << session_iter->properties ()["sessionname"]
                     << "\n"
                     ;
            }
            return 0 ;
        } else {
            cerr << "Could not find the DBGPerspective\n" ;
            return -1 ;
        }
    }

    if (gv_purge_sessions) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (workbench->get_perspective
                                                            ("DBGPerspective")) ;
        if (debug_persp) {
            debug_persp->session_manager ().delete_sessions () ;
        }
        return 0 ;
    }

    if (gv_execute_session) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (workbench->get_perspective
                                                            ("DBGPerspective")) ;
        if (debug_persp) {
            debug_persp->session_manager ().load_sessions () ;
            list<ISessMgr::Session>::iterator session_iter ;
            list<ISessMgr::Session>& sessions =
                            debug_persp->session_manager ().sessions () ;
            bool found_session=false ;
            for (session_iter = sessions.begin ();
                 session_iter != sessions.end ();
                 ++session_iter) {
                if (session_iter->session_id () == gv_execute_session) {
                    debug_persp->execute_session (*session_iter) ;
                    found_session = true ;
                }
            }

            if (!found_session) {
                cerr << "Could not find session of number "
                     << gv_execute_session
                     << "\n";
                return -1 ;
            }
            goto run_app ;
        }
    }


    if (gv_prog_arg) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (workbench->get_perspective
                                                            ("DBGPerspective")) ;
        if (debug_persp) {
            LOG_D ("going to debug program: '"
                   << UString (gv_prog_arg) << "'\n",
                   NMV_DEFAULT_DOMAIN) ;
            debug_persp->execute_program (UString (gv_prog_arg)) ;
        } else {
            cerr << "Could not find the DBGPerspective\n" ;
            return -1 ;
        }
        g_free (gv_prog_arg);
        gv_prog_arg = NULL;
        goto run_app ;

    }
    //********************************
    //</process command line arguments>
    //********************************


run_app:

    workbench->get_root_window ().show_all () ;
    main_loop.run (workbench->get_root_window ()) ;

    NEMIVER_CATCH_NOX

    return 0 ;
}

