//Author: Dodji Seketeli
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
#include <signal.h>
#include <iostream>
#include <gtkmm/window.h>
#include <libglademm.h>
#include <glib/gi18n.h>
#include "nmv-exception.h"
#include "nmv-initializer.h"
#include "nmv-i-workbench.h"
#include "nmv-ui-utils.h"
#include "nmv-proc-mgr.h"
#include "nmv-env.h"
#include "nmv-dbg-perspective.h"
#include "config.h"

using namespace std ;
using nemiver::common::DynamicModuleManager ;
using nemiver::common::Initializer ;
using nemiver::IWorkbench ;
using nemiver::IWorkbenchSafePtr ;
using nemiver::IDBGPerspective ;
using nemiver::common::UString ;
using nemiver::ISessMgr ;

static const UString DBGPERSPECTIVE_PLUGIN_NAME="dbgperspective" ;
static gchar *gv_process_to_attach_to=0;
static bool gv_list_sessions=false ;
static bool gv_purge_sessions=false ;
static int gv_execute_session=0;
static gchar *gv_log_domains=0;
static bool gv_log_debugger_output=false ;

static GOptionEntry entries[] =
{
    {
      "attach",
      0,
      0,
      G_OPTION_ARG_STRING,
      &gv_process_to_attach_to,
      _("attach to a process"),
      "<pid|process name>"
    },
    { "listsessions",
      0,
      0,
      G_OPTION_ARG_NONE,
      &gv_list_sessions,
      _("list the saved debugging sessions"),
      NULL
    },
    { "purgesessions",
      0,
      0,
      G_OPTION_ARG_NONE,
      &gv_purge_sessions,
      _("erase the saved debugging sessions"),
      NULL
    },
    { "executesession",
      0,
      0,
      G_OPTION_ARG_INT,
      &gv_execute_session,
      _("debug the program that was of session number N"),
      "N"
    },
    { "log-domains",
      0,
      0,
      G_OPTION_ARG_STRING,
      &gv_log_domains,
      _("Enable logging domains DOMAINS"),
      "DOMAINS"
    },
    { "logdebuggeroutput",
      0,
      0,
      G_OPTION_ARG_NONE,
      &gv_log_debugger_output,
      _("log the debugger output"),
      NULL
    },
    {0,0,0,(GOptionArg)0,0,0,0}
};

struct GOptionContextUnref {
    void operator () (GOptionContext *a_opt)
    {
        if (a_opt) {
            g_option_context_free (a_opt) ;
        }
    }
};//end struct GOptoinContextUnref

struct GOptionContextRef {
    void operator () (GOptionContext *a_opt)
    {
        if (a_opt) {}
    }
};//end struct GOptoinContextRef

static IWorkbench *s_workbench=0 ;

void
sigint_handler (int a_signum)
{
    if (a_signum != SIGINT) {
        return ;
    }
    static bool s_got_down = false ;
    if (!s_got_down && s_workbench) {
        s_workbench->shut_down () ;
        s_workbench = 0 ;
        s_got_down = true ;
    }
}

