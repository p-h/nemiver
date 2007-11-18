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

class QName;
typedef shared_ptr<QName> QNamePtr;
/// \brief Qualified Name.
///
/// can contain the result of the parsing of
/// a nested name specifier.
class NEMIVER_API QName {

public:
    class ClassOrNSName {
        string m_name;
        bool m_prefixed_with_template;//true if is prefixed by the 'template' keyword.

    public:
        ClassOrNSName (const string &n, bool p=false) :
            m_name (n),
            m_prefixed_with_template (p)
        {}
        ClassOrNSName () :
            m_prefixed_with_template (false)
        {}
        const string& get_name () const {return m_name;}
        void set_name (const string &n) {m_name = n;}
        bool is_prefixed_with_template () const {return m_prefixed_with_template;}
        void set_is_prefixed_with_template (bool a) {m_prefixed_with_template=a;}
    };//end ClassOrNSName

private:
    list<ClassOrNSName>m_names;

public:
    QName () {}
    QName (const QName &a_other):
        m_names (a_other.m_names)
    {}

    /// an empty name means the QName is: '::'
    const list<ClassOrNSName>& get_names () const {return m_names;}
    void append (const string &a_name, bool a_prefixed_with_template=false)
    {
        ClassOrNSName c (a_name, a_prefixed_with_template);
        m_names.push_back (c);
    }
    void append (const QNamePtr &a_q, bool a_prefixed_with_template=false)
    {
        if (!a_q) {return;}
        list<ClassOrNSName>::const_iterator it=a_q->get_names ().begin ();
        ClassOrNSName n (it->get_name (), a_prefixed_with_template);
        m_names.push_back (n);
        for (; it != a_q->get_names ().end (); ++it) {
            m_names.push_back (*it);
        }
    }
    bool to_string (string &) const;
};//end class QName

class NEMIVER_API CVQualifier {
    CVQualifier ();
    CVQualifier (const CVQualifier&);
    CVQualifier& operator= (const CVQualifier&);

public:
    enum Kind {
        UNDEFINED,
        CONST,
        VOLATILE
    };
private:
    Kind m_kind;

public:
    CVQualifier (Kind k) :
        m_kind (k)
    {}
    virtual ~CVQualifier () {}
    Kind get_kind () const {return m_kind;}
    void set_kind (Kind k) {m_kind=k;}
    virtual bool to_string (string &) const=0;
};
typedef shared_ptr<CVQualifier> CVQualifierPtr;

class NEMIVER_API ConstQualifier : public CVQualifier {
    ConstQualifier (const ConstQualifier&);
    ConstQualifier& operator= (const ConstQualifier&);
public:
    ConstQualifier () :
        CVQualifier (CONST)
    {}
    ~ConstQualifier () {}
    bool to_string (string &a_str) const {a_str="const";return true;}
};
typedef shared_ptr<ConstQualifier> ConstQualifierPtr;

class NEMIVER_API VolatileQualifier : public CVQualifier {
    VolatileQualifier (const VolatileQualifier &);
    VolatileQualifier& operator= (const VolatileQualifier &);
public:
    VolatileQualifier ():
        CVQualifier (VOLATILE)
    {}
    ~VolatileQualifier () {}
    bool to_string (string &a_str) const {a_str="volatile";return true;}
};
typedef shared_ptr<VolatileQualifier> VolatileQualifierPtr;

class NEMIVER_API PtrOperator {
    PtrOperator (const PtrOperator&);
    PtrOperator& operator= (const PtrOperator&);

public:
    class Elem {
        Elem ();
    public:
        enum Kind {
            UNDEFINED,
            STAR,
            AND,
            CONST,
            VOLATILE,
        };
    private:
        Kind m_kind;

    public:
        Elem (Kind a_k) : m_kind (a_k)
        {}
        Kind get_kind () const {return m_kind;}
        void set_kind (Kind a_kind) {m_kind=a_kind;}
        virtual ~Elem () {}
        virtual bool to_string (string &) const=0;
    };
    typedef shared_ptr<Elem> ElemPtr;
    class StarElem : public Elem {
    public:
        StarElem () :
            Elem (STAR)
        {}
        ~StarElem () {}
        bool to_string (string &a_str) const {a_str="*";return true;}
    };//end class StarElem
    class AndElem : public Elem {
    public:
        AndElem () :
           Elem (AND)
        {}
        ~AndElem () {}
        bool to_string (string &a_str) const {a_str="&";return true;}
    };//end class AndElem
    class ConstElem : public Elem {
    public:
        ConstElem ():
            Elem (CONST)
        {}
        bool to_string (string &a_str) const {a_str="const";return true;}
    };//end class ConstElem
    class VolatileElem : public Elem {
    public:
        VolatileElem () :
            Elem (VOLATILE)
        {}
        bool to_string (string &a_str) const {a_str="volatile";return true;}
    };//end class ConstElem

private:
    QNamePtr m_scope;
    list<ElemPtr> m_elems;

public:
    PtrOperator () {}
    ~PtrOperator () {}

