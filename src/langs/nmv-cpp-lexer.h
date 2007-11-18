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
#ifndef __NEMIVER_CPP_LEXER_H__
#define __NEMIVER_CPP_LEXER_H__

#include <string>
#include "common/nmv-namespace.h"
#include "common/nmv-api-macros.h"

using std::string ;

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

class NEMIVER_API Token {
public:
    enum Kind {
        UNDEFINED=0,
        IDENTIFIER,
        KEYWORD,
        INTEGER_LITERAL,
        CHARACTER_LITERAL,
        FLOATING_LITERAL,
        STRING_LITERAL,
        BOOLEAN_LITERAL,
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

class NEMIVER_API Lexer {
    Lexer (const Lexer &) ;
    Lexer& operator=(const Lexer&) ;
    struct Priv;
    Priv *m_priv ;

private:
    /// \name token scanning helpers
    /// @{

    bool next_is (const char *a_char_seq) const;
    void skip_blanks ();
    bool is_nondigit (const char a_in) const;
    bool is_digit (const char a_in) const;
    bool is_nonzero_digit (const char a_in) const;
    bool is_octal_digit (const char a_in) const;
    bool is_hexadecimal_digit (const char) const ;
    int  hexadigit_to_decimal (const char) const;

    bool scan_decimal_literal (string &a_result) ;
    bool scan_octal_literal (string &a_result) ;
    bool scan_hexadecimal_literal (string &a_result) ;
    bool scan_hexquad (string &a_result) ;
    bool scan_integer_suffix (string &a_result) ;
    bool scan_integer_literal (string &a_result) ;
    bool scan_simple_escape_sequence (int &a_result) ;
    bool scan_octal_escape_sequence (int &a_result) ;
    bool scan_hexadecimal_escape_sequence (int &a_result) ;
    bool scan_escape_sequence (int &a_result) ;
    bool scan_hexquad (int &a_result) ;
    bool scan_universal_character_name (int &a_result) ;
    bool scan_c_char (int &a_result) ;
    bool scan_c_char_sequence (string &a_result) ;
    bool scan_s_char (int &a_result) ;
    bool scan_s_char_sequence (string &a_result) ;
    bool scan_character_literal (string &a_result) ;
    bool scan_digit_sequence (string &a_result) ;
    bool scan_fractional_constant (string &a_result) ;
    bool scan_exponent_part (string &a_result) ;
    bool scan_floating_literal (string &a_result,
                                string &a_exponent) ;
    bool scan_string_literal (string &a_result) ;
    bool scan_boolean_literal (bool &a_result) ;
    /// @}
public:

    Lexer (const string &a_in) ;
    ~Lexer () ;

    /// \scanning methods
    /// \@{
    bool scan_identifier (Token &a_token) ;
    bool scan_keyword (Token &a_token) ;
    bool scan_literal (Token &a_token) ;
    bool scan_operator (Token &a_token) ;
    bool scan_punctuator (Token &a_token) ;
    /// \@}

    bool get_next_token (Token &a_token) ;
};//end class Lexer

NEMIVER_END_NAMESPACE (cpp)
NEMIVER_END_NAMESPACE (nemiver)

#endif /*__NEMIVER_CPP_LEXER_H__*/

