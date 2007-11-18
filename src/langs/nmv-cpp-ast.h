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
#ifndef __NEMIVER_CPP_AST_H__
#define __NEMIVER_CPP_AST_H__

#include <string>
#include <list>
#include <tr1/memory>
#include "common/nmv-namespace.h"
#include "common/nmv-api-macros.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

using std::tr1::shared_ptr;
using std::list;
using std::string;

class NEMIVER_API Token {
public:
    enum Kind {
        UNDEFINED=0,
        IDENTIFIER,
        KEYWORD,
        //LITERAL
        INTEGER_LITERAL,
        CHARACTER_LITERAL,
        FLOATING_LITERAL,
        STRING_LITERAL,
        BOOLEAN_LITERAL,
        //OPERATOR
        OPERATOR_NEW /*new*/,
        OPERATOR_DELETE /*delete*/,
        OPERATOR_NEW_VECT /*new[]*/,
        OPERATOR_DELETE_VECT /*delete[]*/,
        OPERATOR_PLUS /*+*/,
        OPERATOR_MINUS /*-*/,
        OPERATOR_MULT /* * */,
        OPERATOR_DIV /* / */,
        OPERATOR_MOD /*%*/,
        OPERATOR_BIT_XOR /*^*/,
        OPERATOR_BIT_AND /*&*/,
        OPERATOR_BIT_OR /*|*/,
        OPERATOR_BIT_COMPLEMENT /*~*/,
        OPERATOR_NOT /*!*/,
        OPERATOR_ASSIGN /*=*/,
        OPERATOR_LT /*<*/,
        OPERATOR_GT /*<*/,
        OPERATOR_PLUS_EQ /*+=*/,
        OPERATOR_MINUS_EQ /*-=*/,
        OPERATOR_MULT_EQ /**=*/,
        OPERATOR_DIV_EQ /*/=*/,
        OPERATOR_MOD_EQ /*%=*/,
        OPERATOR_BIT_XOR_EQ /*^=*/,
        OPERATOR_BIT_AND_EQ /*&=*/,
        OPERATOR_BIT_OR_EQ /*|=*/,
        OPERATOR_BIT_LEFT_SHIFT /*<<*/,
        OPERATOR_BIT_RIGHT_SHIFT /*<<*/,
        OPERATOR_BIT_LEFT_SHIFT_EQ /*<<=*/,
        OPERATOR_BIT_RIGHT_SHIFT_EQ /*>>=*/,
        OPERATOR_EQUALS /*==*/,
        OPERATOR_NOT_EQUAL /*!=*/,
        OPERATOR_LT_EQ /*=*/,
        OPERATOR_GT_EQ /*=*/,
        OPERATOR_AND /*&&*/,
        OPERATOR_OR /*||*/,
        OPERATOR_PLUS_PLUS /*++*/,
        OPERATOR_MINUS_MINUS /*--*/,
        OPERATOR_SEQ_EVAL /*,*/,
        OPERATOR_MEMBER_POINTER /*->**/,
        OPERATOR_DEREF /*->*/,
        OPERATOR_GROUP /*()*/,
        OPERATOR_ARRAY_ACCESS /*[]*/,
        OPERATOR_SCOPE_RESOL /*::*/,
        OPERATOR_DOT /*.*/,
        OPERATOR_DOT_STAR /* .* */,
        //punctuators
        PUNCTUATOR_COLON /*:*/,
        PUNCTUATOR_SEMI_COLON /*;*/,
        PUNCTUATOR_CURLY_BRACKET_OPEN /*{*/,
        PUNCTUATOR_CURLY_BRACKET_CLOSE /*}*/,
        PUNCTUATOR_BRACKET_OPEN /*[*/,
        PUNCTUATOR_BRACKET_CLOSE /*]*/,
        PUNCTUATOR_PARENTHESIS_OPEN /*(*/,
        PUNCTUATOR_PARENTHESIS_CLOSE /*(*/,
        PUNCTUATOR_QUESTION_MARK /*?*/
    };

private:
    Kind m_kind ;
    string m_str_value ;
    string m_str_value2 ;
    int m_int_value ;

public:
    Token () ;
    Token (Kind a_kind, const string& a_value) ;
    Token (Kind a_kind,
           const string& a_value,
           const string& a_value2) ;
    Token (Kind a_kind, int a_value) ;
    Token (const Token &) ;
    Token& operator= (const Token&) ;
    ~Token () ;
    const string& get_str_value () const;
    const string& get_str_value2 () const;
    int get_int_value () const;
    Kind get_kind () const;
    void set (Kind a_kind);
    void set (Kind a_kind, const string &a_val);
    void set (Kind a_kind,
              const string &a_val,
              const string &a_val2);
    void set (Kind a_kind, int a_val);
    void clear ();
    bool is_literal () const;
    bool is_operator () const;
};//end class Token