    void set_scope (QNamePtr a_scope) {m_scope = a_scope;}
    const QNamePtr get_scope () const {return m_scope;}
    const list<ElemPtr>& get_elems () const {return m_elems;}
    void append (const ElemPtr &a_elem) {m_elems.push_back (a_elem);}
    bool to_string (string &) const;
};//end class PtrOperator
typedef shared_ptr<PtrOperator> PtrOperatorPtr;

/// \brief the base class of declarations
class NEMIVER_API Declaration {
    Declaration ();
    Declaration (const Declaration&);
    Declaration& operator= (const Declaration&);

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

    Declaration (Kind a_kind);
};//end class Declaration

class DeclSpecifier;
class InitDeclarator;
typedef shared_ptr<DeclSpecifier> DeclSpecifierPtr;
typedef shared_ptr<InitDeclarator> InitDeclaratorPtr;
/// \brief a simple-declaration
class NEMIVER_API SimpleDeclaration : public Declaration {
    SimpleDeclaration (const SimpleDeclaration&);
    SimpleDeclaration& operator= (SimpleDeclaration&);

    list<DeclSpecifierPtr> m_decl_specs;
    list<InitDeclaratorPtr> m_init_decls;
public:

    SimpleDeclaration () :
        Declaration (SIMPLE_DECLARATION)
    {}
    SimpleDeclaration (list<DeclSpecifierPtr> &a, list<InitDeclaratorPtr> &b) :
        Declaration (SIMPLE_DECLARATION),
        m_decl_specs (a),
        m_init_decls (b)
    {}
    ~SimpleDeclaration () {}
    void add_decl_specifier (DeclSpecifierPtr a_spec) {m_decl_specs.push_back (a_spec);}
    const list<DeclSpecifierPtr>& get_decl_specifiers () const {return m_decl_specs;}

    void add_init_declarator (InitDeclaratorPtr a_decl) {m_init_decls.push_back (a_decl);}
    const list<InitDeclaratorPtr>& get_init_declarators () const {return m_init_decls;}
    bool to_string (string &) const;
};//end SimpleDeclaration
typedef shared_ptr<SimpleDeclaration> SimpleDeclarationPtr;


class NEMIVER_API DeclSpecifier {
    DeclSpecifier (const DeclSpecifier&);
    DeclSpecifier& operator= (const DeclSpecifier&);

public:
    enum Kind {
        UNDEFINED,
        //storage class speficiers
        AUTO,
        REGISTER,
        STATIC,
        EXTERN,
        MUTABLE,
        //type specifier
        TYPE,
        //function specifiers
        INLINE,
        VIRTUAL,
        EXPLICIT,
        //friend specifier
        FRIEND,
        //typedef specifier
        TYPEDEF
    };

private:
    Kind m_kind ;

    DeclSpecifier ();
public:

    DeclSpecifier (Kind k):
        m_kind (k)
    {
    }

    virtual ~DeclSpecifier ()
    {
    }

    Kind get_kind () const {return m_kind;}
    void set_kind (Kind a_kind) {m_kind=a_kind;}

    bool is_storage_class ()
    {
        return (m_kind == AUTO || m_kind == REGISTER
                || m_kind == STATIC || m_kind == EXTERN
                || m_kind == MUTABLE) ;
    }
    virtual bool to_string (string &a_str) const=0;
    static bool list_to_string (list<DeclSpecifierPtr> &a_decls, string &a_str);
};//end class DeclSpecifier

class NEMIVER_API AutoSpecifier : public DeclSpecifier {
    AutoSpecifier (const AutoSpecifier&);
    AutoSpecifier& operator= (const AutoSpecifier&);
public:
    AutoSpecifier () :
        DeclSpecifier (AUTO)
    {}
    ~AutoSpecifier () {}
    bool to_string (string &a_str) const {a_str="auto";return true;}
};//end class AutoSpecifier

