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
#include "nmv-cpp-parser.h"
#include "nmv-cpp-lexer.h"
#include "common/nmv-exception.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

struct Parser::Priv {
    Lexer lexer;

    Priv () :
        lexer ("")
    {
    }

    Priv (const string &a_in):
        lexer (a_in)
    {
    }
};

#define LEXER m_priv->lexer

Parser::Parser (const string &a_in) :
    m_priv (new Priv (a_in))
{
}

Parser::~Parser ()
{
}

/// parse a primary-expression production.
///primary-expression:
///           literal
///           this
///           ( expression )
///           id-expression
/// TODO: handle id-expression
bool
Parser::parse_primary_expr (PrimaryExprPtr &a_expr)
{
    bool status=false;
    Token token;
    PrimaryExprPtr result;
    unsigned mark=LEXER.get_token_stream_mark ();

    if (!LEXER.peek_next_token (token)) {
        return false;
    }
    switch (token.get_kind ()) {
        case Token::KEYWORD:
            if (token.get_str_value () == "this") {
                result.reset (new ThisPrimaryExpr);
                LEXER.consume_next_token ();
            } else {
                IDExprPtr expr;
                if (!parse_id_expr (expr)) {goto error;}
                result = expr;
                goto okay;
            }
            break;
        case Token::PUNCTUATOR_PARENTHESIS_OPEN:
            {
                //consume the open parenthesis
                LEXER.consume_next_token ();
                PrimaryExprPtr expr;
                if (!parse_primary_expr (expr)) {goto error;}
                result.reset (new ParenthesisPrimaryExpr (expr));
                LEXER.consume_next_token (token);
                if (token.get_kind () != Token::PUNCTUATOR_PARENTHESIS_CLOSE) {
                    goto error;
                }
                goto okay;
            }
            break;
        default:
            if (token.is_literal ()) {
                result.reset (new LiteralPrimaryExpr (token));
                LEXER.consume_next_token ();
                goto okay;
            } else {
                IDExprPtr expr;
                if (!parse_id_expr (expr)) {goto error;}
                result = expr;
                goto okay;
            }
            break;
    }

error:
    status=false;
    LEXER.rewind_to_mark (mark);
    goto out;
okay:
    a_expr = result;
    status=true;
out:
    return status;
}

/// parses a postfix-expression production:
///  postfix-expression:
///            primary-expression
///            postfix-expression [ expression ]
///            postfix-expression ( expression-list(opt) )
///            simple-type-specifier ( expression-list(opt))
///            typename ::(opt) nested-name-specifier identifier ( expression-list(opt) )
///            typename ::(opt) nested-name-specifier template(opt) template-id ...
///                ( expression-listopt )
///            postfix-expression . template(opt) id-expression
///            postfix-expression -> template(opt) id-expression
///            postfix-expression . pseudo-destructor-name
///            postfix-expression -> pseudo-destructor-name
///            postfix-expression ++
///            postfix-expression --
///            dynamic_cast < type-id > ( expression )
///            static_cast < type-id > ( expression )
///            reinterpret_cast < type-id > ( expression )
///            const_cast < type-id > ( expression )
///            typeid ( expression )
///            typeid ( type-id )
///
///TODO: this only parse the first two forms of the postfix-expression. So need
//to fill the gap in here.
bool
Parser::parse_postfix_expr (PostfixExprPtr &a_result)
{
    bool status=false;
    PostfixExprPtr result;
    PostfixExprPtr pfe;
    unsigned mark = LEXER.get_token_stream_mark ();

    PrimaryExprPtr primary_expr;
    if (parse_primary_expr (primary_expr)) {
        result.reset (new PrimaryPFE (primary_expr));
        goto okay;
    }
    if (parse_postfix_expr (pfe)) {
        Token token;
        if (LEXER.peek_next_token (token)
            && token.get_kind () == Token::PUNCTUATOR_BRACKET_OPEN) {
            LEXER.consume_next_token ();
            //try to parse postfix-expression [ expression ]
            //we don't have parse_expression() function ready yet, so try
            //to parse primary-expression instead.
            if (!parse_primary_expr (primary_expr)) {
                goto error;
            }
            if (!LEXER.consume_next_token (token)
                || token.get_kind () != Token::PUNCTUATOR_BRACKET_CLOSE) {
                goto error;
            }
            result.reset (new ArrayPFE (pfe, primary_expr));
            goto okay;
        } else {
            //TODO: handle other types of postfix-expression
        }
    }
    //TODO: handle other types of postfix-expression

error:
    LEXER.rewind_to_mark (mark);
    status=false;
    goto out;
okay:
    a_result=result;
    status = true;
out:
    return status;
}


