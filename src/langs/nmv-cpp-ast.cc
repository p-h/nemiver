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
#include "nmv-cpp-ast.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

//**************
// class <Token>
//**************

Token::Token () :
    m_kind (UNDEFINED),
    m_int_value (-1)
{
}

Token::Token (Kind a_kind, const string& a_value) :
    m_kind (a_kind), m_str_value (a_value), m_int_value (-1)
{
}

Token::Token (Kind a_kind,
              const string& a_value,
              const string& a_value2) :
    m_kind (a_kind),
    m_str_value (a_value),
    m_str_value2 (a_value2),
    m_int_value (-1)
{
}

Token::Token (Kind a_kind, int a_value) :
    m_kind (a_kind), m_int_value (a_value)
{
}

Token::Token (const Token &a_t)
{
    m_kind = a_t.get_kind () ;
    m_str_value = a_t.get_str_value () ;
    m_int_value = a_t.get_int_value () ;
}

Token::Token&
Token::operator= (const Token &a_t)
{
    m_kind = a_t.get_kind () ;
    m_str_value = a_t.get_str_value () ;
    m_str_value2 = a_t.get_str_value2 () ;
    m_int_value = a_t.get_int_value () ;
    return *this ;
}

Token::~Token ()
{
}

const string&
Token::get_str_value () const
{
    return m_str_value ;
}

const string&
Token::get_str_value2 () const
{
    return m_str_value2 ;
}

int
Token::get_int_value () const
{
    return m_int_value ;
}

Token::Kind
Token::get_kind () const
{
    return m_kind ;
}

void
Token::set (Kind a_kind)
{
    m_kind = a_kind ;
}

void
Token::set (Kind a_kind, const string &a_val)
{
    m_kind = a_kind ;
    m_str_value = a_val ;
}

void
Token::set (Kind a_kind, const string &a_val, const string &a_val2)
{
    m_kind = a_kind ;
    m_str_value = a_val ;
    m_str_value2 = a_val2 ;
}

void
Token::set (Kind a_kind, int a_val)
{
    m_kind = a_kind ;
    m_int_value = a_val ;
}

void
Token::clear ()
{
    m_kind = Token::UNDEFINED;
    m_str_value.clear ();
    m_str_value2.clear ();
    m_int_value=0;
}

bool
Token::is_literal () const
{
    switch (get_kind ()) {
        case INTEGER_LITERAL:
        case CHARACTER_LITERAL:
        case FLOATING_LITERAL:
        case STRING_LITERAL:
        case BOOLEAN_LITERAL:
            return true;
        default:
            break;
    }
    return false;
}

bool
Token::is_operator () const
{
    switch (get_kind ()) {
        case OPERATOR_NEW:
        case OPERATOR_DELETE:
        case OPERATOR_NEW_VECT:
        case OPERATOR_DELETE_VECT:
        case OPERATOR_PLUS:
        case OPERATOR_MINUS:
        case OPERATOR_MULT:
        case OPERATOR_DIV:
        case OPERATOR_MOD:
        case OPERATOR_BIT_XOR:
        case OPERATOR_BIT_AND:
        case OPERATOR_BIT_OR:
        case OPERATOR_BIT_COMPLEMENT:
        case OPERATOR_NOT:
        case OPERATOR_ASSIGN:
        case OPERATOR_LT:
        case OPERATOR_GT:
        case OPERATOR_PLUS_EQ:
        case OPERATOR_MINUS_EQ:
        case OPERATOR_MULT_EQ:
        case OPERATOR_DIV_EQ:
        case OPERATOR_MOD_EQ:
        case OPERATOR_BIT_XOR_EQ:
        case OPERATOR_BIT_AND_EQ:
        case OPERATOR_BIT_OR_EQ:
        case OPERATOR_BIT_LEFT_SHIFT:
        case OPERATOR_BIT_RIGHT_SHIFT:
        case OPERATOR_BIT_LEFT_SHIFT_EQ:
        case OPERATOR_BIT_RIGHT_SHIFT_EQ:
        case OPERATOR_EQUALS:
        case OPERATOR_NOT_EQUAL:
        case OPERATOR_LT_EQ:
        case OPERATOR_GT_EQ:
        case OPERATOR_AND:
        case OPERATOR_OR:
        case OPERATOR_PLUS_PLUS:
        case OPERATOR_MINUS_MINUS:
        case OPERATOR_SEQ_EVAL:
        case OPERATOR_MEMBER_POINTER:
        case OPERATOR_DEREF:
        case OPERATOR_GROUP:
        case OPERATOR_ARRAY_ACCESS:
        case OPERATOR_SCOPE_RESOL:
        case OPERATOR_DOT:
        case OPERATOR_DOT_STAR:
            return true;
        default:
            break;
    }
    return false;
}