class NEMIVER_API RegisterSpecifier : public DeclSpecifier {
    RegisterSpecifier (const RegisterSpecifier&);
    RegisterSpecifier& operator= (const RegisterSpecifier&);
public:
    RegisterSpecifier () :
        DeclSpecifier (REGISTER)
    {}
    ~RegisterSpecifier () {}
    bool to_string (string &a_str) const {a_str="register";return true;}
};
typedef shared_ptr<RegisterSpecifier> RegisterSpecifierPtr;

class NEMIVER_API StaticSpecifier : public DeclSpecifier {
    StaticSpecifier (const StaticSpecifier&);
    StaticSpecifier& operator= (const StaticSpecifier&);
public:
    StaticSpecifier () :
        DeclSpecifier (STATIC)
    {}
    ~StaticSpecifier () {}
    bool to_string (string &a_str) const {a_str="static";return true;}
};
typedef shared_ptr<StaticSpecifier> StaticSpecifierPtr;

class NEMIVER_API ExternSpecifier : public DeclSpecifier {
    ExternSpecifier (const ExternSpecifier&);
    ExternSpecifier& operator= (const ExternSpecifier&);
public:
    ExternSpecifier () :
        DeclSpecifier (EXTERN)
    {}
    ~ExternSpecifier () {}
    bool to_string (string &a_str) const {a_str="extern";return true;}
};


class NEMIVER_API MutableSpecifier : public DeclSpecifier {
    MutableSpecifier (const MutableSpecifier&);
    MutableSpecifier& operator= (const MutableSpecifier&);
public:
    MutableSpecifier () :
        DeclSpecifier (MUTABLE)
    {}
    ~MutableSpecifier () {}
    bool to_string (string &a_str) const {a_str="mutable";return true;}
};

class NEMIVER_API TypeSpecifier : public DeclSpecifier {
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
        DeclSpecifier (TYPE),
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
typedef shared_ptr<TypeSpecifier> TypeSpecifierPtr;

class NEMIVER_API InlineSpecifier : public DeclSpecifier {
    InlineSpecifier (const InlineSpecifier&);
    InlineSpecifier& operator= (const InlineSpecifier&);
public:
    InlineSpecifier () :
        DeclSpecifier (INLINE)
    {}
    ~InlineSpecifier () {}
    bool to_string (string &a_str) const {a_str="inline";return true;}
};
typedef shared_ptr<InlineSpecifier> InlineSpecifierPtr;

class NEMIVER_API VirtualSpecifier : public DeclSpecifier {
    VirtualSpecifier (const VirtualSpecifier&);
    VirtualSpecifier& operator= (const VirtualSpecifier&);
public:
    VirtualSpecifier () :
        DeclSpecifier (VIRTUAL)
    {}
    ~VirtualSpecifier () {}
    bool to_string (string &a_str) const {a_str="virtual";return true;}
};
typedef shared_ptr<VirtualSpecifier> VirtualSpecifierPtr;

class NEMIVER_API ExplicitSpecifier : public DeclSpecifier {
    ExplicitSpecifier (const ExplicitSpecifier&);
    ExplicitSpecifier& operator= (const ExplicitSpecifier&);
public:
    ExplicitSpecifier () :
        DeclSpecifier (EXPLICIT)
    {}
    ~ExplicitSpecifier () {}
    bool to_string (string &a_str) const {a_str="explicit";return true;}
};
typedef shared_ptr<ExplicitSpecifier> ExplicitSpecifierPtr;

class NEMIVER_API FriendSpecifier : public DeclSpecifier {
    FriendSpecifier (const FriendSpecifier&);
    FriendSpecifier& operator= (const FriendSpecifier&);

public:
    FriendSpecifier () :
        DeclSpecifier (FRIEND)
    {}
    ~FriendSpecifier () {}
    bool to_string (string &a_str) const {a_str="friend";return true;}
};
typedef shared_ptr<FriendSpecifier> FriendSpecifierPtr;

class NEMIVER_API TypedefSpecifier : public DeclSpecifier {
    TypedefSpecifier (const TypedefSpecifier&);
    TypedefSpecifier& operator= (const TypedefSpecifier&);
public:
    TypedefSpecifier () :
        DeclSpecifier (TYPEDEF)
    {}
    ~TypedefSpecifier () {}
    bool to_string (string &a_str) const {a_str="typedef";return true;}
};
typedef shared_ptr<TypedefSpecifier> TypedefSpecifierPtr;

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
typedef shared_ptr<IDExpr> IDExprPtr;

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
    virtual ~UnqualifiedIDExpr () {}
    virtual bool to_string (string &) const=0;
};//end class UnqualifiedIDExpr
typedef shared_ptr<UnqualifiedIDExpr> UnqualifiedIDExprPtr;