/// parse a cast-expression production.
///
/// cast-expression:
///            unary-expression
///            ( type-id ) cast-expression
bool
Parser::parse_cast_expr (CastExprPtr &a_result)
{
    bool status=false;
    CastExprPtr result;
    Token token;
    unsigned mark=LEXER.get_token_stream_mark ();

    if (!LEXER.peek_next_token (token)) {goto error;}
    if (token.get_kind () == Token::PUNCTUATOR_PARENTHESIS_OPEN) {
        if (!LEXER.consume_next_token ()) {goto error;}
        TypeIDPtr type_id;
        if (!parse_type_id (type_id)) {goto error;}
        if (!LEXER.consume_next_token (token)
            || token.get_kind () != Token::PUNCTUATOR_PARENTHESIS_CLOSE) {
            goto error;
        }
        CastExprPtr right_expr;
        if (!parse_cast_expr (right_expr)) {goto error;}
        result.reset (new CStyleCastExpr (type_id, right_expr));
        goto okay;
    } else  {
        UnaryExprPtr unary_expr;
        if (parse_unary_expr (unary_expr)) {
            result.reset (new UnaryCastExpr (unary_expr));
            goto okay;
        }
    }

error:
    status=false;
    LEXER.rewind_to_mark (mark);
    goto out;
okay:
    status=true;
    a_result=result;
out:
    return status;
}

/// parse a pm-expression production.
///
/// pm-expression:
///           cast-expression
///           pm-expression .* cast-expression
///           pm-expression ->* cast-expression
bool
Parser::parse_pm_expr (PMExprPtr &a_result)
{
    bool status=false;
    CastExprPtr cast_expr, rhs;
    PMExprPtr lhs, result;
    unsigned mark = LEXER.get_token_stream_mark ();
    Token token;

    if (!parse_cast_expr (cast_expr)) {goto error;}
    lhs.reset (new CastPMExpr (cast_expr));

    while (true) {
        if (!LEXER.peek_next_token (token)) {
            result = lhs;
            goto okay;
        }
        if (token.get_kind () == Token::OPERATOR_DOT_STAR
            || token.get_kind () == Token::OPERATOR_ARROW_STAR) {
            LEXER.consume_next_token ();
        } else {
            result = lhs;
            goto okay;
        }
        if (!parse_cast_expr (rhs)) {goto error;}
        if (token.get_kind () == Token::OPERATOR_DOT_STAR) {
            lhs.reset (new DotStarPMExpr (lhs, rhs));
        } else {
            lhs.reset (new ArrowStarPMExpr (lhs, rhs));
        }
    }

error:
    status = false;
    LEXER.rewind_to_mark (mark);
    goto out;
okay:
    status = true;
    a_result = result;
out:
    return status;
}

/// parse a multiplicative-expression production
///
/// multiplicative-expression:
///            pm-expression
///            multiplicative-expression * pm-expression
///            multiplicative-expression / pm-expression
///            multiplicative-expression % pm-expression
bool
Parser::parse_mult_expr (MultExprPtr &a_result)
{
    bool status=false;
    MultExprPtr lhs, result;
    PMExprPtr pm_expr, rhs;
    Expr::Operator op=Expr::OP_UNDEFINED;
    Token token;
    unsigned mark=LEXER.get_token_stream_mark ();

    if (!parse_pm_expr (pm_expr)) {goto error;}
    lhs.reset (new MultExpr (pm_expr));

    while (true) {
        if (!LEXER.peek_next_token (token)) {
            result = lhs;
            goto okay;
        }
        if (token.get_kind () == Token::OPERATOR_MULT) {
            op = Expr::MULT;
        } else if (token.get_kind () == Token::OPERATOR_DIV) {
            op = Expr::DIV;
        } else if (token.get_kind () == Token::OPERATOR_MOD) {
            op = Expr::MOD;
        } else {
            result = lhs;
            goto okay;
        }
        LEXER.consume_next_token ();//consume the operator token
        if (!parse_pm_expr (rhs)) {goto error;}
        lhs.reset (new MultExpr (lhs, op, rhs));
    }

okay:
    status=true;
    a_result=result;
    goto out;
error:
    status=false;
    LEXER.rewind_to_mark (mark);
out:
    return status;
}

/// parses an additive-expression production
///
/// additive-expression:
///            multiplicative-expression
///            additive-expression + multiplicative-expression
///            additive-expression - multiplicative-expression
bool
Parser::parse_add_expr (AddExprPtr &a_result)
{
    bool status=false;
    AddExprPtr lhs, result;
    MultExprPtr mult_expr, rhs;
    Expr::Operator op=Expr::OP_UNDEFINED;
    Token token;
    unsigned mark=LEXER.get_token_stream_mark ();

    if (!parse_mult_expr (mult_expr)) {goto error;}
    lhs.reset (new AddExpr (mult_expr));

    while (true) {
        if (!LEXER.peek_next_token (token)) {
            result = lhs;
            goto okay;
        }
        if (token.get_kind () == Token::OPERATOR_PLUS) {
            op = Expr::PLUS;
        } else if (token.get_kind () == Token::OPERATOR_MINUS) {
            op = Expr::MINUS;
        } else {
            result = lhs;
            goto okay;
        }
        LEXER.consume_next_token ();//consume the operator token
        if (!parse_mult_expr (rhs)) {goto error;}
        lhs.reset (new AddExpr (lhs, op, rhs));
    }

okay:
    status=true;
    a_result=result;
    goto out;
error:
    status=false;
    LEXER.rewind_to_mark (mark);
out:
    return status;
}