int
main (int a_argc, char *a_argv[])
{
    bindtextdomain (GETTEXT_PACKAGE, NEMIVERLOCALEDIR) ;
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8") ;
    textdomain (GETTEXT_PACKAGE) ;
    Initializer::do_init () ;
    Gtk::Main gtk_kit (a_argc, a_argv);
    typedef SafePtr<GOptionContext,
                    GOptionContextRef,
                    GOptionContextUnref> GOptionContextSafePtr ;
    GOptionContextSafePtr context ;

    //***************************
    //parse command line options
    //***************************
    context.reset (g_option_context_new
            (_(" [<prog-to-debug> [prog-args]] - a C/C++ debugger for GNOME"))) ;
    g_option_context_add_main_entries (context.get (), entries, "") ;
    g_option_context_add_group (context.get (), gtk_get_option_group (TRUE)) ;
    g_option_context_set_ignore_unknown_options (context.get (), FALSE) ;
    g_option_context_parse (context.get (), &a_argc, &a_argv, NULL) ;

    NEMIVER_TRY

    //**********************************
    //load the workbench dynamic module
    //**********************************
    DynamicModuleManager module_manager ;

    IWorkbenchSafePtr workbench = module_manager.load<IWorkbench> ("workbench");
    s_workbench = workbench.get () ;
    LOG_D ("workbench refcount: " <<  (int) s_workbench->get_refcount (),
            "refcount-domain") ;
    s_workbench->do_init (gtk_kit) ;
    LOG_D ("workbench refcount: " <<  (int) s_workbench->get_refcount (),
           "refcount-domain") ;

    //********************************
    //<process command line arguments>
    //********************************
    if (gv_log_debugger_output) {
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

    if (gv_process_to_attach_to) {
        using nemiver::common::IProcMgrSafePtr ;
        using nemiver::common::IProcMgr ;

        IDBGPerspective *debug_persp =
        dynamic_cast<IDBGPerspective*> (s_workbench->get_perspective
                                                    (DBGPERSPECTIVE_PLUGIN_NAME));
        if (!debug_persp) {
            cerr << "Could not get the debugging perspective" << endl ;
            return -1 ;
        }
        int pid = atoi (gv_process_to_attach_to) ;
        if (!pid) {
            IProcMgrSafePtr proc_mgr = IProcMgr::create () ;
            if (!proc_mgr) {
                cerr << "Could not create proc mgr" << endl ;
                return -1 ;
            }
            IProcMgr::Process process ;
            if (!proc_mgr->get_process_from_name (gv_process_to_attach_to,
                        process,
                        true)) {
                cerr << "Could not find any process named '"
                << gv_process_to_attach_to
                << "'"
                << endl
                ;
                return -1 ;
            }
            pid = process.pid () ;
        }
        if (!pid) {
            cerr << "Could not find any process '"
                 << gv_process_to_attach_to
                 << "'"
                 << endl
                 ;
            return -1 ;
        } else {
            debug_persp->attach_to_program (pid) ;
        }
    }

    if (gv_list_sessions) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (s_workbench->get_perspective
                                                    (DBGPERSPECTIVE_PLUGIN_NAME));
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
            cerr << "Could not find the debugger perpective plugin" ;
            return -1 ;
        }
    }

    if (gv_purge_sessions) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (s_workbench->get_perspective
                                                    (DBGPERSPECTIVE_PLUGIN_NAME)) ;
        if (debug_persp) {
            debug_persp->session_manager ().delete_sessions () ;
        }
        return 0 ;
    }

    if (gv_execute_session) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (s_workbench->get_perspective
                                                    (DBGPERSPECTIVE_PLUGIN_NAME)) ;
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
                    break;
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


    if (a_argc > 1) {
        UString prog_args ;
        for (int i=1 ; i < a_argc ;++i) {
            prog_args += Glib::locale_to_utf8 (a_argv[i]) ;
        }
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (s_workbench->get_perspective
                                                            (DBGPERSPECTIVE_PLUGIN_NAME)) ;
        if (debug_persp) {
            LOG_D ("going to debug program: '"
                   << prog_args << "'\n",
                   NMV_DEFAULT_DOMAIN) ;
            map<UString, UString> env ;
            debug_persp->execute_program (prog_args, env) ;
        } else {
            cerr << "Could not find the debugger perspective plugin\n" ;
            return -1 ;
        }
        goto run_app ;

    }
    //********************************
    //</process command line arguments>
    //********************************


run_app:

    //intercept ctrl-c/SIGINT
    signal (SIGINT, sigint_handler) ;
    s_workbench->get_root_window ().show_all () ;
    gtk_kit.run (s_workbench->get_root_window ()) ;

    NEMIVER_CATCH_NOX
    s_workbench = 0 ;

    return 0 ;
}