class UnqualifiedID : public UnqualifiedIDExpr {
    string m_name;

public:
    UnqualifiedID ():
        UnqualifiedIDExpr (UnqualifiedIDExpr::IDENTIFIER)
    {}
    UnqualifiedID (const string &a_s):
        UnqualifiedIDExpr (UnqualifiedIDExpr::IDENTIFIER),
        m_name (a_s)
    {}
    ~UnqualifiedID ()
    {}
    const string& get_name () const {return m_name;}
    void set_name (const string &a_n) {m_name=a_n;}
    bool to_string (string &a_s) const;
};
typedef shared_ptr<UnqualifiedID> UnqualifiedIDPtr;

class UnqualifiedOpFuncID : public UnqualifiedIDExpr {
    UnqualifiedOpFuncID (const UnqualifiedOpFuncID&);
    UnqualifiedOpFuncID& operator= (const UnqualifiedOpFuncID&);
    Token m_op_token;

public:
    UnqualifiedOpFuncID ():
        UnqualifiedIDExpr (UnqualifiedIDExpr::OP_FUNC_ID)
    {}
    UnqualifiedOpFuncID (const Token a_t):
        UnqualifiedIDExpr (UnqualifiedIDExpr::OP_FUNC_ID),
        m_op_token (a_t)
    {}
    ~UnqualifiedOpFuncID ()
    {}
    void set_token (Token &a_t) {m_op_token = a_t;}
    Token::Kind get_kind () const {return m_op_token.get_kind ();}
    bool to_string (string &a_s) const;
};
typedef shared_ptr<UnqualifiedOpFuncID>  UnqualifiedOpFuncIDPtr;

class DestructorID : public UnqualifiedIDExpr {
    DestructorID (const DestructorID&);
    DestructorID& operator= (const DestructorID&);
    string m_name;

public:
    DestructorID ():
        UnqualifiedIDExpr (UnqualifiedIDExpr::DESTRUCTOR_ID)
    {}
    DestructorID (const string &a_n) :
        UnqualifiedIDExpr (UnqualifiedIDExpr::DESTRUCTOR_ID),
        m_name (a_n)
    {}
    ~DestructorID ()
    {}
    void set_name (const string &a_n) {m_name=a_n;}
    const string& get_name () const {return m_name;}
    bool to_string (string &a_s) const;
};

class QualifiedIDExpr;
typedef shared_ptr<QualifiedIDExpr> QualifiedIDExprPtr;
class NEMIVER_API QualifiedIDExpr : public IDExpr {
    QualifiedIDExpr (const QualifiedIDExpr &);
    QualifiedIDExpr& operator= (const QualifiedIDExpr &);

    QNamePtr m_scope;
    UnqualifiedIDExprPtr m_id;

public:
    QualifiedIDExpr ():
        IDExpr (QUALIFIED)
    {}
    QualifiedIDExpr (const QNamePtr a_scope, const UnqualifiedIDExprPtr a_id):
        IDExpr (QUALIFIED),
        m_scope (a_scope),
        m_id (a_id)
    {}
    QualifiedIDExpr (const QName &a_scope, const UnqualifiedIDExprPtr a_id):
        IDExpr (QUALIFIED),
        m_scope (new QName (a_scope)),
        m_id (a_id)
    {}

    const QNamePtr get_scope () {return m_scope;}
    void set_scope (const QNamePtr a_scope) {m_scope=a_scope;}
    const UnqualifiedIDExprPtr get_unqualified_id () const {return m_id;}
    void set_unqualified_id (const UnqualifiedIDExprPtr a_id) {m_id = a_id;}
    bool to_string (string &) const ;
    //TODO: support template-id
};//end QualifiedIDExpr


/// \brief the declarator class
/// the result of the direct-declarator production is stored
/// in this type, as well as the declarator production.
class NEMIVER_API Declarator {
    Declarator ();
    Declarator (const Declarator&);
    Declarator& operator= (const Declarator&);


public:
    enum Kind {
        UNDEFINED,
        ID_DECLARATOR,
        FUNCTION_DECLARATOR,
        ARRAY_DECLARATOR,
        FUNCTION_POINTER_DECLARATOR
    };

private:
    Kind m_kind;
    PtrOperatorPtr m_ptr;

public:
    Declarator (Kind k) :
        m_kind (k)
    {}
    virtual ~Declarator () {}
    Kind get_kind () const {return m_kind;}
    void set_kind (Kind k) {m_kind=k;}
    PtrOperatorPtr get_ptr_operator () const {return m_ptr;}
    void set_ptr_operator (const PtrOperatorPtr a_ptr) {m_ptr=a_ptr;}
    virtual bool to_string (string &) const=0;
};//class Declarator
typedef shared_ptr<Declarator> DeclaratorPtr;

