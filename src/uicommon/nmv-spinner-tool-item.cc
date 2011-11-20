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
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#include "config.h"
#include "nmv-spinner-tool-item.h"
#include <gtkmm/spinner.h>

NEMIVER_BEGIN_NAMESPACE (nemiver)

SpinnerToolItem::SpinnerToolItem ()
{
    m_spinner.reset (new Gtk::Spinner);

    // Don't recursively show the spinner with show_all ()
    m_spinner->set_no_show_all ();

    add (*m_spinner);
}

SpinnerToolItem::~SpinnerToolItem ()
{
}

void
SpinnerToolItem::start ()
{
    m_spinner->start ();
    m_spinner->show ();
}

void
SpinnerToolItem::stop ()
{
    m_spinner->stop ();
    m_spinner->hide ();
}

void
SpinnerToolItem::on_toolbar_reconfigured ()
{
    int spinner_width;
    int spinner_height;
    Gtk::IconSize::lookup (get_icon_size (), spinner_width, spinner_height);
    m_spinner->set_size_request (spinner_width, spinner_height);

    // Call base class
    Gtk::ToolItem::on_toolbar_reconfigured ();
}

NEMIVER_END_NAMESPACE (nemiver)