/// parses a unary-expression production.
///
/// unary-expression:
///          postfix-expression
///           ++ cast-expression
///           -- cast-expression
///           unary-operator cast-expression
///           sizeof unary-expression
///           sizeof ( type-id )
///           new-expression
///           delete-expression
///TODO: this only support the postfix-expression form parsing.
///      need to support the other forms
bool
Parser::parse_unary_expr (UnaryExprPtr &a_result)
{
    PostfixExprPtr pfe;

    if (parse_postfix_expr (pfe)) {
        a_result.reset (new PFEUnaryExpr (pfe));
        return true;
    }
    return false;
}


/// parses a class-or-namespace-name production.
///
/// class-or-namespace-name:
///           class-name
///           namespace-name
/// with,
/// class-name:
///           identifier
///           template-id
///  and,
//
/// namespace-name:
///           original-namespace-name
///           namespace-alias
/// and,
//
/// original-namespace-name:
///           identifier
///
/// namespace-alias:
///           identifier
bool
Parser::parse_class_or_namespace_name (string &a_result)
{
    Token token;
    if (!LEXER.peek_next_token (token)) {return false;}

    if (token.get_kind () == Token::IDENTIFIER) {
        a_result = token.get_str_value ();
    } else {
        //TODO: handle the case of template-id
        return false;
    }
    LEXER.consume_next_token ();
    return true;
}

/// parse a type-name production.
///
/// type-name:
///           class-name
///           enum-name
///           typedef-name
/// with:
///
/// class-name:
///           identifier
///           template-id
///
/// enum-name:
///      identifier
///
/// typedef-name:
///      identifier
bool
Parser::parse_type_name (UnqualifiedIDExprPtr &a_result)
{
    UnqualifiedIDExprPtr result;
    Token token;

    if (!LEXER.peek_next_token (token)) {return false;}

    if (token.get_kind () == Token::IDENTIFIER) {
        if (!LEXER.consume_next_token ()) {return false;}
        a_result.reset (new UnqualifiedID (token.get_str_value ()));
        return true;
    }
    //TODO: handle template-id
    return false;
}

/// parse nested-name-specifier production.
///
/// nested-name-specifier:
///          class-or-namespace-name :: nested-name-specifier(opt)
///          class-or-namespace-name :: 'template' nested-name-specifier
bool
Parser::parse_nested_name_specifier (QNamePtr &a_result)
{
    bool result=false;
    string con_name, specifier, specifier2;
    QNamePtr qname, qname2;
    Token token;
    unsigned mark = LEXER.get_token_stream_mark ();

    if (!parse_class_or_namespace_name (con_name)) {goto error;}
    qname.reset (new QName);
    qname->append (con_name);
    if (!LEXER.consume_next_token (token)) {goto error;}
    if (token.get_kind () != Token::OPERATOR_SCOPE_RESOL) {goto error;}
    if (parse_nested_name_specifier (qname2)) {
        qname->append (qname2);
    } else {
        if (LEXER.consume_next_token (token)
            && token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template") {
            if (parse_nested_name_specifier (qname2)) {
                qname->append (qname2, true);
            }
        }
    }
    a_result = qname;
    result=true;
    goto out;

error:
    LEXER.rewind_to_mark (mark);
out:
    return result;
}

/// parse an unqualified-id production.
///
/// unqualified-id:
///   identifier
///   operator-function-id
///   conversion-function-id
///   ~class-name
///   template-id
///
/// TODO: support template-id and conversion-function-id cases.
bool
Parser::parse_unqualified_id (UnqualifiedIDExprPtr &a_expr)
{
    Token token;
    if (!LEXER.peek_next_token (token)) {return false;}

    UnqualifiedIDExprPtr expr;
    switch (token.get_kind ()) {
        case Token::IDENTIFIER:
            expr.reset (new UnqualifiedID (token.get_str_value ()));
            break;
        case Token::KEYWORD:
            //operator-function-id:
            // 'operartor' operator
            if (token.get_str_value () == "operator") {
                //look forward the second token in the queue to see if it's an
                //operator
                Token op_token;
                if (!LEXER.peek_nth_token (1, op_token)) {return false;}
                if (!op_token.is_operator ()) {return false;}
                //okay the second token to come is an operator so consume the
                //first token and build the expression.
                LEXER.consume_next_token ();
                expr.reset (new UnqualifiedOpFuncID (op_token));
            } else {
                return false;
            }
            break;
        case Token::OPERATOR_BIT_COMPLEMENT:
            {
                // ~class-name, with
                // class-name:
                //   identifier
                //   template-id

                //lookahead the second token in the stream to see if it's an
                //identifier token
                Token t;
                if (LEXER.peek_nth_token (1, t) && t.get_kind () == Token::IDENTIFIER) {
                    LEXER.consume_next_token ();
                    expr.reset (new DestructorID (t.get_str_value ()));
                } else {
                    //TODO: support template-id
                    return false;
                }
            }
            break;
        default:
            //TODO: support template-id and conversion-function-id
            break;
    }
    if (!expr)
        return false;
    if (!LEXER.consume_next_token ()) {
        //should never been reached.
        return false;
    }
    a_expr = expr;
    return true;
}

