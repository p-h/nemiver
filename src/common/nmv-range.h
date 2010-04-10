// Author: Dodji Seketeli
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
#ifndef __NEMIVER_RANGE_H__
#define __NEMIVER_RANGE_H__

#include "nmv-namespace.h"
#include "nmv-api-macros.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (common)

class Range {
    size_t m_min;
    size_t m_max;

public:
    Range (size_t a_min = 0, size_t a_max = 0) :
        m_min (a_min),
        m_max (a_max)
    {
    }

    size_t min () const {return m_min;}
    void min (int a) {m_min = a;}
    size_t max () const {return m_max;}
    void max (int a) {m_max = a;}
    bool contains (size_t a_value) const
    {
        return (a_value >= m_min && a_value <= m_max);
    }
    void extend (size_t a_value)
    {
        if (!contains (a_value)) {
            if (a_value < m_min)
                m_min = a_value;
            else
                m_max = a_value;
        }
    }
};

NEMIVER_END_NAMESPACE (common)
NEMIVER_END_NAMESPACE (nemiver)

#endif // __NMV_RANGE_H__

