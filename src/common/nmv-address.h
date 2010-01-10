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
#ifndef __NEMIVER_ADDRESS_H__
#define __NEMIVER_ADDRESS_H__
#include <string>
#include "nmv-namespace.h"
#include "nmv-api-macros.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

class NEMIVER_API Address
{
    std::string m_addr;
public:
    Address ();
    Address (const std::string &a_addr);
    Address (const Address &);
    operator const std::string& () const;
    int size () const;
    Address& operator= (const std::string &);
};// end class Address

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)

#endif // __NEMIVER_ADDRESS_H__
