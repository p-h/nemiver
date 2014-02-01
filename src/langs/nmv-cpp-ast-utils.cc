//Author: Dodji Seketeli <dodji@gnome.org>
/*
 *This file is part of the Nemiver Project.
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
#include "nmv-cpp-ast-utils.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

static bool
get_declarator_id_as_string (const DeclaratorPtr a_decl, string &a_id)
{
    if (!a_decl)
        return false;

    if (a_decl->get_decl_node ()) {
        return get_declarator_id_as_string (a_decl->get_decl_node (), a_id);
    }
    Declarator::Kind kind =
        a_decl->get_kind ();

    switch (kind) {
        case Declarator::UNDEFINED:
            return false;
        case Declarator::ID_DECLARATOR:
            a_decl->to_string (a_id);
            return true;
        case Declarator::ARRAY_DECLARATOR: {
            ArrayDeclaratorPtr decl;
            decl = std::tr1::static_pointer_cast<ArrayDeclarator> (a_decl);
            if (!decl->get_declarator ()) {
                return false;
            }
            decl->get_declarator ()->to_string (a_id);
            return true;
        }
        case Declarator::FUNCTION_DECLARATOR:
        case Declarator::FUNCTION_POINTER_DECLARATOR:
        default:
            return false;
    }
    return false;
}

bool
get_declarator_id_as_string (const InitDeclaratorPtr a_decl, string &a_id)
{
    if (!a_decl
        || !a_decl->get_declarator ()
        || !a_decl->get_declarator ()->get_decl_node ()) {
        return false;
    }
    DeclaratorPtr decl = a_decl->get_declarator ()->get_decl_node ();
    return get_declarator_id_as_string (decl, a_id);
}

NEMIVER_END_NAMESPACE (cpp)
NEMIVER_END_NAMESPACE (nemiver)
