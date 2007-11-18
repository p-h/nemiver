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
#include "nmv-cpp-lexer.h"

NEMIVER_BEGIN_NAMESPACE (nemiver)
NEMIVER_BEGIN_NAMESPACE (cpp)

//********************
//<class Token implem>
//********************
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
//********************
//</class Token implem>
//********************

//********************
//<class Lexer implem>
//********************
struct Lexer::Priv {
    string input ;
    string::size_type cursor ;

    Priv (const string &a_in) :
        input (a_in), cursor (0)
    {
    }

    Priv () :
        cursor (0)
    {
    }
};//end struct Lexer::Priv

#define CUR m_priv->cursor

#define INPUT m_priv->input

#define CUR_CHAR INPUT[CUR]

#define IN_BOUNDS(cursor) \
((cursor) < m_priv->input.size ())

#define CURSOR_IN_BOUNDS (CUR < INPUT.size ())

#define CHECK_CURSOR_BOUNDS \
if (!IN_BOUNDS (CUR)) {return false;}

#define CHECK_CURSOR_BOUNDS2 \
if (!IN_BOUNDS (CUR)) {return;}

#define SCAN_KEYWORD(len,key) \
((!(INPUT.compare (CUR, len, key)) and (key_length=len)))

#define RECORD_POSITION unsigned saved_cur=CUR

#define RESTORE_POSITION CUR=saved_cur

#define MOVE_FORWARD(nb) CUR += nb

#define MOVE_FORWARD_AND_CHECK(nb) {CUR+=nb;if (!IN_BOUNDS(CUR)) {goto error;}}

#define CONSUME_CHAR MOVE_FORWARD(1)

#define CONSUME_CHAR_AND_CHECK {MOVE_FORWARD(1);if (!CURSOR_IN_BOUNDS) {goto error;}}

Lexer::Lexer (const string &a_in)
{
    m_priv = new Lexer::Priv (a_in) ;
}

Lexer::~Lexer ()
{
    if (m_priv) {
        delete m_priv ;
        m_priv = NULL ;
    }
}

bool
Lexer::next_is (const char *a_char_seq) const
{
    CHECK_CURSOR_BOUNDS

    if (!a_char_seq)
        return false ;

    int len = strlen (a_char_seq) ;
    if (!len)
        return false ;
    if (IN_BOUNDS (CUR + len - 1) && !INPUT.compare (CUR, len, a_char_seq)) {
        return true ;
    }
    return false ;
}

void
Lexer::skip_blanks ()
{
    CHECK_CURSOR_BOUNDS2;

    while (CURSOR_IN_BOUNDS && isblank (CUR_CHAR)) {
        MOVE_FORWARD (1) ;
    }
}

bool
Lexer::is_nondigit (const char a_in) const
{
    if (a_in == '_' ||
        (a_in >= 'A' && a_in <= 'Z') ||
        (a_in >= 'a' && a_in <= 'z')) {
        return true ;
    }
    return false ;
}

bool
Lexer::is_digit (const char a_in) const
{
    if (a_in >= '0' and a_in <= '9')
        return true ;
    return false ;
}

bool
Lexer::is_nonzero_digit (const char a_in) const
{
    if (a_in >= '1' and a_in <= '9')
        return true ;
    return false ;
}

bool
Lexer::is_octal_digit (const char a_in) const
{
    if (a_in >= '0' && a_in <= '7')
        return true ;
    return false ;
}

bool
Lexer::is_hexadecimal_digit (const char a_in) const
{
    if (is_digit (a_in) ||
        (a_in >= 'a' && a_in <= 'f') ||
        (a_in >= 'A' && a_in <= 'F')) {
        return true ;
    }
    return false ;
}

int
Lexer::hexadigit_to_decimal (const char a_hexa) const
{
    if (a_hexa >= '0' && a_hexa <= '9') {
        return a_hexa - '0' ;
    } else if (a_hexa >= 'a' && a_hexa <= 'f') {
        return 10 + a_hexa - 'a' ;
    } else if (a_hexa >= 'A' && a_hexa <= 'F') {
        return 10 + a_hexa - 'A' ;
    }
    return -1 ;
}