/// \brief the base class of declarations
class NEMIVER_API Decl {
    Decl ();
    Decl (const Decl&);
    Decl& operator= (const Decl&);

public:
    enum Kind {
        UNDEFINED=0,
        SIMPLE_DECLARATION
    };

private:
    Kind m_kind;

protected:
    void set_kind (Kind a_kind);

public:
    Kind get_kind () const;

    Decl (Kind a_kind);
};//end class Declaration

class DeclSpecifier;
class InitDeclarator;
typedef shared_ptr<DeclSpecifier> DeclSpecifierPtr;
typedef shared_ptr<InitDeclarator> InitDeclaratorPtr;

/// \brief a simple-declaration
class NEMIVER_API SimpleDecl : public Decl {
    SimpleDecl (const SimpleDecl&);
    SimpleDecl& operator= (SimpleDecl&);

    list<DeclSpecifierPtr> m_decl_specs;
    list<InitDeclaratorPtr> m_init_decls;
public:

    SimpleDecl ();
    ~SimpleDecl ();
    void add_decl_specifier (DeclSpecifierPtr a_spec);
    const list<DeclSpecifierPtr>& get_decl_specifiers () const;

    void add_init_declarator (InitDeclaratorPtr a_decl);
    const list<InitDeclaratorPtr>& get_init_declarators () const;
};//end SimpleDecl

class NEMIVER_API TypeSpecifier {
    TypeSpecifier (const TypeSpecifier&);
    TypeSpecifier& operator= (const TypeSpecifier&);

public:
    enum Kind {
        UNDEFINED,
        SIMPLE,
        CLASS,
        ENUM,
        ELABORATED,
        CONST,
        VOLATILE
    };

private:
    Kind m_kind;
    string m_simple;
    string m_elaborated;

public:

    TypeSpecifier ():
        m_kind (UNDEFINED)
    {
    }
    ~TypeSpecifier () {}

    Kind get_kind () const {return m_kind;}
    void set_kind (Kind a_kind) {m_kind = a_kind;}
    void set_elaborated (const string &e) {m_kind = ELABORATED; m_elaborated = e;}
    const string& get_elaborated () const {return m_elaborated;}
    void set_simple (const string &s) {m_kind = SIMPLE; m_simple = s;}
    const string& get_simple () const {return m_simple;}
    bool is_cv_qualifier () const {return (m_kind == CONST || m_kind == VOLATILE);}
    bool to_string (string &a_str) const;
};

class NEMIVER_API DeclSpecifier {
    DeclSpecifier (const DeclSpecifier&);
    DeclSpecifier& operator= (const DeclSpecifier&);

public:
    enum Kind {
        UNDEFINED,
        //storage-class-speficier
        AUTO,
        REGISTER,
        STATIC,
        EXTERN,
        MUTABLE,
        TYPE,
        FUNCTION,
        FRIEND,
        TYPEDEF
    };

private:
    Kind m_kind ;
    shared_ptr<TypeSpecifier> m_type_specifier;

public:
    DeclSpecifier ():
        m_kind (UNDEFINED)
    {
    }

    ~DeclSpecifier ()
    {
    }

    Kind get_kind () const {return m_kind;}
    void set_kind (Kind a_kind) {m_kind=a_kind;}
    void set_type_specifier (const shared_ptr<TypeSpecifier> a_type_spec)
    {
        m_kind = TYPE;
        m_type_specifier = a_type_spec;
    }
    shared_ptr<TypeSpecifier> get_type_specifier () const {return m_type_specifier;}

    bool is_storage_class ()
    {
        return (m_kind == AUTO || m_kind == REGISTER
                || m_kind == STATIC || m_kind == EXTERN
                || m_kind == MUTABLE || m_kind == TYPE
                || m_kind == FUNCTION || m_kind == FRIEND
                || m_kind == TYPEDEF);
    }
    bool to_string (string &a_str) const ;
    static bool list_to_string (list<shared_ptr<DeclSpecifier> > &a_decls,
                                string &a_str);
};//end class DeclSpecifier

class NEMIVER_API Expr {
    Expr (const Expr&);
    Expr& operator= (const Expr&);
    Expr ();

public:
    enum Kind {
        UNDEFINED=0,
        ID_EXPRESSION,
        PRIMARY_EXPRESSION,
        CONDITIONAL_EXPRESSION,
        ASSIGNMENT_EXPRESION,
        THROW_EXPRESSION
    };

private:
    Kind m_kind;

public:
    Expr (Kind a_kind) :
        m_kind (a_kind)
    {
    }
    ~Expr () {}
    Kind get_kind () {return m_kind;}
};//end class Expr

class UnqualifiedIDExpr;
class QualifiedIDExpr;

