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
#include "common/nmv-exception.h"
#include "nmv-dbg-common.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)

struct OutputHandlerList::Priv {
    list<OutputHandlerSafePtr> output_handlers;
};//end OutputHandlerList

OutputHandlerList::OutputHandlerList ()
{
    m_priv.reset (new OutputHandlerList::Priv);
}

OutputHandlerList::~OutputHandlerList ()
{
}

void
OutputHandlerList::add (const OutputHandlerSafePtr &a_handler)
{
    THROW_IF_FAIL (m_priv);
    m_priv->output_handlers.push_back (a_handler);
}

void
OutputHandlerList::submit_command_and_output (CommandAndOutput &a_cao)
{
    list<OutputHandlerSafePtr>::iterator iter;
    for (iter = m_priv->output_handlers.begin ();
            iter != m_priv->output_handlers.end ();
            ++iter)
    {
        if ((*iter)->can_handle (a_cao)) {
            NEMIVER_TRY;
            (*iter)->do_handle (a_cao);
            NEMIVER_CATCH_NOX;
        }
    }
}

NEMIVER_END_NAMESPACE (nemiver)

