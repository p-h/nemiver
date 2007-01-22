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
#include "nmv-i-var-list.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_END_NAMESPACE (nemiver)
//the dynmod initial factory.
extern "C" {

bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    //*a_new_instance = new nemiver::GDBEngine () ;
    return (*a_new_instance != 0) ;
}

}