bool
Lexer::scan_decimal_literal (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string result ;
    if (is_nonzero_digit (CUR_CHAR)) {
        result += CUR_CHAR;
        CONSUME_CHAR;
    } else {
        goto error;
    }
    while (CURSOR_IN_BOUNDS && is_digit (CUR_CHAR)) {
        result += CUR_CHAR ;
        CONSUME_CHAR;
    }
    a_result = result ;
    return true;

error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::scan_octal_literal (string &a_result)
{
    CHECK_CURSOR_BOUNDS ;
    RECORD_POSITION;

    string result ;
    if (CUR_CHAR != '0')
        goto error;

    result += CUR_CHAR ;
    CONSUME_CHAR;

    while (CURSOR_IN_BOUNDS && is_octal_digit (CUR_CHAR)) {
        result += CUR_CHAR;
        CONSUME_CHAR;
    }
    a_result = result ;
    return true ;

error:
    RESTORE_POSITION;
    return false;
}

bool
Lexer::scan_hexadecimal_literal (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string result ;
    if (IN_BOUNDS (CUR+1)
        && INPUT[CUR] == '0'
        && (INPUT[CUR+1] == 'x' || INPUT[CUR+1] == 'X')) {
        MOVE_FORWARD_AND_CHECK (2);
    }
    while (CURSOR_IN_BOUNDS && is_hexadecimal_digit (CUR_CHAR)) {
        result +=  CUR_CHAR ;
        CONSUME_CHAR;
    }
    if (result.empty ()) {
        goto error;
    }
    a_result = result ;
    return true;

error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::scan_escape_sequence (int &a_result)
{
    CHECK_CURSOR_BOUNDS

    if (scan_simple_escape_sequence (a_result) ||
        scan_octal_escape_sequence (a_result)  ||
        scan_hexadecimal_escape_sequence (a_result)) {
        return true ;
    }
    return false ;
}

bool
Lexer::scan_universal_character_name (int &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    if (!IN_BOUNDS (CUR+5) ||
        INPUT[CUR] != '\\' ||
        (INPUT[CUR+1] != 'U' && INPUT[CUR+1] != 'u')) {
        return false ;
    }
    MOVE_FORWARD_AND_CHECK (2) ;
    if (!scan_hexquad (a_result)) {
        goto error;
    }
    return true ;

error:
    RESTORE_POSITION;
    return false;
}

bool
Lexer::scan_c_char (int &a_result)
{
    CHECK_CURSOR_BOUNDS

    if (CUR_CHAR != '\\' && CUR_CHAR != '\'' && CUR_CHAR != '\n') {
        a_result = CUR_CHAR ;
        CONSUME_CHAR;
        return true ;
    } else {
        if (scan_escape_sequence (a_result) ||
            scan_universal_character_name (a_result)) {
            return true ;
        }
    }
    return false ;
}

bool
Lexer::scan_c_char_sequence (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    int c=0 ;

    if (!scan_c_char (c)) {
        return false ;
    }
    a_result = c ;
    while (CURSOR_IN_BOUNDS && scan_c_char (c)) {
        a_result += c ;
    }
    return true ;
}

bool
Lexer::scan_s_char (int &a_result)
{
    CHECK_CURSOR_BOUNDS

    if (CUR_CHAR != '\\' && CUR_CHAR != '"' && CUR_CHAR != '\n') {
        a_result = CUR_CHAR ;
        CONSUME_CHAR;
        return true ;
    } else {
        if (scan_escape_sequence (a_result) ||
            scan_universal_character_name (a_result)) {
            return true ;
        }
    }
    return false ;
}

bool
Lexer::scan_s_char_sequence (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    int c=0 ;

    if (!scan_s_char (c)) {
        return false ;
    }
    a_result = c ;
    while (CURSOR_IN_BOUNDS && scan_s_char (c)) {
        a_result += c ;
    }
    return true ;
}

bool
Lexer::scan_character_literal (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string result;
    if (CUR_CHAR == 'L') {
        CONSUME_CHAR_AND_CHECK;
    }
    if (CUR_CHAR == '\'') {
        CONSUME_CHAR_AND_CHECK;
    } else {
        goto error;
    }
    if (!scan_c_char_sequence (result)) {
        goto error;
    }
    if (CUR_CHAR == '\'') {
        CONSUME_CHAR;
    } else {
        goto error;
    }
    a_result = result;
    return true ;

error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::scan_digit_sequence (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string result ;
    while (CURSOR_IN_BOUNDS && is_digit (CUR_CHAR)) {
        result += CUR_CHAR ;
        CONSUME_CHAR;
    }
    if (result.empty ()) {
        goto error;
    }

    a_result = result ;
    return true ;

error:
    RESTORE_POSITION;
    return false;
}

bool
Lexer::scan_fractional_constant (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string left, right;

    scan_digit_sequence (left) ;
    if (CUR_CHAR != '.') {
        goto error ;
    }
    CONSUME_CHAR_AND_CHECK;
    if (!scan_digit_sequence (right) && left.empty ()) {
        goto error ;
    }
    a_result = left + "." + right ;
    return true ;

error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::scan_exponent_part (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string result, sign ;
    if (CUR_CHAR != 'e' && CUR_CHAR != 'E') {
        goto error ;
    }
    CONSUME_CHAR_AND_CHECK;
    if (INPUT[CUR] == '-' || INPUT[CUR] == '+') {
        sign = INPUT[CUR] ;
        CONSUME_CHAR_AND_CHECK;
    }
    if (!scan_digit_sequence (result)) {
        goto error ;
    }
    a_result = sign + result ;
    return true ;

error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::scan_floating_literal (string &a_result,
                                 string &a_exponent)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string fract, exp;

    if (scan_fractional_constant (fract)) {
        scan_exponent_part (exp) ;
        if (CUR_CHAR == 'f' || CUR_CHAR == 'F'
            || CUR_CHAR == 'L' || CUR_CHAR == 'l') {
            CONSUME_CHAR_AND_CHECK;
        }
    } else if (scan_digit_sequence (fract)) {
        if (!scan_exponent_part (exp)) {
            goto error;
        }
        if (CUR_CHAR == 'f' || CUR_CHAR == 'F'
            || CUR_CHAR == 'L' || CUR_CHAR == 'l') {
            CONSUME_CHAR;
        }
    } else {
        goto error;
    }

    a_result = fract;
    a_exponent = exp;
    return true ;

error:
    RESTORE_POSITION;
    return false;

}

bool
Lexer::scan_string_literal (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string result ;
    if (CUR_CHAR == 'L') {
        CONSUME_CHAR_AND_CHECK;
    }
    if (CUR_CHAR == '"') {
        CONSUME_CHAR_AND_CHECK;
    } else {
        goto error;
    }
    if (!scan_s_char_sequence (result)) {
        goto error ;
    }
    if (CUR_CHAR == '"') {
        CONSUME_CHAR;
    } else {
        goto error;
    }
    a_result = result ;
    return true ;

error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::scan_boolean_literal (bool &a_result)
{
    CHECK_CURSOR_BOUNDS

    if (IN_BOUNDS (CUR+4)
        && INPUT[CUR] == 'f'
        && INPUT[CUR+1] == 'a'
        && INPUT[CUR+2] == 'l'
        && INPUT[CUR+3] == 's'
        && INPUT[CUR+4] == 'e') {
        MOVE_FORWARD (4);
        a_result = false ;
        return true ;
    } else if (IN_BOUNDS (CUR+3)
               && INPUT[CUR] == 't'
               && INPUT[CUR+1] == 'r'
               && INPUT[CUR+2] == 'u'
               && INPUT[CUR+3] == 'e') {
        MOVE_FORWARD (3);
        a_result = true ;
        return true ;
    }
    return false ;
}

bool
Lexer::scan_integer_suffix (string &a_result)
{
    CHECK_CURSOR_BOUNDS
    RECORD_POSITION;

    string result ;
    if (CUR_CHAR == 'u' or CUR_CHAR == 'U') {
        result += CUR_CHAR;
        CONSUME_CHAR_AND_CHECK;
        if (CUR_CHAR == 'l' or CUR_CHAR == 'L') {
            result += CUR_CHAR;
            CONSUME_CHAR;
        }
    } else if (CUR_CHAR == 'L' or CUR_CHAR == 'L') {
        result += CUR_CHAR;
        CONSUME_CHAR_AND_CHECK;
        if (CUR_CHAR == 'u' or CUR_CHAR == 'U') {
            result += CUR_CHAR ;
            CONSUME_CHAR;
        }
    } else {
        goto error;
    }
    if (result.empty ()) {
        goto error;
    }
    a_result = result ;
    return true ;

error:
    RESTORE_POSITION;
    return false;
}

bool
Lexer::scan_integer_literal (string &a_result)
{
    CHECK_CURSOR_BOUNDS

    string literal, tmp ;
    if (is_nonzero_digit (CUR_CHAR)) {
        if (!scan_decimal_literal (literal)) {
            return false ;
        }
        if (CUR_CHAR == 'l' or
            CUR_CHAR == 'L' or
            CUR_CHAR == 'u' or
            CUR_CHAR == 'U') {
            if (scan_integer_suffix (tmp)) {
                literal += tmp ;
            }
        }
    } else if (IN_BOUNDS (CUR+1) and CUR_CHAR == '0' and
               (INPUT[CUR+1] == 'x' or INPUT[CUR+1] == 'X')) {
        if (!scan_hexadecimal_literal (literal)) {
            return false ;
        }
    } else if (CUR_CHAR == '0') {
        if (!scan_octal_literal (literal)) {
            return false ;
        }
    } else {
        return false ;
    }
    a_result = literal ;
    return true ;
}

bool
Lexer::scan_simple_escape_sequence (int &a_result)
{
    CHECK_CURSOR_BOUNDS;
    RECORD_POSITION;

    if (CUR_CHAR != '\\')
        return false ;

    CONSUME_CHAR_AND_CHECK;

    switch (CUR_CHAR) {
        case '\'':
            a_result = '\\' ;
            break ;
        case '"' :
            a_result = '"' ;
            break ;
        case '?':
            a_result = '?' ;
            break ;
        case '\\':
            a_result = '\\' ;
            break ;
        case 'a' :
            a_result = '\a';
            break ;
        case 'b' :
            a_result = '\b';
            break ;
        case 'f' :
            a_result = '\f';
            break ;
        case 'n' :
            a_result = '\n';
            break ;
        case 'r' :
            a_result = '\r' ;
            break ;
        case 't' :
            a_result = '\t' ;
            break ;
        case 'v':
            a_result = '\v' ;
            break ;
        default:
            goto error;
    }
    CONSUME_CHAR;
    return true ;

error:
    RESTORE_POSITION;
    return false;
}

bool
Lexer::scan_octal_escape_sequence (int &a_result)
{
    CHECK_CURSOR_BOUNDS ;
    unsigned cur=CUR;
    int result=0 ;

    if (!IN_BOUNDS (cur+1) || INPUT[cur] != '\\' || !is_octal_digit (INPUT[cur+1])) {
        return false ;
    }
    ++cur ;
    result = INPUT[CUR] - '0' ;
    ++cur ;
    if (IN_BOUNDS (cur) && is_octal_digit (INPUT[cur])) {
        result = 8*result + (INPUT[cur] - '0') ;

        ++cur ;
        if (IN_BOUNDS (cur) && is_octal_digit (INPUT[cur])) {
            result = 8*result + (INPUT[cur] - '0') ;
            ++cur ;
        }
    }

    CUR = cur ;
    a_result = result ;
    return true ;
}

bool
Lexer::scan_hexadecimal_escape_sequence (int &a_result)
{
    CHECK_CURSOR_BOUNDS ;
    unsigned cur=CUR;

    if (!IN_BOUNDS (cur+1) || INPUT[cur] != '\\' ||
        !is_hexadecimal_digit (INPUT[cur+1])) {
        return false ;
    }
    ++cur ;
    a_result = INPUT[cur] ;
    ++cur ;
    while (IN_BOUNDS (cur) && is_hexadecimal_digit (INPUT[cur])) {
        a_result = 16*a_result + hexadigit_to_decimal (INPUT[cur]) ;
        ++cur ;
    }
    CUR = cur ;
    return true ;
}

bool
Lexer::scan_hexquad (int &a_result)
{
    CHECK_CURSOR_BOUNDS ;
    unsigned cur=CUR;

    if (!IN_BOUNDS (cur+3)) {
        return false ;
    }
    if (is_hexadecimal_digit (cur) &&
        is_hexadecimal_digit (cur+1) &&
        is_hexadecimal_digit (cur+2) &&
        is_hexadecimal_digit (cur+3)) {
        a_result = INPUT[cur] ;
        a_result = 16*a_result + hexadigit_to_decimal (INPUT[cur+1]) ;
        a_result = 16*a_result + hexadigit_to_decimal (INPUT[cur+2]) ;
        a_result = 16*a_result + hexadigit_to_decimal (INPUT[cur+3]) ;
        CUR = cur+4 ;
        return true ;
    }
    return false ;
}

bool
Lexer::scan_identifier (Token &a_token)
{
    CHECK_CURSOR_BOUNDS;
    string identifier;
    RECORD_POSITION;

    if (!is_nondigit (CUR_CHAR)) {
        goto error;
    }
    identifier += CUR_CHAR;
    CONSUME_CHAR;
    while (CURSOR_IN_BOUNDS && (is_nondigit (CUR_CHAR) || is_digit (CUR_CHAR))) {
        identifier += CUR_CHAR ;
        CONSUME_CHAR;
    }
    if (identifier.empty ()) {
        goto error;
    }
    a_token.set (Token::IDENTIFIER, identifier) ;
    return true ;

error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::scan_keyword (Token &a_token)
{
    CHECK_CURSOR_BOUNDS
    int key_length=0 ;

    if (SCAN_KEYWORD (3, "and") or
        SCAN_KEYWORD (6, "and_eq") or
        SCAN_KEYWORD (3, "asm")  or
        SCAN_KEYWORD (4, "auto")  or
        SCAN_KEYWORD (6, "bitand") or
        SCAN_KEYWORD (5, "bitor") or
        SCAN_KEYWORD (4, "bool") or
        SCAN_KEYWORD (5, "break") or
        SCAN_KEYWORD (4, "case") or
        SCAN_KEYWORD (5, "catch") or
        SCAN_KEYWORD (4, "char") or
        SCAN_KEYWORD (5, "class") or
        SCAN_KEYWORD (5, "compl") or
        SCAN_KEYWORD (5, "const") or
        SCAN_KEYWORD (10, "const_cast") or
        SCAN_KEYWORD (8, "continue") or
        SCAN_KEYWORD (7, "default") or
        SCAN_KEYWORD (6, "delete") or
        SCAN_KEYWORD (2, "do") or
        SCAN_KEYWORD (6, "double") or
        SCAN_KEYWORD (12, "dynamic_cast") or
        SCAN_KEYWORD (4, "else") or
        SCAN_KEYWORD (4, "enum") or
        SCAN_KEYWORD (8, "explicit") or
        SCAN_KEYWORD (6, "export") or
        SCAN_KEYWORD (6, "extern") or
        SCAN_KEYWORD (5, "false") or
        SCAN_KEYWORD (5, "float") or
        SCAN_KEYWORD (3, "for") or
        SCAN_KEYWORD (6, "friend") or
        SCAN_KEYWORD (4, "goto") or
        SCAN_KEYWORD (2, "if") or
        SCAN_KEYWORD (6, "inline") or
        SCAN_KEYWORD (3, "int") or
        SCAN_KEYWORD (4, "long") or
        SCAN_KEYWORD (7, "mutable") or
        SCAN_KEYWORD (9, "namespace") or
        SCAN_KEYWORD (3, "new") or
        SCAN_KEYWORD (3, "not") or
        SCAN_KEYWORD (6, "not_eq") or
        SCAN_KEYWORD (8, "operator") or
        SCAN_KEYWORD (2, "or") or
        SCAN_KEYWORD (5, "or_eq") or
        SCAN_KEYWORD (7, "private") or
        SCAN_KEYWORD (9, "protected") or
        SCAN_KEYWORD (6, "public") or
        SCAN_KEYWORD (8, "register") or
        SCAN_KEYWORD (16, "reinterpret_cast") or
        SCAN_KEYWORD (6, "return") or
        SCAN_KEYWORD (5, "short") or
        SCAN_KEYWORD (6, "signed") or
        SCAN_KEYWORD (6, "sizeof") or
        SCAN_KEYWORD (6, "static") or
        SCAN_KEYWORD (11, "static_cast") or
        SCAN_KEYWORD (6, "struct") or
        SCAN_KEYWORD (6, "switch") or
        SCAN_KEYWORD (8, "template") or
        SCAN_KEYWORD (4, "this") or
        SCAN_KEYWORD (5, "throw") or
        SCAN_KEYWORD (4, "true") or
        SCAN_KEYWORD (3, "try") or
        SCAN_KEYWORD (7, "typedef") or
        SCAN_KEYWORD (6, "typeid") or
        SCAN_KEYWORD (8, "typename") or
        SCAN_KEYWORD (5, "union") or
        SCAN_KEYWORD (8, "unsigned") or
        SCAN_KEYWORD (5, "using") or
        SCAN_KEYWORD (7, "virtual") or
        SCAN_KEYWORD (4, "void") or
        SCAN_KEYWORD (8, "volatile") or
        SCAN_KEYWORD (7, "wchar_t") or
        SCAN_KEYWORD (5, "while") or
        SCAN_KEYWORD (3, "xor") or
        SCAN_KEYWORD (6, "xor_eq")) {
            string value = INPUT.substr (CUR, key_length) ;
            a_token.set (Token::KEYWORD, value) ;
            CUR += key_length ;
            return true ;
    }
    return false ;
}

bool
Lexer::scan_literal (Token &a_token)
{
    CHECK_CURSOR_BOUNDS
    string lit1, lit2;
    bool lit3=0;
    if (scan_character_literal (lit1)) {
        a_token.set (Token::CHARACTER_LITERAL, lit1) ;
    } else if (scan_integer_literal (lit1)) {
        a_token.set (Token::INTEGER_LITERAL, lit1) ;
    } else if (scan_floating_literal (lit1, lit2)) {
        a_token.set (Token::FLOATING_LITERAL, lit1, lit2) ;
    } else if (scan_string_literal (lit1)) {
        a_token.set (Token::STRING_LITERAL, lit1) ;
    } else if (scan_boolean_literal (lit3)) {
        a_token.set (Token::BOOLEAN_LITERAL, lit3) ;
    } else {
        return false ;
    }
    return true ;
}

bool
Lexer::scan_operator (Token &a_token)
{
    CHECK_CURSOR_BOUNDS;
    RECORD_POSITION;

    if (next_is ("new")) {
        MOVE_FORWARD (sizeof ("new"));
        skip_blanks ();
        if (next_is ("[]")) {
            MOVE_FORWARD (sizeof ("[]"));
            a_token.set (Token::OPERATOR_NEW_VECT);
        } else {
            a_token.set (Token::OPERATOR_NEW) ;
        }
    } else if (next_is ("delete")) {
        MOVE_FORWARD (sizeof ("delete"));
        skip_blanks ();
        if (next_is ("[]")) {
            MOVE_FORWARD (sizeof ("[]"));
            a_token.set (Token::OPERATOR_DELETE_VECT);
        } else {
            a_token.set (Token::OPERATOR_DELETE);
        }
    } else if (CUR_CHAR == '+') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_PLUS_EQ);
        } else if (CUR_CHAR == '+') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_PLUS_PLUS);
        } else {
            a_token.set (Token::OPERATOR_PLUS);
        }
    } else if (CUR_CHAR == '-') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_MINUS_EQ);
        } else if (CUR_CHAR == '-') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_MINUS_MINUS);
        } else if ('>') {
            CONSUME_CHAR;
            if (CUR_CHAR == '*') {
                CONSUME_CHAR;
                a_token.set (Token::OPERATOR_MEMBER_POINTER);
            } else {
                a_token.set (Token::OPERATOR_DEREF);
            }
        } else {
            a_token.set (Token::OPERATOR_MINUS);
        }
    } else if (CUR_CHAR == '*') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_MULT_EQ) ;
        } else {
            a_token.set (Token::OPERATOR_MULT) ;
        }
    } else if (CUR_CHAR ==  '/') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_DIV_EQ);
        } else {
            a_token.set (Token::OPERATOR_DIV) ;
        }
    } else if (CUR_CHAR == '%') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_MOD_EQ);
        } else {
            a_token.set (Token::OPERATOR_MOD);
        }
    } else if (CUR_CHAR == '^') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_BIT_XOR_EQ);
        } else {
            a_token.set (Token::OPERATOR_BIT_XOR);
        }
    } else if (CUR_CHAR == '&') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_BIT_AND_EQ);
        } else if (CUR_CHAR == '&') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_AND);
        } else {
            a_token.set (Token::OPERATOR_BIT_AND);
        }
    } else if (CUR_CHAR == '|') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_BIT_OR_EQ);
        } else if (CUR_CHAR == '|') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_OR);
        } else {
            a_token.set (Token::OPERATOR_BIT_OR);
        }
    } else if (CUR_CHAR == '~') {
        CONSUME_CHAR;
        a_token.set (Token::OPERATOR_BIT_COMPLEMENT);
    } else if (CUR_CHAR == '!') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_NOT_EQUAL);
        } else {
            a_token.set (Token::OPERATOR_NOT);
        }
    } else if (CUR_CHAR == '=') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_EQUALS);
        } else {
            a_token.set (Token::OPERATOR_ASSIGN);
        }
    } else if (CUR_CHAR == '<') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_LT_EQ);
        } else if (CUR_CHAR == '<') {
            CONSUME_CHAR;
            if (CUR_CHAR == '=') {
                CONSUME_CHAR;
                a_token.set (Token::OPERATOR_BIT_LEFT_SHIFT_EQ);
            } else {
                a_token.set (Token::OPERATOR_BIT_LEFT_SHIFT);
            }
        } else {
            a_token.set (Token::OPERATOR_LT);
        }
    } else if (CUR_CHAR == '>') {
        CONSUME_CHAR;
        if (CUR_CHAR == '=') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_GT_EQ);
        } else if (CUR_CHAR == '>') {
            CONSUME_CHAR;
            if (CUR_CHAR == '=') {
                CONSUME_CHAR;
                a_token.set (Token::OPERATOR_BIT_RIGHT_SHIFT_EQ);
            } else {
                a_token.set (Token::OPERATOR_BIT_RIGHT_SHIFT);
            }
        } else {
            a_token.set (Token::OPERATOR_GT);
        }
    } else if (CUR_CHAR == ',') {
        CONSUME_CHAR;
        a_token.set (Token::OPERATOR_SEQ_EVAL);
    } else if (CUR_CHAR == '(') {
        CONSUME_CHAR;
        if (CUR_CHAR == ')') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_GROUP);
        } else {
            goto error;
        }
    } else if (CUR_CHAR == '[') {
        CONSUME_CHAR;
        if (CUR_CHAR == ']') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_ARRAY_ACCESS);
        } else {
            goto error;
        }
    } else if (CUR_CHAR == '.') {
        CONSUME_CHAR;
        if (CUR_CHAR == '*') {
            CONSUME_CHAR;
            a_token.set (Token::OPERATOR_DOT_STAR);
        } else {
            a_token.set (Token::OPERATOR_DOT);
        }
    } else if (CUR_CHAR == ':') {
        CONSUME_CHAR;
        if (CUR_CHAR == ':') {
            a_token.set (Token::OPERATOR_SCOPE_RESOL) ;
        } else {
            goto error;
        }
    } else {
        goto error;
    }
    return true;

