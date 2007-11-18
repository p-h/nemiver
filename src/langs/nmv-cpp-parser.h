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
#ifndef __NMV_CPP_PARSER_H__
#define __NMV_CPP_PARSER_H__

#include <string>
#include <tr1/memory>
#include "nmv-cpp-ast.h"
#include "common/nmv-namespace.h"
#include "common/nmv-api-macros.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

using std::tr1::shared_ptr;
using std::string;

class NEMIVER_API Parser {
    Parser (const Parser&);
    Parser& operator= (const Parser&);
    Parser ();
    struct Priv;
    shared_ptr<Priv> m_priv;


public:
    Parser (const string&);
    ~Parser ();
    bool parse_primary_expr (shared_ptr<PrimaryExpr> &expr);
    bool parse_class_or_namespace_name (string &str);
    bool parse_type_name (string &);
    bool parse_nested_name_specifier (string &str);
    bool parse_unqualified_id (shared_ptr<UnqualifiedIDExpr> &a_expr);
    bool parse_qualified_id (shared_ptr<QualifiedIDExpr> &a_expr);
    bool parse_id_expr (shared_ptr<IDExpr> &a_expr);
    bool parse_elaborated_type_specifier (string &);
    bool parse_simple_type_specifier (string &);
    bool parse_type_specifier (shared_ptr<TypeSpecifier> &);
    bool parse_type_id (string &);
    bool parse_decl_specifier (shared_ptr<DeclSpecifier> &);
    bool parse_decl_specifier_seq (list<shared_ptr<DeclSpecifier> > &);
    bool parse_declarator_id (string &);
    bool parse_direct_declarator (string &);
    bool parse_cv_qualifier (string &);
    bool parse_cv_qualifier_seq (string &);
    bool parse_ptr_operator (string &);
    bool parse_declarator (string &);
    bool parse_init_declarator (string &);
    bool parse_init_declarator_list (string &);
    bool parse_simple_declaration (string &);
};//end class Parser
NEMIVER_END_NAMESPACE (nemiver)
NEMIVER_END_NAMESPACE (cpp)
#endif //__NMV_CPP_PARSER_H__

