//Author: Jonathon Jongsma
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

#include "nmv-hex-document.h"
#include "common/nmv-exception.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (Hex)
struct HexDocRef {
    void operator () (::HexDocument *o)
    {
        if (o && G_IS_OBJECT (o)) {
            g_object_ref (G_OBJECT (o)) ;
        } else {
            LOG_ERROR ("bad HexDocument") ;
        }
    }
};

struct HexDocUnref {
    void operator () (::HexDocument *o)
    {
        if (o && G_IS_OBJECT (o)) {
            g_object_unref (G_OBJECT (o)) ;
        } else {
            LOG_ERROR ("bad HexDocument") ;
        }
    }

};

struct Document::Priv {
    SafePtr< ::HexDocument, HexDocRef, HexDocUnref> document ;

    Priv (const std::string& filename) :
        document (HEX_DOCUMENT (hex_document_new_from_file (filename.c_str ())), true)
    {}

    Priv () :
        document (HEX_DOCUMENT (hex_document_new ()), true)
    {}

    ~Priv ()
    {}
};//end struct Document::Priv

Document::~Document ()
{
}

Document::Document ()
{
    m_priv.reset (new Priv ()) ;
}

Document::Document (const std::string& filename)
{
    m_priv.reset (new Priv (filename)) ;
}

::HexDocument* Document::cobj()
{
    THROW_IF_FAIL (m_priv && m_priv->document);
    return m_priv->document.get ();
}

void
Document::set_data(guint offset,
            guint len, guint rep_len, guchar *data,
            bool undoable)
{
    THROW_IF_FAIL (m_priv && m_priv->document);
    hex_document_set_data(m_priv->document.get (), offset, len, rep_len, data, undoable);
}

void
Document::delete_data(guint offset, guint len, bool undoable)
{

    THROW_IF_FAIL (m_priv && m_priv->document);
    hex_document_delete_data(m_priv->document.get (), offset, len, undoable);
}

void
Document::clear (bool undoable)
{
    THROW_IF_FAIL (m_priv && m_priv->document);
    LOG_DD ("file size = " << (int) m_priv->document->file_size);
    delete_data(0, m_priv->document->file_size, undoable);
}


DocumentSafePtr
Document::create ()
{
    DocumentSafePtr result (new Document ()) ;
    THROW_IF_FAIL (result) ;
    return result ;
}

DocumentSafePtr
Document::create (const std::string& filename)
{
    DocumentSafePtr result (new Document (filename)) ;
    THROW_IF_FAIL (result) ;
    return result ;
}

NEMIVER_END_NAMESPACE (Hex)
NEMIVER_END_NAMESPACE (nemiver)