error:
    RESTORE_POSITION;
    return false;
}

bool
Lexer::scan_punctuator (Token &a_token)
{
    CHECK_CURSOR_BOUNDS;
    RECORD_POSITION;

    switch (CUR_CHAR) {
        case ':':
            CONSUME_CHAR;
            if (CUR_CHAR == ':') {
                goto error;
            } else {
                a_token.set (Token::PUNCTUATOR_COLON);
            }
            break;
        case ';':
            a_token.set (Token::PUNCTUATOR_SEMI_COLON);
            break ;
        case '{':
            a_token.set (Token::PUNCTUATOR_CURLY_BRACKET_OPEN);
            break;
        case '}':
            a_token.set (Token::PUNCTUATOR_CURLY_BRACKET_CLOSE);
            break ;
        case '[':
            a_token.set (Token::PUNCTUATOR_BRACKET_OPEN);
            break ;
        case ']':
            a_token.set (Token::PUNCTUATOR_BRACKET_CLOSE);
            break ;
        case '(':
            a_token.set (Token::PUNCTUATOR_PARENTHESIS_OPEN);
            break;
        case ')':
            a_token.set (Token::PUNCTUATOR_PARENTHESIS_CLOSE);
            break;
        case '?':
            a_token.set (Token::PUNCTUATOR_QUESTION_MARK);
            break;
        default:
            goto error;
    }
    CONSUME_CHAR;
    return true ;
error:
    RESTORE_POSITION;
    return false ;
}