/// parse a qualified-id production.
///
/// qualified-id:
///   ::(opt) nested-name-specifier 'template'(opt) unqualified-id
///   :: identifier
///   :: operator-function-id
///   :: template-id
bool
Parser::parse_qualified_id (QualifiedIDExprPtr &a_expr)
{
    bool result=false, has_template=false;
    UnqualifiedIDExprPtr id;
    Token token;
    QNamePtr scope;
    unsigned mark = LEXER.get_token_stream_mark ();

    if (!LEXER.consume_next_token (token)) {return false;}
    QualifiedIDExprPtr expr;

    if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        LEXER.consume_next_token ();
    }

    if (parse_nested_name_specifier (scope)) {
        if (LEXER.peek_next_token (token)
            && token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template") {
            LEXER.consume_next_token ();
            has_template=true;
            if (!parse_unqualified_id (id)) {goto error;}
        }
        expr.reset (new QualifiedIDExpr (scope, id));
        goto okay;
    }
    if (token.get_kind () != Token::OPERATOR_SCOPE_RESOL) {
        goto error;
    }
    if (!LEXER.consume_next_token (token)) {goto error;}

    if (parse_unqualified_id (id)) {
        expr.reset (new QualifiedIDExpr (scope, id));
    } else {
        goto error;
    }

okay:
    a_expr=expr;
    result = true;
    goto out;
error:
    LEXER.rewind_to_mark (mark);
out:
    return result;
}

/// parse an id-expression production.
///
/// id-expression:
///            unqualified-id
///            qualified-id
bool
Parser::parse_id_expr (IDExprPtr &a_expr)
{
    bool result=false;
    Token token;

    if (!LEXER.peek_next_token (token)) {return false;}
    switch (token.get_kind ()) {
        case Token::KEYWORD:
        case Token::OPERATOR_BIT_COMPLEMENT: {
            UnqualifiedIDExprPtr unq_expr;
            result = parse_unqualified_id (unq_expr);
            if (result) {
                a_expr = unq_expr;
            }
            return result;
        }
        case Token::OPERATOR_SCOPE_RESOL: {
            QualifiedIDExprPtr q_expr;
            result = parse_qualified_id (q_expr);
            if (result) {
                a_expr = q_expr;
            }
            return result;
        }
        case Token::IDENTIFIER: {
            UnqualifiedIDExprPtr unq_expr;
            QualifiedIDExprPtr q_expr;
            if (parse_unqualified_id (unq_expr)) {
                a_expr = unq_expr;
                result = true;
            } else {
                if (parse_qualified_id (q_expr)) {
                    a_expr = q_expr;
                    result = true;
                }
            }
            return result;
        }
        default:
            break;
    }
    return false;
}

/// parse the elaborated-type-specifier production.
///
/// elaborated-type-specifier:
///           class-key ::(opt) nested-name-specifier(opt) identifier
///           enum ::(opt) nested-name-specifier(opt) identifier
///           typename ::(opt) nested-name-specifier identifier
///           typename ::(opt) nested-name-specifier template(opt) template-id
bool
Parser::parse_elaborated_type_specifier (ElaboratedTypeSpecPtr &a_result)
{
    bool status=false;
    ElaboratedTypeSpecPtr result;
    Token token;
    list<ElaboratedTypeSpec::ElemPtr> elems;
    ElaboratedTypeSpec::ElemPtr elem;
    unsigned mark = LEXER.get_token_stream_mark ();

    if (!LEXER.consume_next_token (token)) {goto error;}

    if (token.get_kind () == Token::KEYWORD) {
        if (token.get_str_value () == "class") {
            elem.reset (new ElaboratedTypeSpec::ClassElem);
            elems.push_back (elem);
        } else if (token.get_str_value () == "struct") {
            elem.reset (new ElaboratedTypeSpec::StructElem);
            elems.push_back (elem);
        } else if (token.get_str_value () == "union") {
            elem.reset (new ElaboratedTypeSpec::UnionElem);
            elems.push_back (elem);
        } else {
            goto error;
        }
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
            //TODO: add empty scope here.
            if (!LEXER.consume_next_token ()) {goto error;}
        }
        QNamePtr scope;
        parse_nested_name_specifier (scope);
        if (scope) {
            elem.reset (new ElaboratedTypeSpec::ScopeElem (scope));
            elems.push_back (elem);
        }
        if (!LEXER.consume_next_token (token)) {goto error;}
        if (token.get_kind () != Token::IDENTIFIER) {goto error;}
        elem.reset (new ElaboratedTypeSpec::IdentifierElem (token.get_str_value ()));
        elems.push_back (elem);
        result.reset (new ElaboratedTypeSpec (elems));
        goto okay;
    } else if (token.get_kind () == Token::KEYWORD
               && token.get_str_value () == "enum") {
        elem.reset (new ElaboratedTypeSpec::EnumElem);
        elems.push_back (elem);
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
            //TODO: add empty scope here.
            LEXER.consume_next_token ();
        }
        QNamePtr scope;
        if (parse_nested_name_specifier (scope)) {
            elem.reset (new ElaboratedTypeSpec::ScopeElem (scope));
            elems.push_back (elem);
        }
        if (!LEXER.consume_next_token (token)) {goto error;}
        if (token.get_kind () != Token::IDENTIFIER) {goto error;}
        elem.reset (new ElaboratedTypeSpec::IdentifierElem (token.get_str_value ()));
        elems.push_back (elem);
        result.reset (new ElaboratedTypeSpec (elems));
        goto okay;
    } else if (token.get_kind () == Token::KEYWORD
               && token.get_str_value () == "typename") {
        elem.reset (new ElaboratedTypeSpec::TypenameElem);
        elems.push_back (elem);
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
            LEXER.consume_next_token ();
            //TODO: add empty scope here.
        }
        QNamePtr scope;
        if (!parse_nested_name_specifier (scope) || !scope) {goto error;}
        elem.reset (new ElaboratedTypeSpec::ScopeElem (scope));
        elems.push_back (elem);
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::IDENTIFIER) {
            LEXER.consume_next_token ();
            elem.reset (new ElaboratedTypeSpec::IdentifierElem (token.get_str_value ()));
            elems.push_back (elem);
            result.reset (new ElaboratedTypeSpec (elems));
            goto okay;
        }
        if (token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template"){
            LEXER.consume_next_token ();
            elem.reset (new ElaboratedTypeSpec::ScopeElem (scope));
            elems.push_back (elem);
        }
        //TODO: handle template id
        //string template_id;
        //if (!parse_template_id (template_id)) {goto error;}
        //str += template_id;
        //goto okay;
        goto error;
    } else {
        goto error;
    }

