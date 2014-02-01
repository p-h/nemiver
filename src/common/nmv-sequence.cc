/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4;-*- */

//Authors: Dodji Seketeli
//Copyright 2006 Dodji Seketeli
/*
 *This file is part of the nemiver Project.
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
#include "nmv-sequence.h"

namespace nemiver {
namespace common {

struct Sequence::Priv {
    Glib::Mutex integer_seq_mutex;
    long long integer_seq;

    Priv () :
        integer_seq (0)
    {}
};//end struct Sequence::Priv

Sequence::Sequence () :
    m_priv (new Sequence::Priv ())
{
}

Sequence::~Sequence ()
{
    LOG_D ("delete", "destructor-domain");
}

long long
Sequence::create_next_integer ()
{
    LOG_FUNCTION_SCOPE_NORMAL_DD;
    Glib::Mutex::Lock (m_priv->integer_seq_mutex);
    long long new_val = ++m_priv->integer_seq;
    if (new_val < m_priv->integer_seq) {
        THROW_EXCEPTION (Sequence::OverflowException,
                         "Integer sequence overflow");
    }
    m_priv->integer_seq = new_val;
    return m_priv->integer_seq;
}

long long
Sequence::get_current_integer () const
{
    return m_priv->integer_seq;
}

}//end namespace common
}//end namespace nemiver

