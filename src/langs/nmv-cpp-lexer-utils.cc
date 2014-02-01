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
#include "nmv-cpp-lexer-utils.h"
#include "common/nmv-ustring.h"

using nemiver::common::UString;

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

bool
token_type_as_string (const Token &a_token, std::string &a_out)
{
    switch (a_token.get_kind ()) {
        case Token::UNDEFINED:
            a_out = "UNDEFINED";
            break;
        case Token::IDENTIFIER:
            a_out = "IDENTIFIER";
            break;
        case Token::KEYWORD:
            a_out = "KEYWORD";
            break;
        case Token::INTEGER_LITERAL:
            a_out = "INTEGER_LITERAL";
            break;
        case Token::CHARACTER_LITERAL:
            a_out = "CHARACTER_LITERAL";
            break;
        case Token::FLOATING_LITERAL:
            a_out = "FLOATING_LITERAL";
            break;
        case Token::STRING_LITERAL:
            a_out = "STRING_LITERAL";
            break;
        case Token::BOOLEAN_LITERAL:
            a_out = "BOOLEAN_LITERAL";
            break;
        case Token::OPERATOR_NEW:
            a_out = "OPERATOR_NEW";
            break;
        case Token::OPERATOR_DELETE:
            a_out = "OPERATOR_DELETE";
            break;
        case Token::OPERATOR_NEW_VECT:
            a_out = "OPERATOR_NEW_VECT";
            break;
        case Token::OPERATOR_DELETE_VECT:
            a_out = "OPERATOR_DELETE_VECT";
            break;
        case Token::OPERATOR_PLUS:
            a_out="OPERATOR_PLUS";
            break;
        case Token::OPERATOR_MINUS:
            a_out="OPERATOR_MINUS";
            break;
        case Token::OPERATOR_MULT:
            a_out="OPERATOR_MULT";
            break;
        case Token::OPERATOR_DIV:
            a_out="OPERATOR_DIV";
            break;
        case Token::OPERATOR_MOD:
            a_out="OPERATOR_MOD";
            break;
        case Token::OPERATOR_BIT_XOR:
            a_out="OPERATOR_BIT_XOR";
            break;
        case Token::OPERATOR_BIT_AND:
            a_out="OPERATOR_BIT_AND";
            break;
        case Token::OPERATOR_BIT_OR:
            a_out="OPERATOR_BIT_OR";
            break;
        case Token::OPERATOR_BIT_COMPLEMENT:
            a_out="OPERATOR_BIT_COMPLEMENT";
            break;
        case Token::OPERATOR_NOT:
            a_out="OPERATOR_NOT";
            break;
        case Token::OPERATOR_ASSIGN:
            a_out="OPERATOR_NOT";
            break;
        case Token::OPERATOR_LT:
            a_out="OPERATOR_LT";
            break;
        case Token::OPERATOR_GT:
            a_out="OPERATOR_GT";
            break;
        case Token::OPERATOR_PLUS_EQ:
            a_out="OPERATOR_PLUS_EQ";
            break;
        case Token::OPERATOR_MINUS_EQ:
            a_out="OPERATOR_MINUS_EQ";
            break;
        case Token::OPERATOR_MULT_EQ:
            a_out="OPERATOR_MULT_EQ";
            break;
        case Token::OPERATOR_DIV_EQ:
            a_out="OPERATOR_DIV_EQ";
            break;
        case Token::OPERATOR_MOD_EQ:
            a_out="OPERATOR_MOD_EQ";
            break;
        case Token::OPERATOR_BIT_XOR_EQ:
            a_out="OPERATOR_BIT_XOR_EQ";
            break;
        case Token::OPERATOR_BIT_AND_EQ:
            a_out="OPERATOR_BIT_AND_EQ";
            break;
        case Token::OPERATOR_BIT_OR_EQ:
            a_out="OPERATOR_BIT_OR_EQ";
            break;
        case Token::OPERATOR_BIT_LEFT_SHIFT:
            a_out="OPERATOR_BIT_LEFT_SHIFT";
            break;
        case Token::OPERATOR_BIT_RIGHT_SHIFT:
            a_out="OPERATOR_BIT_RIGHT_SHIFT";
            break;
        case Token::OPERATOR_BIT_LEFT_SHIFT_EQ:
            a_out="OPERATOR_BIT_LEFT_SHIFT_EQ";
            break;
        case Token::OPERATOR_BIT_RIGHT_SHIFT_EQ:
            a_out="OPERATOR_BIT_RIGHT_SHIFT_EQ";
            break;
        case Token::OPERATOR_EQUALS:
            a_out="OPERATOR_EQUALS";
            break;
        case Token::OPERATOR_NOT_EQUAL:
            a_out="OPERATOR_NOT_EQUAL";
            break;
        case Token::OPERATOR_LT_EQ:
            a_out="OPERATOR_LT_EQ";
            break;
        case Token::OPERATOR_GT_EQ:
            a_out="OPERATOR_GT_EQ";
            break;
        case Token::OPERATOR_AND:
            a_out="OPERATOR_AND";
            break;
        case Token::OPERATOR_OR:
           a_out="OPERATOR_OR";
            break;
        case Token::OPERATOR_PLUS_PLUS:
            a_out="OPERATOR_PLUS_PLUS";
            break;
        case Token::OPERATOR_MINUS_MINUS:
            a_out="OPERATOR_MINUS_MINUS";
            break;
        case Token::OPERATOR_SEQ_EVAL:
            a_out="OPERATOR_SEQ_EVAL";
            break;
        case Token::OPERATOR_ARROW_STAR:
            a_out="OPERATOR_ARROR_STAR";
            break;
        case Token::OPERATOR_DEREF:
            a_out="OPERATOR_DEREF";
            break;
        case Token::OPERATOR_GROUP:
            a_out="OPERATOR_GROUP";
            break;
        case Token::OPERATOR_ARRAY_ACCESS:
            a_out="OPERATOR_ARRAY_ACCESS";
            break;
        case Token::OPERATOR_SCOPE_RESOL:
            a_out="OPERATOR_SCOPE_RESOL";
            break;
        case Token::OPERATOR_DOT:
            a_out="OPERATOR_DOT";
            break;
        case Token::OPERATOR_DOT_STAR:
            a_out="OPERATOR_DOT_STAR";
            break;
        case Token::PUNCTUATOR_COLON:
            a_out="PUNCTUATOR_COLON";
            break;
        case Token::PUNCTUATOR_SEMI_COLON:
            a_out="PUNCTUATOR_SEMI_COLON";
            break;
        case Token::PUNCTUATOR_CURLY_BRACKET_OPEN:
            a_out="PUNCTUATOR_CURLY_BRACKET_OPEN";
            break;
        case Token::PUNCTUATOR_CURLY_BRACKET_CLOSE:
            a_out="PUNCTUATOR_CURLY_BRACKET_CLOSE";
            break;
        case Token::PUNCTUATOR_BRACKET_OPEN:
            a_out="PUNCTUATOR_BRACKET_OPEN";
            break;
        case Token::PUNCTUATOR_BRACKET_CLOSE:
            a_out="PUNCTUATOR_BRACKET_CLOSE";
            break;
        case Token::PUNCTUATOR_PARENTHESIS_OPEN:
            a_out="PUNCTUATOR_PARENTHESIS_OPEN";
            break;
        case Token::PUNCTUATOR_PARENTHESIS_CLOSE:
            a_out="PUNCTUATOR_PARENTHESIS_CLOSE";
            break;
        case Token::PUNCTUATOR_QUESTION_MARK:
            a_out="PUNCTUATOR_QUESTION_MARK";
            break;
        default:
            a_out="UNKNOWN_TOKEN";
            return false;
    }
    return true;
}

bool
token_as_string (const Token &a_token, std::string &a_out)
{
    token_type_as_string (a_token, a_out);
    switch (a_token.get_kind ()) {
        case Token::IDENTIFIER:
        case Token::KEYWORD:
        case Token::INTEGER_LITERAL:
        case Token::FLOATING_LITERAL:
        case Token::STRING_LITERAL:
            a_out += ":" + a_token.get_str_value ();
            break;
        case Token::BOOLEAN_LITERAL:
            a_out += ":" + UString::from_int (a_token.get_int_value ()).raw ();
            break;
        default:
            break;
    }
    return true;
}

std::ostream&
operator<< (std::ostream &a_out, const Token &a_token)
{
    std::string str;
    token_as_string (a_token, str);
    a_out << str;
    return a_out;
}

NEMIVER_END_NAMESPACE (cpp)
NEMIVER_END_NAMESPACE (nemiver)