bool
Lexer::get_next_token (Token &a_token)
{
    CHECK_CURSOR_BOUNDS;
    RECORD_POSITION;
    bool is_ok=false;

    skip_blanks ();

    //try operators
    switch (CUR_CHAR) {
        case 'n':
        case 'd':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
        case '&':
        case '|':
        case '~':
        case '=':
        case '<':
        case '>':
        case ',':
        case '(':
        case '[':
        case '.':
        case ':':
            is_ok = scan_operator (a_token) ;
            if (is_ok) {
                return true ;
            }
            break;
        default:
            break;
    }
    //then try punctuators
    switch (CUR_CHAR) {
        case ':':
        case ';':
        case '{':
        case '}':
        case '[':
        case ']':
        case '(':
        case ')':
        case '?':
            is_ok = scan_punctuator (a_token);
            if (is_ok) {
                return true;
            }
            break;
        default:
            break;
    }
    //then try literals
    switch (CUR_CHAR) {
        case '\\':
        case '\'':
        case '"':
        case 'L':
        case 'l':
        case 'u':
        case 'U':
        case '.':
        case 't':
        case 'f':
            is_ok = scan_literal (a_token);
            break;
        default :
            if (is_digit (CUR_CHAR)) {
                is_ok = scan_literal (a_token);
            }
            break;
    }
    if (is_ok) {
        return true;
    }
    //then try keywords
    if (is_nondigit (CUR_CHAR)) {
        is_ok = scan_keyword (a_token) ;
        if (is_ok) {
            return true ;
        }
    }
    //then try identifiers
    if (is_nondigit (CUR_CHAR)) {
        is_ok = scan_identifier (a_token) ;
        if (is_ok) {
            return true ;
        }
    }

    RESTORE_POSITION;
    return false;
}
//********************
//</class Lexer implem>
//********************
NEMIVER_END_NAMESPACE (cpp)
NEMIVER_END_NAMESPACE (nemiver)