//**************
// class </Token>
//**************

Decl::Decl (Decl::Kind a_kind) :
    m_kind (a_kind)
{
}

Decl::Kind
Decl::get_kind () const
{
    return m_kind;
}

void
Decl::set_kind (Decl::Kind a_kind)
{
    m_kind = a_kind;
}

SimpleDecl::SimpleDecl () :
    Decl (SIMPLE_DECLARATION)
{
}

SimpleDecl::~SimpleDecl ()
{
}

void
SimpleDecl::add_decl_specifier (DeclSpecifierPtr a_spec)
{
    m_decl_specs.push_back (a_spec);
}

const list<DeclSpecifierPtr>&
SimpleDecl::get_decl_specifiers () const
{
    return m_decl_specs;
}

void
SimpleDecl::add_init_declarator (InitDeclaratorPtr a_decl)
{
    m_init_decls.push_back (a_decl);
}

const list<InitDeclaratorPtr>&
SimpleDecl::get_init_declarators () const
{
    return m_init_decls;
}


bool
TypeSpecifier::to_string (string &a_str) const
{
    switch (get_kind ()) {
        case UNDEFINED:
            break;
        case SIMPLE:
            a_str = get_simple ();
            break;
        case CLASS:
            a_str = "class {...};";//TODO: handle this.
            break;
        case ENUM:
            a_str = "enum {...};";//TODO: handle this.
            break;
        case ELABORATED:
            a_str = get_elaborated ();
            break;
        case CONST:
            a_str = "const";
            break;
        case VOLATILE:
            a_str = "volatile";
            break;
        default:
            a_str = "<unknown>";
            return false;
            break;
    }
    return true;
}

bool
DeclSpecifier::to_string (string &a_str) const
{
    switch (get_kind ()) {
        case DeclSpecifier::AUTO:
            a_str = "auto";
            break;
        case DeclSpecifier::REGISTER:
            a_str = "register";
        case DeclSpecifier::STATIC:
            a_str = "static";
            break;
        case DeclSpecifier::EXTERN:
            a_str = "extern";
            break;
        case DeclSpecifier::MUTABLE:
            a_str = "mutable";
            break;
        case DeclSpecifier::TYPE:
            if (!get_type_specifier ()) {return false;}
            get_type_specifier ()->to_string (a_str);
            break;
        case DeclSpecifier::FUNCTION:
            a_str = "<TODO: handle this>";
            break;
        case DeclSpecifier::FRIEND:
            a_str = "friend";
            break;
        case DeclSpecifier::TYPEDEF:
            a_str = "typedef";
            break;
        default:
            return false;
            break;
    }
    return true;
}

bool
DeclSpecifier::list_to_string (list<shared_ptr<DeclSpecifier> > &a_decls,
                               string &a_str)
{
    list<shared_ptr<DeclSpecifier> >::const_iterator it;

    string str;
    for (it = a_decls.begin (); it != a_decls.end (); ++it) {
        (*it)->to_string (str);
        a_str += str + " ";
    }
    return true;
}

IDExpr::~IDExpr ()
{
}

bool
UnqualifiedIDExpr::to_string (string &a_result) const
{
    switch (get_kind () ) {
        case UnqualifiedIDExpr::IDENTIFIER:
            a_result = get_token ().get_str_value ();
            break;
        case UnqualifiedIDExpr::OP_FUNC_ID:
            a_result = get_token ().get_str_value ();
            break;
        case UnqualifiedIDExpr::CONV_FUNC_ID:
            return false;
            break;
        case UnqualifiedIDExpr::DESTRUCTOR_ID:
            {
                string str (get_token ().get_str_value ());
                a_result = "~" + str;
            }
            break;
        case UnqualifiedIDExpr::TEMPLATE_ID:
            return false;
        default:
            return false;
    }
    return true;
}

bool
QualifiedIDExpr::to_string (string &a_result) const
{
    switch (get_kind ()) {
        case QualifiedIDExpr::IDENTIFIER_ID:
            a_result = "::" + get_token ().get_str_value ();
            break;
        case QualifiedIDExpr::OP_FUNC_ID:
            a_result = "::" + get_token ().get_str_value ();
            break;
        case QualifiedIDExpr::NESTED_ID:
            a_result = "::<not supported>";
            break;
        case QualifiedIDExpr::TEMPLATE_ID:
            a_result = "::<not supported>";
            break;
        default:
            return false;
            break;
    }
    return true;
}
NEMIVER_END_NAMESPACE (cpp)
NEMIVER_END_NAMESPACE (nemiver)