class NEMIVER_API IDExpr : public Expr {
    IDExpr (const IDExpr&);
    IDExpr& operator= (const IDExpr&);
    IDExpr ();

public:
    enum Kind {
        UNDEFINED,
        QUALIFIED,
        UNQUALIFIED
    };

private:
    Kind m_kind;

public:
    IDExpr (Kind kind) :
        Expr (ID_EXPRESSION),
        m_kind (kind)

    {
    }

    virtual ~IDExpr ();

    virtual bool to_string (string &) const=0;

    Kind get_kind () const {return m_kind;}
};//end class Expr

class NEMIVER_API UnqualifiedIDExpr : public IDExpr {
    UnqualifiedIDExpr (const UnqualifiedIDExpr&);
    UnqualifiedIDExpr& operator= (const UnqualifiedIDExpr&);

public:
    enum Kind {
        UNDEFINED,
        IDENTIFIER,
        OP_FUNC_ID,
        CONV_FUNC_ID, //TODO:not supported yet
        DESTRUCTOR_ID,
        TEMPLATE_ID //TODO: not supported yet
    };

private:
    Kind m_kind;
    Token m_token;

public:
    UnqualifiedIDExpr () :
        IDExpr (UNQUALIFIED),
        m_kind (UNDEFINED)
    {
    }

    UnqualifiedIDExpr (Kind kind) :
        IDExpr (UNQUALIFIED),
        m_kind (kind)

    {
    }
    Kind get_kind () const {return m_kind;}
    void set_kind (Kind kind) {m_kind=kind;}
    void set_token (Kind kind, Token token) {m_kind=kind, m_token=token;}
    const Token& get_token () const {return m_token;}
    ~UnqualifiedIDExpr () {}
    bool to_string (string &) const;
};//end class UnqualifiedIDExpr

class NEMIVER_API QualifiedIDExpr : public IDExpr {
    QualifiedIDExpr (const QualifiedIDExpr &);
    QualifiedIDExpr& operator= (const QualifiedIDExpr &);

public:
    enum Kind {
        UNDEFINED,
        IDENTIFIER_ID,
        OP_FUNC_ID,
        NESTED_ID,//TODO: support this
        TEMPLATE_ID//TODO: support this
    };

private:
    Kind m_kind;
    Token m_token;
    string m_nested_name_specifier;
    shared_ptr<UnqualifiedIDExpr> m_template;

public:

    QualifiedIDExpr () :
        IDExpr (QUALIFIED),
        m_kind (UNDEFINED)
    {}
    QualifiedIDExpr (Kind kind) :
        IDExpr (QUALIFIED),
        m_kind (kind)
    {}

    void set_kind (Kind a_kind) {m_kind=a_kind;}
    Kind get_kind () const {return m_kind;}
    void set_token (Kind kind, const Token &token) {m_kind=kind, m_token=token;}
    const Token& get_token () const {return m_token;}
    void set_nested_name_specifier (const string &s) {m_nested_name_specifier=s;}
    const string& get_nested_name_specifier () {return m_nested_name_specifier;}
    void set_template (shared_ptr<UnqualifiedIDExpr> t) {m_template=t;}
    shared_ptr<UnqualifiedIDExpr> get_template () {return m_template;}
    bool to_string (string &) const ;
    //TODO: support template-id
};//end QualifiedIDExpr

class NEMIVER_API PrimaryExpr : public Expr {
    PrimaryExpr (const PrimaryExpr&);
    PrimaryExpr& operator= (const PrimaryExpr&);

public:
    enum Kind {
        UNDEFINED,
        LITERAL,
        THIS,
        PARENTHESIZED,
        ID_EXPR,
    };

private:
    Kind m_kind;
    Token m_token;
    shared_ptr<IDExpr> m_id_expr;
    shared_ptr<Expr> m_parenthesized;

public:
    PrimaryExpr () : Expr (PRIMARY_EXPRESSION), m_kind (UNDEFINED) {}
    ~PrimaryExpr () {}
    Kind get_kind () const {return m_kind;}
    void set_kind (Kind kind) {m_kind=kind;}
    void set_token (Kind kind, const Token &token) {m_kind=kind, m_token=token;}
    const Token& get_token () const {return m_token;}
    void set_id_expr (shared_ptr<IDExpr> id_expr) {m_kind=ID_EXPR, m_id_expr=id_expr;}
    const shared_ptr<IDExpr> get_id_expr () const {return m_id_expr;}
    void set_parenthesized (shared_ptr<Expr> expr)
    {
        m_kind=PARENTHESIZED, m_parenthesized=expr;
    }
    const shared_ptr<Expr> get_parenthesized () const {return m_parenthesized;}
};//end class PrimaryExpr

NEMIVER_END_NAMESPACE (cpp)
NEMIVER_END_NAMESPACE (nemiver)
#endif /*__NEMIVER_CPP_AST_H__*/