class NEMIVER_API IDDeclarator : public Declarator {
    IDDeclarator (const IDDeclarator&);
    IDDeclarator& operator= (const IDDeclarator&);

    IDExprPtr m_id;

public:
    IDDeclarator () :
        Declarator (Declarator::ID_DECLARATOR)
    {}
    IDDeclarator (const IDExprPtr a_id) :
        Declarator (ID_DECLARATOR),
        m_id (a_id)
    {}
    const IDExprPtr get_id () const {return m_id;}
    bool to_string (string &a_str) const
    {
        if (!m_id) {return false;}
        if (get_ptr_operator ()) {a_str = "*";}
        string str;
        m_id->to_string (str);
        a_str += " " + str;
        return true;
    }
};//class IDDeclarator
typedef shared_ptr<IDDeclarator> IDDeclaratorPtr;

class NEMIVER_API FuncDeclarator : public Declarator {
    FuncDeclarator (const FuncDeclarator&);
    FuncDeclarator& operator= (const FuncDeclarator&);

    DeclaratorPtr m_declarator;
    //TODO: put parameters here
    list<CVQualifierPtr> m_cvs;
    //TODO: put exception spec here
public:
    FuncDeclarator () :
        Declarator (FUNCTION_DECLARATOR)
    {}
    ~FuncDeclarator () {}
    void set_declarator (const DeclaratorPtr a_decl) {m_declarator = a_decl;}
    DeclaratorPtr get_declarator () const {return m_declarator;}
    void append_cv_qualifier (const CVQualifierPtr a_cv) {m_cvs.push_back (a_cv);}
    const list<CVQualifierPtr>& get_cv_qualifiers () const {return m_cvs;}
    bool to_string (string &a_str) {a_str="<FuncDecl not supported>";return true;}
};//end FuncDeclarator
typedef shared_ptr<FuncDeclarator> FuncDeclaratorPtr;

class NEMIVER_API ArrayDeclarator : public Declarator {
    ArrayDeclarator (const ArrayDeclarator&);
    ArrayDeclarator& operator= (const ArrayDeclarator&);

    DeclaratorPtr m_declarator;
    //TODO: add constant expression here.

public:
    ArrayDeclarator () :
        Declarator (Declarator::ARRAY_DECLARATOR)
    {}
    ~ArrayDeclarator () {}
    void set_declarator (const DeclaratorPtr a_decl) {m_declarator=a_decl;}
    const DeclaratorPtr get_declarator () const {return m_declarator;}
    bool to_string (string &a_str) {a_str="<ArrayDecl not supported>";return true;}
};//class ArrayDeclarator
typedef shared_ptr<ArrayDeclarator> ArrayDeclaratorPtr;

class NEMIVER_API FuncPointerDeclarator : public Declarator {
    FuncPointerDeclarator (const FuncPointerDeclarator&);
    FuncPointerDeclarator& operator= (const FuncPointerDeclarator&);

    DeclaratorPtr m_declarator;

public:
    FuncPointerDeclarator () :
        Declarator (Declarator::FUNCTION_POINTER_DECLARATOR)
    {}
    ~FuncPointerDeclarator () {}
    void set_declarator (const DeclaratorPtr a_decl) {m_declarator=a_decl;}
    const DeclaratorPtr get_declarator () const {return m_declarator;}
    bool to_string (string &a_str)
    {
        if (!m_declarator) {return false;}
        if (get_ptr_operator ()) {a_str = "*";}
        string str;
        m_declarator->to_string (str);
        a_str += "(" + str + ")";
        return true;
    }
};//class FuncPointerDeclarator

class NEMIVER_API InitDeclarator {
    InitDeclarator (const InitDeclarator&);
    InitDeclarator& operator= (const InitDeclarator&);

    DeclaratorPtr m_declarator;

public:
    InitDeclarator () {}
    InitDeclarator (const DeclaratorPtr &d) :
        m_declarator (d)
    {}
    ~InitDeclarator () {}
    const DeclaratorPtr get_declarator () const {return m_declarator;}
    void set_declarator (const DeclaratorPtr &a_decl) {m_declarator=a_decl;}
    bool to_string (string &a_str) const
    {
        if (!get_declarator ()) {return false;}
        return m_declarator->to_string (a_str);
    }
    static bool list_to_string (list<InitDeclaratorPtr> &, string &);
};

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