okay:
    status = true;
    a_result = result;
    goto out;
error:
    LEXER.rewind_to_mark (mark);
out:
    return status;

}

/// parse a simple-type-specifier production
///
/// simple-type-specifier:
///             ::(opt) nested-name-specifier(opt) type-name
///             ::(opt) nested-name-specifier template template-id
///             char
///             wchar_t
///             bool
///             short
///             int
///             long
///             signed
///             unsigned
///             float
///             double
///             void
/// TODO: support template-id part
bool
Parser::parse_simple_type_specifier (SimpleTypeSpecPtr &a_str)
{
    bool status=false;
    string str;
    SimpleTypeSpecPtr result;
    Token token;
    QNamePtr scope;
    UnqualifiedIDExprPtr type_name;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (!LEXER.peek_next_token (token)) {goto error;}

    if (token.get_kind () == Token::KEYWORD
        && (token.get_str_value () == "char")
            || token.get_str_value () == "wchar_t"
            || token.get_str_value () == "bool"
            || token.get_str_value () == "short"
            || token.get_str_value () == "int"
            || token.get_str_value () == "long"
            || token.get_str_value () == "signed"
            || token.get_str_value () == "unsigned"
            || token.get_str_value () == "float"
            || token.get_str_value () == "double"
            || token.get_str_value () == "void") {
        LEXER.consume_next_token ();
        result.reset (new SimpleTypeSpec (scope, token.get_str_value ()));
        goto okay;
    }

    if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        LEXER.consume_next_token ();
        //TODO: add nil scope;
    }
    if (parse_nested_name_specifier (scope) && scope) {
        if (LEXER.peek_next_token (token)
            && token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template") {
            //TODO: handle template id
            goto error;
        }
    }
    if (!parse_type_name (type_name) || !type_name) {goto error;}
    type_name->to_string (str);
    result.reset (new SimpleTypeSpec (scope, str));

okay:
    status = true;
    a_str = result;
    goto out;
error:
    LEXER.rewind_to_mark (mark);
out:
    return status;
}

/// parse a type-specifier production
///
/// type-specifier:
///            simple-type-specifier
///            class-specifier
///            enum-specifier
///            elaborated-type-specifier
///            cv-qualifier
/// TODO: handle class-specifier and enum specifier
bool
Parser::parse_type_specifier (TypeSpecifierPtr &a_result)
{
    string str;
    TypeSpecifierPtr result ;
    SimpleTypeSpecPtr type_spec;
    ElaboratedTypeSpecPtr type_spec2;
    Token token;
    bool is_ok=false;
    unsigned mark = LEXER.get_token_stream_mark ();

    if (parse_simple_type_specifier (type_spec)) {
        result = type_spec;
        goto okay;
    }
    if (parse_elaborated_type_specifier (type_spec2)) {
        result = type_spec2;
        goto okay;
    }
    if (LEXER.consume_next_token (token)
        && token.get_kind () == Token::KEYWORD) {
        if (token.get_str_value () == "const") {
            result.reset (new ConstTypeSpec);
            goto okay;
        } else if (token.get_str_value () == "volatile") {
            result.reset (new VolatileTypeSpec);
            goto okay;
        } else {
            goto error;
        }
    }
    // TODO: handle class-specifier and enum specifier

error:
    LEXER.rewind_to_mark (mark);
    is_ok = false;
    goto out;
okay:
    a_result = result;
    is_ok = true;
out:
    return is_ok;
}

