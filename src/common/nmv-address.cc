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
#include "nmv-address.h"
#include "nmv-str-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

Address::Address ()
{
}

Address::Address (const std::string &a) :
    m_addr (a)
{
    str_utils::chomp (m_addr);
}

Address::Address (const Address &a_other) :
    m_addr (a_other.m_addr)
{
}

bool
Address::empty () const
{
    return m_addr.empty ();
}

const std::string&
Address::to_string () const
{
    return m_addr;
}

Address::operator size_t () const
{
    if (m_addr.empty ())
        return 0;
    return str_utils::hexa_to_int (m_addr);
}


size_t
Address::size () const
{
    if (m_addr.empty ())
        return 0;
    int suffix_len = 0;
    if (m_addr[0] == '0' && m_addr[1] == 'x')
        suffix_len = 2;
    return m_addr.size () - suffix_len;
}

size_t
Address::string_size () const
{
    return m_addr.size ();
}

Address&
Address::operator= (const std::string &a_addr)
{
    m_addr = a_addr;
    str_utils::chomp (m_addr);
    return *this;
}

const char&
Address::operator[] (size_t a_index) const
{
    return m_addr[a_index];
}

void
Address::clear ()
{
    m_addr.clear ();
}

bool
operator== (const Address &a_address,
            const std::string &a_addr)
{
    return a_address.m_addr == a_addr;
}

bool
operator== (const Address &a_address,
            size_t a_addr)
{
    return (size_t) a_address == a_addr;
}

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)
