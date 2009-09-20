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
    bool parse_primary_expr (PrimaryExprPtr &);
    bool parse_postfix_expr (PostfixExprPtr &);
    bool parse_unary_expr (UnaryExprPtr &);
    bool parse_cast_expr (CastExprPtr &);
    bool parse_pm_expr (PMExprPtr &);
    bool parse_mult_expr (MultExprPtr &);
    bool parse_add_expr (AddExprPtr &);
    bool parse_shift_expr (ShiftExprPtr &);
    bool parse_rel_expr (RelExprPtr &);
    bool parse_eq_expr (EqExprPtr &);
    bool parse_and_expr (AndExprPtr &);
    bool parse_xor_expr (XORExprPtr &);
    bool parse_or_expr (ORExprPtr &);
    bool parse_log_and_expr (LogAndExprPtr &);
    bool parse_log_or_expr (LogOrExprPtr &);
    bool parse_cond_expr (CondExprPtr &);
    bool parse_assign_expr (AssignExprPtr &);
    bool parse_expr (ExprPtr &);
    bool parse_const_expr (ConstExprPtr &);
    bool parse_class_or_namespace_name (UnqualifiedIDExprPtr &a_name);
    bool parse_type_name (UnqualifiedIDExprPtr &);
    bool parse_nested_name_specifier (QNamePtr &a_result);
    bool parse_unqualified_id (UnqualifiedIDExprPtr &a_expr);
    bool parse_qualified_id (QualifiedIDExprPtr &a_expr);
    bool parse_id_expr (IDExprPtr &a_expr);
    bool parse_elaborated_type_specifier (ElaboratedTypeSpecPtr &);
    bool parse_simple_type_specifier (SimpleTypeSpecPtr &);
    bool parse_type_specifier (TypeSpecifierPtr &);
    bool parse_type_specifier_seq (list<TypeSpecifierPtr> &);
    bool parse_template_argument (TemplateArgPtr &);
    bool parse_template_argument_list (list<TemplateArgPtr> &);
    bool parse_template_id (TemplateIDPtr &);
    bool parse_type_id (TypeIDPtr &);
    bool parse_decl_specifier (DeclSpecifierPtr &);
    bool parse_decl_specifier_seq (list<DeclSpecifierPtr> &);
    bool parse_declarator_id (IDDeclaratorPtr &);
    bool parse_array_declarator (ArrayDeclaratorPtr &);
    bool parse_direct_declarator (DeclaratorPtr &);
    bool parse_cv_qualifier (CVQualifierPtr &);
    bool parse_cv_qualifier_seq (list<CVQualifierPtr> &);
    bool parse_ptr_operator (PtrOperatorPtr &);
    bool parse_declarator (DeclaratorPtr &);
    bool parse_init_declarator (InitDeclaratorPtr &);
    bool parse_init_declarator_list (list<InitDeclaratorPtr> &);
    bool parse_simple_declaration (SimpleDeclarationPtr &a_result);
};//end class Parser
typedef shared_ptr<Parser> ParserPtr;
NEMIVER_END_NAMESPACE (nemiver)
NEMIVER_END_NAMESPACE (cpp)
#endif //__NMV_CPP_PARSER_H__

