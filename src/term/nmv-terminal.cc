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

#include <pty.h>
#include <iostream>
#include "nmv-ustring.h"
#include <gtkmm/bin.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <vte/vte.h>
#include "nmv-exception.h"
#include "nmv-log-stream-utils.h"
#include "nmv-env.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-initializer.h"
#include <utmp.h>

using nemiver::common::UString ;
using nemiver::common::Object ;
using nemiver::common::ObjectRef;
using nemiver::common::ObjectUnref;

NEMIVER_BEGIN_NAMESPACE(nemiver)

class Terminal : public Object {
    //non copyable
    Terminal (const Terminal &) ;
    Terminal& operator= (const Terminal &) ;

    //the master pty of the terminal (and of the whole process)
    int m_master_pty ;

    int m_slave_pty ;

    //the real vte terminal widget
    VteTerminal *m_vte ;

    //the same object as
    //m_vte, but wrapped as a Gtk::Widget
    Gtk::Widget *m_widget ;

    bool init_pty () ;

public:

    Terminal () ;
    ~Terminal () ;
    Gtk::Widget& widget () const ;
    UString slave_pts_name () const ;
};//end class Terminal

Terminal::Terminal () :
        m_master_pty (0),
        m_slave_pty (0),
        m_vte (NULL),
        m_widget (NULL)
{
    GtkWidget *w = vte_terminal_new () ;
    m_vte = VTE_TERMINAL (w) ;
    THROW_IF_FAIL (m_vte) ;

    m_widget = Glib::wrap (w) ;
    THROW_IF_FAIL (m_widget) ;

    m_widget->reference () ;
    THROW_IF_FAIL (init_pty ()) ;

}

Terminal::~Terminal ()
{
    if (m_slave_pty) {
        close (m_slave_pty) ;
        m_slave_pty = 0 ;
    }

    if (m_master_pty) {
        close (m_master_pty) ;
        m_master_pty = 0 ;
    }

    if (m_widget) {
        m_widget->unreference () ;
        m_widget = NULL ;
        m_vte = NULL ;
    }
}

bool
Terminal::init_pty ()
{
    if (openpty (&m_master_pty, &m_slave_pty, NULL, NULL, NULL)) {
        LOG_ERROR ("oops") ;
        return false ;
    }
    THROW_IF_FAIL (m_slave_pty) ;
    THROW_IF_FAIL (m_master_pty) ;

    if (grantpt (m_master_pty)) {
        LOG_ERROR ("oops") ;
        return false ;
    }

    if (unlockpt (m_master_pty)) {
        LOG_ERROR ("oops") ;
        return false ;
    }

    vte_terminal_set_pty (m_vte, m_master_pty) ;
    return true ;
}

Gtk::Widget&
Terminal::widget () const
{
    THROW_IF_FAIL (m_widget && m_vte) ;
    return *m_widget ;
}

UString
Terminal::slave_pts_name () const
{
    UString result ;

    if (!m_master_pty) {
        LOG_ERROR ("oops") ;
        return result;
    }

    result = ptsname (m_master_pty) ;
    return result ;
}

NEMIVER_END_NAMESPACE //nemiver

struct Option {
    bool display_help ;
    int gtk_socket_id ;

    Option () :
        display_help (false),
        gtk_socket_id (0)
    {}
};//end struct Option


int
main (int a_argc, char *a_argv[])
{
    Gtk::Main loop (a_argc, a_argv) ;

    nemiver::common::Initializer::do_init () ;

    Gtk::Window window ;
    nemiver::Terminal terminal ;

    window.add (terminal.widget ()) ;

    window.show_all () ;

    std::cout << "pts name: '"<< terminal.slave_pts_name () << "'" << std::endl ;

    loop.run (window) ;
    return 0 ;
}