/// parse a type-specifier-seq production.
///
/// type-specifier-seq:
///            type-specifier type-specifier-seq(opt)
///
bool
Parser::parse_type_specifier_seq (list<TypeSpecifierPtr> &a_result)
{
    TypeSpecifierPtr type_spec;

    if (!parse_type_specifier (type_spec)) {return false;}
    a_result.push_back (type_spec);
    while (parse_type_specifier (type_spec)) {
        a_result.push_back (type_spec);
    }
    return true;
}

/// parse a type-id production
///
/// type-id:
///         type-specifier-seq abstract-declarator(opt)
///TODO: handle abstract-declarator
bool
Parser::parse_type_id (TypeIDPtr &a_result)
{
    list<TypeSpecifierPtr> type_specs;

    if (!parse_type_specifier_seq (type_specs)) {return false;}
    a_result.reset (new TypeID (type_specs));
    //TODO:handle abstract-declarator here.
    return true;
}

/// parse a decl-specifier production
///
///decl-specifier:
///            storage-class-specifier
///            type-specifier
///            function-specifier
///            friend
///            typedef
///
/// TODO: handle function-specifier
bool
Parser::parse_decl_specifier (DeclSpecifierPtr &a_result)
{
    bool status=false;
    Token token;
    TypeSpecifierPtr type_spec;
    DeclSpecifierPtr result;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (!LEXER.peek_next_token (token)) {goto error;}

    if (token.get_kind () == Token::KEYWORD) {
        if (token.get_str_value () == "auto") {
            result.reset (new AutoSpecifier);
        } else if (token.get_str_value () == "register") {
            result.reset (new RegisterSpecifier);
        } else if (token.get_str_value () == "static") {
            result.reset (new StaticSpecifier);
            result->set_kind (DeclSpecifier::STATIC);
        } else if (token.get_str_value () == "extern") {
            result.reset (new ExternSpecifier);
        } else if (token.get_str_value () == "mutable") {
            result.reset (new MutableSpecifier);
        } else if (token.get_str_value () == "friend") {
            result.reset (new FriendSpecifier);
        } else if (token.get_str_value () == "typedef") {
            result.reset (new TypedefSpecifier);
        } else {
            if (!parse_type_specifier (type_spec)) {
                goto error;
            }
            result = type_spec;
            goto okay;
        }
        LEXER.consume_next_token ();
        if (!result) {goto error;}
        goto okay;
    }
    if (parse_type_specifier (type_spec)) {
        result = type_spec;
        goto okay;
    }
    //TODO: handle function-specifier

error:
    status=false;
    goto out;
    LEXER.rewind_to_mark (mark);

okay:
    status = true;
    a_result=result;

out:
    return status;
}

/// parse a decl-specifier-seq production.
///
/// decl-specifier-seq:
///            decl-specifier-seq(opt) decl-specifier
///
/// [dcl.type]
///
/// As a general rule, at most one type-specifier is
///  allowed in the complete decl-specifier-seq of a declaration.
/// The only exceptions to this rule are the following:
/// — const or volatile can be combined with any other type-specifier.
///   However, redundant cv- qualifiers are prohibited except
///   when introduced through the use of typedefs (7.1.3) or template type
///   arguments (14.3), in which case the redundant cv-qualifiers are ignored.
/// — signed or unsigned can be combined with char, long, short, or int.
/// — short or long can be combined with int.
/// — long can be combined with double.
/// TODO: finish this.
bool
Parser::parse_decl_specifier_seq (list<DeclSpecifierPtr> &a_result)
{
    string str;
    list<DeclSpecifierPtr> result;
    DeclSpecifierPtr decl, decl2;
    bool is_ok=false;
    unsigned mark = LEXER.get_token_stream_mark ();
    Token token;
    int nb_type_specifiers=0;

    //TODO:REMIND: make sure only one type-specifier is parsed at most, except
    //for the special cases.

    unsigned mark2=0;
    while (true ) {
        mark2=LEXER.get_token_stream_mark ();
        if (!parse_decl_specifier (decl) || !decl) {
            break;
        }
        //type specifier special cases
        if (decl->get_kind () == DeclSpecifier::TYPE) {
            if (nb_type_specifiers) {
                LEXER.rewind_to_mark (mark2);
                break;
            }
            TypeSpecifierPtr type_specifier =
                std::tr1::static_pointer_cast<TypeSpecifier> (decl);
            //— const or volatile can be combined with any other type-specifier.
            //  However, redundant cv-qualifiers are prohibited except
            if (type_specifier->is_cv_qualifier ()) {
                result.push_back (type_specifier);
                if (!LEXER.peek_next_token (token)) {break;}
                if (type_specifier->get_kind () == TypeSpecifier::CONST
                    && token.get_kind () == Token::KEYWORD
                    && token.get_str_value () == "const") {
                    goto error;
                } else if (type_specifier->get_kind () == TypeSpecifier::VOLATILE
                           && token.get_kind () == Token::KEYWORD
                           && token.get_str_value () == "volatile") {
                    goto error;
                }
            } else if (type_specifier->get_kind () == TypeSpecifier::SIMPLE) {
                //— signed or unsigned can be combined with char, long, short, or int.
                //— short or long can be combined with int.
                //— long can be combined with double.
                //TODO: handle this
                SimpleTypeSpecPtr simple_type_spec =
                    std::tr1::static_pointer_cast<SimpleTypeSpec> (type_specifier);

                result.push_back (simple_type_spec);
                if (simple_type_spec->get_name () == "signed"
                    || simple_type_spec->get_name () == "unsigned") {
                    if (!LEXER.peek_next_token (token)) {break;}
                    if (token.get_kind () == Token::KEYWORD
                        && (token.get_str_value () ==    "char"
                            || token.get_str_value () == "long"
                            || token.get_str_value () == "short"
                            || token.get_str_value () == "int")) {
                        if (!parse_decl_specifier (decl) || !decl) {goto error;}
                        result.push_back (decl);
                        nb_type_specifiers++;
                        continue;
                    }
                } else if (simple_type_spec->get_name () == "short"
                           || simple_type_spec->get_name () == "long") {
                    if (!LEXER.peek_next_token (token)) {break;}
                    if (token.get_kind () == Token::KEYWORD
                        && (token.get_str_value () == "int")) {
                        if (!parse_decl_specifier (decl) || !decl) {goto error;}
                        result.push_back (decl);
                        nb_type_specifiers++;
                    } else if (simple_type_spec->get_name () == "long"
                               && token.get_kind () == Token::KEYWORD
                               && token.get_str_value () == "double") {
                        if (!parse_decl_specifier (decl) || !decl) {goto error;}
                        result.push_back (decl);
                        nb_type_specifiers++;
                    } else {
                        nb_type_specifiers++;
                    }
                } else {
                    nb_type_specifiers++;
                }
            } else {
                result.push_back (decl);
                nb_type_specifiers++;
            }
        } else {
            result.push_back (decl);
        }
    }
    if (result.empty ())
        goto error;

    a_result = result;
    is_ok=true;
    goto out;
error:
    LEXER.rewind_to_mark (mark);
    is_ok =false;
out:
    return is_ok;
}

