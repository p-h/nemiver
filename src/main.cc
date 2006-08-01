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

static gchar *gv_prog_arg=NULL ;

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
    {NULL}
};

int
main (int a_argc, char *a_argv[])
{
    Initializer::do_init () ;
    Gtk::Main main_loop (a_argc, a_argv);
    GOptionContext *context=NULL ;

    context = g_option_context_new ("- debug a program") ;
    g_option_context_add_main_entries (context, entries, "") ;
    g_option_context_add_group (context, gtk_get_option_group (TRUE)) ;
    g_option_context_parse (context, &a_argc, &a_argv, NULL) ;

    NEMIVER_TRY

    DynamicModuleManager module_manager ;

    IWorkbenchSafePtr workbench = module_manager.load<IWorkbench> ("workbench");
    workbench->do_init (main_loop) ;

    if (gv_prog_arg) {
        IDBGPerspective *debug_persp =
            dynamic_cast<IDBGPerspective*> (workbench->get_perspective
                                                            ("DBGPerspective")) ;
        if (debug_persp) {
            LOG ("going to debug program: '" << UString (gv_prog_arg) << "'\n") ;
            debug_persp->execute_program (UString (gv_prog_arg)) ;
        } else {
            cerr << "Could not find the DBGPerspective\n" ;
            return -1 ;
        }
        g_free (gv_prog_arg);
        gv_prog_arg = NULL;

    }

    //***************************
    //parse command line options
    //***************************

    main_loop.run (workbench->get_root_window ()) ;

    NEMIVER_CATCH


    return 0 ;
}

