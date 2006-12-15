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
#include <gtkmm/toolbutton.h>
#include "nmv-spinner-tool-item.h"
#include "ephy-spinner-tool-item.h"
#include "nmv-exception.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
struct ESpinnerRef {
    void operator () (EphySpinnerToolItem *o)
    {
        if (o && G_IS_OBJECT (o)) {
            g_object_ref (G_OBJECT (o)) ;
        } else {
            LOG_ERROR ("bad ephy spinner") ;
        }
    }
};

struct ESpinnerUnref {
    void operator () (EphySpinnerToolItem *o)
    {
        if (o && G_IS_OBJECT (o)) {
            g_object_unref (G_OBJECT (o)) ;
        } else {
            LOG_ERROR ("bad ephy spinner") ;
        }
    }

};

struct SpinnerToolItem::Priv {
    SafePtr<EphySpinnerToolItem, ESpinnerRef, ESpinnerUnref> spinner ;
    bool is_started ;
    Gtk::ToolItem *widget ;

    Priv () :
        spinner (EPHY_SPINNER_TOOL_ITEM (ephy_spinner_tool_item_new ()), true),
        is_started (false),
        widget (0)
    {
        THROW_IF_FAIL (GTK_IS_WIDGET (spinner.get ())) ;
        widget = Glib::wrap (GTK_TOOL_ITEM (spinner.get ())) ;
        THROW_IF_FAIL (widget) ;
    }

    ~Priv ()
    {
        widget = 0 ;
        is_started = false ;
    }
};//end struct SpinnerToolItem::Priv

SpinnerToolItem::~SpinnerToolItem ()
{
}

SpinnerToolItem::SpinnerToolItem ()
{
    m_priv.reset (new Priv) ;
}

SpinnerToolItemSafePtr
SpinnerToolItem::create ()
{
    SpinnerToolItemSafePtr result (new SpinnerToolItem) ;
    THROW_IF_FAIL (result) ;
    return result ;
}

void
SpinnerToolItem::start ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->spinner) ;

    ephy_spinner_tool_item_set_spinning (m_priv->spinner.get (), true) ;
    m_priv->is_started = true ;
}

bool
SpinnerToolItem::is_started () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->spinner) ;

    return m_priv->is_started  ;
}

void
SpinnerToolItem::stop ()
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->spinner) ;

    ephy_spinner_tool_item_set_spinning (m_priv->spinner.get (), false) ;
    m_priv->is_started = false ;
}

void
SpinnerToolItem::toggle_state ()
{
    if (is_started ()) {
        stop () ;
    } else {
        start () ;
    }
}

Gtk::ToolItem&
SpinnerToolItem::get_widget () const
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->widget) ;
    return *m_priv->widget ;
}

NEMIVER_END_NAMESPACE (nemiver)

