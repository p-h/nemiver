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
#include <gtkmm/container.h>
#include "common/nmv-exception.h"
#include "nmv-hex-editor.h"
#include <gtkhex.h>

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (Hex)

struct GtkHexRef {
    void operator () (GtkHex *o)
    {
        if (o && G_IS_OBJECT (o)) {
            g_object_ref (G_OBJECT (o));
        } else {
            LOG_ERROR ("bad GtkHex");
        }
    }
};

struct GtkHexUnref {
    void operator () (GtkHex *o)
    {
        if (o && G_IS_OBJECT (o)) {
            g_object_unref (G_OBJECT (o));
        } else {
            LOG_ERROR ("bad GtkHex");
        }
    }

};

struct Editor::Priv {
    SafePtr<GtkHex, GtkHexRef, GtkHexUnref> hex;
    Gtk::Container *widget;

    Priv (const DocumentSafePtr& a_doc) :
        hex (GTK_HEX (gtk_hex_new (a_doc->cobj())), true),
        widget (0)
    {
        THROW_IF_FAIL (GTK_IS_WIDGET (hex.get ()));
        widget = Glib::wrap (GTK_CONTAINER (hex.get ()));
        THROW_IF_FAIL (widget);
    }

    ~Priv ()
    {
        widget = 0;
    }
};//end struct Editor::Priv

Editor::~Editor ()
{
}

Editor::Editor (const DocumentSafePtr &a_document)
{
    m_priv.reset (new Priv (a_document));
}

EditorSafePtr
Editor::create (const DocumentSafePtr &a_document)
{
    EditorSafePtr result (new Editor (a_document));
    THROW_IF_FAIL (result);
    return result;
}


void
Editor::set_group_type (guint a_group_type)
{
    THROW_IF_FAIL (m_priv && m_priv->hex);
    gtk_hex_set_group_type (m_priv->hex.get (), a_group_type);
}

void
Editor::set_starting_offset (int a_starting_offset)
{
    THROW_IF_FAIL (m_priv && m_priv->hex);
    gtk_hex_set_starting_offset (m_priv->hex.get (), a_starting_offset);
}

void
Editor::show_offsets (bool show)
{
    THROW_IF_FAIL (m_priv && m_priv->hex);
    gtk_hex_show_offsets (m_priv->hex.get (), show);
}

void
Editor::set_font (const Pango::FontDescription& a_desc)
{
    THROW_IF_FAIL (m_priv && m_priv->hex);
    Glib::RefPtr<Pango::Context> context =
                                Glib::wrap (gdk_pango_context_get ());
    Glib::RefPtr<Pango::Font> new_font = context->load_font (a_desc);
    if (new_font) {
        Pango::FontMetrics new_metrics = new_font->get_metrics ();
        gtk_hex_set_font (m_priv->hex.get (),
                          const_cast<PangoFontMetrics*>(new_metrics.gobj ()),
                          const_cast<PangoFontDescription*>(a_desc.gobj ()));
    }
}


void
Editor::set_geometry (int cpl, int vis_lines)
{
    THROW_IF_FAIL (m_priv && m_priv->hex);
    gtk_hex_set_geometry (m_priv->hex.get (), cpl, vis_lines);
}

void
Editor::get_geometry (int& cpl, int& vis_lines) const
{
    THROW_IF_FAIL (m_priv && m_priv->hex);
    cpl = m_priv->hex.get ()->cpl;
    vis_lines = m_priv->hex.get ()->vis_lines;
}


Gtk::Container&
Editor::get_widget () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->widget);
    return *m_priv->widget;
}

NEMIVER_END_NAMESPACE (Hex)
NEMIVER_END_NAMESPACE (nemiver)
