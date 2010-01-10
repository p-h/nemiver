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

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

Address::Address ()
{
}

Address::Address (const std::string &a) :
    m_addr (a)
{
}

Address::operator const std::string& () const
{
    return m_addr;
}

int
Address::size () const
{
    if (m_addr.empty ())
        return 0;
    int suffix_len = 0;
    if (m_addr[0] == '0' && m_addr[1] == 'x')
        suffix_len = 2;
    return m_addr.size () - suffix_len;
}

Address&
Address::operator= (const std::string &a_addr)
{
    m_addr = a_addr;
    return *this;
}

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)