///
///
/// declarator-id:
///            id-expression
///            ::(opt) nested-name-specifier(opt) type-name
bool
Parser::parse_declarator_id (IDDeclaratorPtr &a_result)
{
    bool status=false;
    string str;
    IDDeclaratorPtr result;
    IDExprPtr id_expr;
    UnqualifiedIDExprPtr type_name;
    Token token;
    QNamePtr scope;
    unsigned mark = LEXER.get_token_stream_mark ();

    if (parse_id_expr (id_expr)) {
        result.reset (new IDDeclarator (id_expr));
        if (!result) {
            goto error;
        } else {
            goto okay;
        }
    }
    if (!LEXER.peek_next_token (token)) {goto error;}
    if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        LEXER.consume_next_token ();
    }
    parse_nested_name_specifier (scope) ;
    if (parse_type_name (type_name)) {
        IDExprPtr id_expr (new QualifiedIDExpr (scope, type_name));
        result.reset (new IDDeclarator (id_expr));
        goto okay;
    }

error:
    LEXER.rewind_to_mark (mark);
    status = false;
    goto out;

okay:
    a_result = result;
    status = true;
    

out:
    return status;
}

/// parse a direct-declarator production.
///
///direct-declarator:
///            declarator-id
///            direct-declarator ( parameter-declaration-clause ) cv-qualifier-seq(opt)...
///               exception-specification(opt)
///            direct-declarator [ constant-expression(opt) ]
///            ( declarator )
/// TODO: handle function style direct-declarator
/// TODO: handle subscript style direct-declarator
/// TODO: handle function pointer style direct declarator
bool
Parser::parse_direct_declarator (DeclaratorPtr &a_result)
{
    bool status=false;
    DeclaratorPtr result;
    IDDeclaratorPtr id;
    unsigned mark = LEXER.get_token_stream_mark ();

    if (!parse_declarator_id (id) || !id) {goto error;}

    //TODO: handle function style direct-declarator
    //TODO: handle subscript style direct-declarator
    //TODO: handle function pointer style direct declarator
    result = id;
    goto okay;

error:
    LEXER.rewind_to_mark (mark);
    status = false;
    goto out;

okay:
    status = true;
    a_result = result;

out:
    return status;
}

bool
Parser::parse_cv_qualifier (CVQualifierPtr &a_result)
{
    bool status=false;
    Token token;
    CVQualifierPtr result;

    if (!LEXER.consume_next_token (token)) {goto error;}

    if (token.get_kind () == Token::KEYWORD) {
        if (token.get_str_value () == "const") {
            result.reset (new ConstQualifier);
        } else if (token.get_str_value () == "volatile") {
            result.reset (new VolatileQualifier);
        } else {
            goto error;
        }
        goto okay;
    }

error:
    status = false;
    goto out;
okay:
    status = true;
    a_result = result;
out:
    return status;
}

/// parses the cv-qualifier-seq production
///
/// cv-qualifier-seq:
///          cv-qualifier cv-qualifier-seq(opt)
///
bool
Parser::parse_cv_qualifier_seq (list<CVQualifierPtr> &a_result)

