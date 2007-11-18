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
Parser::parse_primary_expr (shared_ptr<PrimaryExpr> &a_expr)
{
    Token token;
    if (!LEXER.peek_next_token (token)) {
        return false;
    }
    shared_ptr<PrimaryExpr> pe (new PrimaryExpr);
    switch (token.get_kind ()) {
        case Token::KEYWORD:
            if (token.get_str_value () == "this") {
                pe->set_kind (PrimaryExpr::THIS);
            } else {
                //TODO: handle id-expression
                return false;
            }
            break;
        case Token::PUNCTUATOR_PARENTHESIS_OPEN:
            //TODO: support sub expr parsing.
            return false;
            break;
        default:
            if (token.is_literal ()) {
                pe->set_token (PrimaryExpr::LITERAL, token);
            } else {
                //TODO: handle id-expression
                return false;
            }
            break;
    }
    if (pe->get_kind () == PrimaryExpr::UNDEFINED)
        return false;
    a_expr = pe;
    LEXER.consume_next_token ();
    return true;
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
Parser::parse_type_name (string &a_str)
{
    Token token;

    if (!LEXER.peek_next_token (token)) {return false;}

    if (token.get_kind () == Token::IDENTIFIER) {
        if (!LEXER.consume_next_token ()) {return false;}
        a_str = token.get_str_value ();
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
Parser::parse_nested_name_specifier (string &a_specifier)
{
    bool result=false;
    string con_name, specifier, specifier2;
    Token token;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (!parse_class_or_namespace_name (con_name)) {goto error;}
    specifier += con_name;
    if (!LEXER.consume_next_token (token)) {goto error;}
    if (token.get_kind () != Token::OPERATOR_SCOPE_RESOL) {goto error;}
    specifier += "::" ;

    if (parse_nested_name_specifier (specifier2)) {
        specifier += specifier2;
    } else {
        if (LEXER.consume_next_token (token)
            && token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template") {
            if (parse_nested_name_specifier (specifier2)) {
                specifier += specifier2;
            }
        }
    }
    a_specifier = specifier;
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
Parser::parse_unqualified_id (shared_ptr<UnqualifiedIDExpr> &a_expr)
{
    Token token;
    if (!LEXER.peek_next_token (token)) {return false;}

    shared_ptr<UnqualifiedIDExpr> expr (new UnqualifiedIDExpr);
    switch (token.get_kind ()) {
        case Token::IDENTIFIER:
            expr->set_token (UnqualifiedIDExpr::IDENTIFIER, token);
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
                expr->set_token (UnqualifiedIDExpr::OP_FUNC_ID, op_token);
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
                if (LEXER.peek_nth_token (1, t)) {
                    LEXER.consume_next_token ();
                    expr->set_token (UnqualifiedIDExpr::DESTRUCTOR_ID, t);
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
Parser::parse_qualified_id (shared_ptr<QualifiedIDExpr> &a_expr)
{
    bool result=false;

    unsigned mark = LEXER.get_token_stream_mark ();

    Token token;
    string nested_name_specifier;

    if (!LEXER.consume_next_token (token)) {return false;}
    shared_ptr<QualifiedIDExpr> expr (new QualifiedIDExpr);

    if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        LEXER.consume_next_token ();
    }

    if (parse_nested_name_specifier (nested_name_specifier)) {
        if (LEXER.peek_next_token (token)
            && token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template") {
            LEXER.consume_next_token ();
            shared_ptr<UnqualifiedIDExpr> unqualified_id;
            if (!parse_unqualified_id (unqualified_id)) {goto error;}
            expr->set_template (unqualified_id);
        }
        expr->set_nested_name_specifier (nested_name_specifier);
        expr->set_kind (QualifiedIDExpr::NESTED_ID);
        goto okay;
    }
    if (token.get_kind () != Token::OPERATOR_SCOPE_RESOL) {
        goto error;
    }
    if (!LEXER.consume_next_token (token)) {goto error;}

    if (token.get_kind () == Token::IDENTIFIER) {
        expr->set_token (QualifiedIDExpr::IDENTIFIER_ID, token);
        goto okay;
    } else if (token.get_kind () == Token::KEYWORD
               && token.get_str_value () == "operator") {
        //operator-function-id:
        // 'operartor' operator
        Token op_token;
        if (!LEXER.consume_next_token (op_token)) {goto error;}
        if (op_token.is_operator ()) {goto error;}
        expr->set_token (QualifiedIDExpr::OP_FUNC_ID, op_token);
        goto okay;
    } else {
        //TODO: handle template-id case
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
Parser::parse_id_expr (shared_ptr<IDExpr> &a_expr)
{
    bool result=false;
    Token token;

    if (!LEXER.peek_next_token (token)) {return false;}
    switch (token.get_kind ()) {
        case Token::KEYWORD:
        case Token::OPERATOR_BIT_COMPLEMENT: {
            shared_ptr<UnqualifiedIDExpr> unq_expr;
            result = parse_unqualified_id (unq_expr);
            if (result) {
                a_expr = unq_expr;
            }
            return result;
        }
        case Token::OPERATOR_SCOPE_RESOL: {
            shared_ptr<QualifiedIDExpr> q_expr;
            result = parse_qualified_id (q_expr);
            if (result) {
                a_expr = q_expr;
            }
            return result;
        }
        case Token::IDENTIFIER: {
            shared_ptr<UnqualifiedIDExpr> unq_expr;
            shared_ptr<QualifiedIDExpr> q_expr;
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
Parser::parse_elaborated_type_specifier (string &a_str)
{
    bool result=false;
    string str;
    Token token;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (!LEXER.consume_next_token (token)) {goto error;}

    if (token.get_kind () == Token::KEYWORD
        && (token.get_str_value () == "class"
            || token.get_str_value () == "struct"
            || token.get_str_value () == "union")) {
        str += token.get_str_value () + " ";
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
            str += "::";
            if (!LEXER.consume_next_token ()) {goto error;}
        }
        string nested;
        if (parse_nested_name_specifier (nested)) {
            str += nested + " ";
        }
        if (!LEXER.consume_next_token (token)) {goto error;}
        if (token.get_kind () != Token::IDENTIFIER) {goto error;}
        str += token.get_str_value ();
        goto okay;
    } else if (token.get_kind () == Token::KEYWORD
               && token.get_str_value () == "enum") {
        str += "enum " ;
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
            str += "::";
            LEXER.consume_next_token ();
        }
        string nested;
        if (parse_nested_name_specifier (nested)) {
            str += nested;
        }
        if (!LEXER.consume_next_token (token)) {goto error;}
        if (token.get_kind () != Token::IDENTIFIER) {goto error;}
        str += token.get_str_value () ;
        goto okay;
    } else if (token.get_kind () == Token::KEYWORD
               && token.get_str_value () == "typename") {
        str += "typename ";
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
            LEXER.consume_next_token ();
            str += "::";
        }
        string nested_ns;
        if (!parse_nested_name_specifier (nested_ns)) {goto error;}
        str += nested_ns;
        if (!LEXER.peek_next_token (token)) {goto error;}
        if (token.get_kind () == Token::IDENTIFIER) {
            LEXER.consume_next_token ();
            str += token.get_str_value ();
            goto okay;
        }
        if (token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template"){
            LEXER.consume_next_token ();
            str += "template ";
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
    result = true;
    a_str = str;
    goto out;
error:
    LEXER.rewind_to_mark (mark);
out:
    return result;

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
Parser::parse_simple_type_specifier (string &a_str)
{
    bool status=false;
    string result, str;
    Token token;

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
        result = token.get_str_value ();
        goto okay;
    }

    if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        LEXER.consume_next_token ();
        result += "::";
    }
    if (parse_nested_name_specifier (str)) {
        result += str;
        str.clear ();
        if (LEXER.peek_next_token (token)
            && token.get_kind () == Token::KEYWORD
            && token.get_str_value () == "template") {
            //TODO: handle template id
            goto error;
        }
    }
    if (!parse_type_name (str)) {goto error;}
    result += str;

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
Parser::parse_type_specifier (shared_ptr<TypeSpecifier> &a_result)
{
    string str;
    shared_ptr<TypeSpecifier> result (new TypeSpecifier);
    Token token;
    bool is_ok=false;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (parse_simple_type_specifier (str)) {
        result->set_simple (str);
        goto okay;
    }
    if (parse_elaborated_type_specifier (str)) {
        result->set_elaborated (str);
        goto okay;
    }
    if (LEXER.consume_next_token (token)
        && token.get_kind () == Token::KEYWORD) {
        if (token.get_str_value () == "const") {
            LEXER.consume_next_token ();
            result->set_kind (TypeSpecifier::CONST);
        } else if (token.get_str_value () == "volatile") {
            LEXER.consume_next_token ();
            result->set_kind (TypeSpecifier::VOLATILE);
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

/// parse a type-id production
///
/// type-id:
///         type-specifier-seq abstract-declarator(opt)
///TODO: handle abstract-declarator
bool
Parser::parse_type_id (string &a_result)
{
    bool status=false;
    string result, str;
    shared_ptr<TypeSpecifier> type_spec;

    unsigned mark = LEXER.get_token_stream_mark ();
    if (!parse_type_specifier (type_spec)) {goto error;}
    type_spec->to_string (result);
    result += " ";

    type_spec.reset ();
    while (parse_type_specifier (type_spec)) {
        type_spec->to_string (str);
        result += str;
        type_spec.reset ();
    }
    //TODO:handle abstract-declarator here.
    goto okay;

error:
    LEXER.rewind_to_mark (mark);
    status=false;
    goto out;

okay:
    a_result = result;
    status=true;
    

out:
    return status;

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
Parser::parse_decl_specifier (shared_ptr<DeclSpecifier> &a_result)
{
    bool status=false;
    Token token;
    shared_ptr<TypeSpecifier> type_spec (new TypeSpecifier);
    shared_ptr<DeclSpecifier> result (new DeclSpecifier);

    unsigned mark = LEXER.get_token_stream_mark ();

    if (!LEXER.peek_next_token (token)) {goto error;}

    if (token.get_kind () == Token::KEYWORD) {
        if (token.get_str_value () == "auto") {
            result->set_kind (DeclSpecifier::AUTO);
        } else if (token.get_str_value () == "register") {
            result->set_kind (DeclSpecifier::REGISTER);
        } else if (token.get_str_value () == "static") {
            result->set_kind (DeclSpecifier::STATIC);
        } else if (token.get_str_value () == "extern") {
            result->set_kind (DeclSpecifier::EXTERN);
        } else if (token.get_str_value () == "mutable") {
            result->set_kind (DeclSpecifier::MUTABLE);
        } else if (token.get_str_value () == "friend") {
            result->set_kind (DeclSpecifier::FRIEND);
        } else if (token.get_str_value () == "typedef") {
            result->set_kind (DeclSpecifier::TYPEDEF);
        }
        LEXER.consume_next_token ();
        goto okay;
    }
    if (parse_type_specifier (type_spec)) {
        result->set_type_specifier (type_spec);
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
Parser::parse_decl_specifier_seq (list<shared_ptr<DeclSpecifier> > &a_result)
{
    string str;
    list<shared_ptr<DeclSpecifier> > result;
    shared_ptr<DeclSpecifier> decl, decl2;
    bool is_ok=false, watch_nb_types=false;
    unsigned mark = LEXER.get_token_stream_mark ();

    unsigned mark2=0;
    while (true ) {
        mark2 = LEXER.get_token_stream_mark ();
        if (!parse_decl_specifier (decl) || !decl) {
            break;
        }
        //type specifier special cases
        if (decl->get_type_specifier ()) {
            //— const or volatile can be combined with any other type-specifier.
            //  However, redundant cv- qualifiers are prohibited except
            if (decl->get_type_specifier ()->is_cv_qualifier ()) {
                if (!parse_decl_specifier (decl2)) {goto error;}
                if (decl2->get_type_specifier ()) {
                    if (decl2->get_type_specifier ()->is_cv_qualifier ()) {
                        goto error;
                    }
                    watch_nb_types=true;
                    result.push_back (decl);
                    result.push_back (decl2);
                }
            } else if (decl->get_type_specifier ()->get_kind ()
                       == TypeSpecifier::SIMPLE) {
                //— signed or unsigned can be combined with char, long, short, or int.
                //— short or long can be combined with int.
                //— long can be combined with double.
                //TODO: handle this
                if (watch_nb_types) {
                    LEXER.rewind_to_mark (mark2);
                    break;
                }
                watch_nb_types=true;
                result.push_back (decl);
            } else {
                if (watch_nb_types) {
                    LEXER.rewind_to_mark (mark2);
                    break;
                }
                watch_nb_types=true;
                result.push_back (decl);
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
Parser::parse_declarator_id (string &a_result)
{
    bool status=false;
    string result, str;
    Token token;

    unsigned mark = LEXER.get_token_stream_mark ();

    shared_ptr<IDExpr> expr;
    if (parse_id_expr (expr)) {
        if (!expr) {
            goto error;
        }
        expr->to_string (result);
        goto okay;
    }

    if (!LEXER.consume_next_token (token)) {goto error;}
    if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        result += "::";
    }
    if (parse_nested_name_specifier (str)) {
        result += str + " ";
    }
    if (parse_type_name (str)) {
        result += str;
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
///            direct-declarator [ constant-expressionopt ]
///            ( declarator )
/// TODO: handle function style direct-declarator
/// TODO: handle subscript style direct-declarator
/// TODO: handle function pointer style direct declarator
bool
Parser::parse_direct_declarator (string &a_result)
{
    bool status=false;
    string result;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (!parse_declarator_id (result)) {goto error;}

    //TODO: handle function style direct-declarator
    // TODO: handle subscript style direct-declarator
    // TODO: handle function pointer style direct declarator
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
Parser::parse_cv_qualifier (string &a_result)
{
    bool status=false;
    string result;
    Token token;

    if (!LEXER.consume_next_token (token)) {goto error;}

    if (token.get_kind () == Token::KEYWORD
        && (token.get_str_value () == "const"
            || token.get_str_value () == "volatile")) {
        result += token.get_str_value ();
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

bool
Parser::parse_cv_qualifier_seq (string & a_result)

{
    bool status=false;
    string result, str;

    unsigned mark = LEXER.get_token_stream_mark ();

    while (parse_cv_qualifier_seq (str)) {
        result += str + " ";
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
Parser::parse_ptr_operator (string &a_result)
{
    bool status=false;
    string result, str;
    Token token;

    unsigned mark = LEXER.get_token_stream_mark ();

start:
    if (!LEXER.consume_next_token (token)) {goto error;}
    if (token.get_kind () == Token::OPERATOR_BIT_AND) {
        result += "&";
        goto okay;
    } else if (token.get_kind () == Token::OPERATOR_MULT) {
        result += "*";
        if (!parse_cv_qualifier_seq (str)) {goto error;}
        result += str;
        goto okay;
    } else if (token.get_kind () == Token::OPERATOR_SCOPE_RESOL) {
        result += "::";
    }
    if (parse_nested_name_specifier (str)) {
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
Parser::parse_declarator (string &a_result)
{
    if (parse_direct_declarator (a_result)) {
        return true;
    }
    if (parse_ptr_operator (a_result)) {
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
Parser::parse_init_declarator (string &a_result)
{

    if (parse_declarator (a_result)) {
        return true;
    }
    return false;
}

/// parse an init-declarator-list production.
///
/// init-declarator-list:
///           init-declarator
///           init-declarator-list , init-declarator
bool
Parser::parse_init_declarator_list (string &a_result)
{
    bool status=false;
    string result, str;
    Token token;

    unsigned mark = LEXER.get_token_stream_mark ();

    if (parse_init_declarator (str)) {
        result = str ;
    } else {
        goto error;
    }

loop:
    if (LEXER.peek_next_token (token)
        && token.get_kind () == Token::OPERATOR_SEQ_EVAL) {
        if (parse_init_declarator (str)) {
            result += ", " + str;
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
Parser::parse_simple_declaration (string &a_result)
{
    list<shared_ptr<DeclSpecifier> >  decls;
    string result, str;

    if (!parse_decl_specifier_seq (decls))
        return true;
    DeclSpecifier::list_to_string (decls, str);

    result = str;

    if (parse_init_declarator_list (str)) {
        result += " " + str;
    }

    a_result = result;
    return true;
}

NEMIVER_END_NAMESPACE (nemiver)
NEMIVER_END_NAMESPACE (cpp)