{
    bool status=false;
    list<CVQualifierPtr> result;
    CVQualifierPtr qualifier;
    unsigned mark = LEXER.get_token_stream_mark ();

    while (parse_cv_qualifier (qualifier) && qualifier) {
            result.push_back (qualifier);
    }
    if (result.empty ()) {goto error;}
    goto okay;

error:
    LEXER.rewind_to_mark (mark);
    status = false;
    goto out;

okay:
    status = true;
    a_result = result;

out:
    return status;
}

///
///
/// ptr-operator:
///           * cv-qualifier-seq(opt)
///           &
///           ::(opt) nested-name-specifier * cv-qualifier-seq(opt)
bool
Parser::parse_ptr_operator (PtrOperatorPtr &a_result)
{
    bool status=false;
    string  str;
    PtrOperatorPtr result;
    Token token;
    QNamePtr scope;
    PtrOperator::ElemPtr elem;
    list<CVQualifierPtr> qualifiers;
    unsigned mark = LEXER.get_token_stream_mark ();

start:
    if (!LEXER.consume_next_token (token)) {goto error;}
    result.reset (new PtrOperator);

    if (token.get_kind () == Token::OPERATOR_BIT_AND) {
        elem.reset (new PtrOperator::AndElem);
        result->append (elem);
        goto okay;
    } else if (token.get_kind () == Token::OPERATOR_MULT) {
        elem.reset (new PtrOperator::StarElem);
        result->append (elem);
        if (parse_cv_qualifier_seq (qualifiers)) {
            for (list<CVQualifierPtr>::const_iterator it=qualifiers.begin ();
                 it != qualifiers.end ();
                 ++it) {
                if ((*it)->get_kind () == CVQualifier::CONST) {
                    elem.reset (new PtrOperator::ConstElem);
                } else if ((*it)->get_kind () == CVQualifier::VOLATILE) {
                    elem.reset (new PtrOperator::VolatileElem);
                } else {
                    continue;
                }
                result->append (elem);
            }
        }
        goto okay;
    } else if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        //TODO: insert nil scope here.
    }
    if (parse_nested_name_specifier (scope)) {
        result->set_scope (scope);
        goto start;
    }
    goto error;

error:
    status = false;
    LEXER.rewind_to_mark (mark);
    goto out;

okay:
    status = true;
    a_result = result;

out:
    return status;
}

/// parse a declarator production.
///
/// declarator:
///            direct-declarator
///            ptr-operator declarator
bool
Parser::parse_declarator (DeclaratorPtr &a_result)
{
    DeclaratorPtr result;
    PtrOperatorPtr ptr;
    unsigned mark = LEXER.get_token_stream_mark ();

    if (parse_direct_declarator (a_result)) {
        return true;
    }
    if (parse_ptr_operator (ptr)) {
        if (!parse_direct_declarator (a_result)) {
            LEXER.rewind_to_mark (mark);
            return false;
        }
        a_result->set_ptr_operator (ptr);
        return true;
    }
    return false;
}

/// parse an init-declarator production
///
/// init-declarator:
///            declarator initializer(opt)
///
/// TODO: support initializer
bool
Parser::parse_init_declarator (InitDeclaratorPtr &a_decl)
{
    DeclaratorPtr decl;
    if (parse_declarator (decl)) {
        a_decl.reset (new InitDeclarator (decl));
        return true;
    }
    //TODO: support initializer
    return false;
}

/// parse an init-declarator-list production.
///
/// init-declarator-list:
///           init-declarator
///           init-declarator-list , init-declarator
bool
Parser::parse_init_declarator_list (list<InitDeclaratorPtr> &a_result)
{
    bool status=false;
    Token token;
    InitDeclaratorPtr decl;
    list<InitDeclaratorPtr> result;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (parse_init_declarator (decl)) {
        result.push_back (decl);
    } else {
        goto error;
    }

loop:
    if (LEXER.peek_next_token (token)
        && token.get_kind () == Token::OPERATOR_SEQ_EVAL) {
        if (parse_init_declarator (decl)) {
            result.push_back (decl);
        } else {
            goto okay;
        }
    } else {
        goto okay;
    }
    goto loop;

error:
    LEXER.rewind_to_mark (mark);
    status = false;
    goto out;

okay:
    status = true;
    a_result = result;

out:
    return status;
}

/// parse a simple-declaration production
///
/// simple-declaration:
///           decl-specifier-seq(opt) init-declarator-list(opt) ;
///
bool
Parser::parse_simple_declaration (SimpleDeclarationPtr &a_result)
{
    list<DeclSpecifierPtr>  decl_specs;
    list<InitDeclaratorPtr> init_decls;

    if (!parse_decl_specifier_seq (decl_specs))
        return true;
    parse_init_declarator_list (init_decls) ;
    a_result.reset (new SimpleDeclaration (decl_specs, init_decls));
    return true;
}

NEMIVER_END_NAMESPACE (nemiver)
NEMIVER_END_NAMESPACE (cpp)
