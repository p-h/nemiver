#include <iostream>
#include <string>
#include <boost/test/unit_test.hpp>
#include <glibmm.h>
#include "common/nmv-ustring.h"
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "langs/nmv-cpp-parser.h"

const char *prog0 = "foo bar" ;
const char *prog1 = "foo *bar" ;
const char *prog1_1 = "foo bar[10]" ;
const char *prog2 = "const foo *bar" ;
const char *prog3 = "static const foo *bar" ;
const char *prog3_1 = "static const bar<baz> maman" ;
const char *prog4 = "static const unsigned int" ;
const char *prog4_1 = "foo<bar>" ;
const char *prog5 = "(int)5";
const char *prog5_1 = "5";
const char *prog5_2 = "foo.*bar";
const char *prog5_3 = "foo->*bar";
const char *prog5_4 = "foo*bar";
const char *prog5_5 = "foo/bar";
const char *prog5_6 = "foo%bar";
const char *prog5_7 = "foo+bar-baz";
const char *prog5_8 = "foo<<bar";
const char *prog5_9 = "foo>>bar";
const char *prog5_10 = "foo<bar";
const char *prog5_11 = "foo>bar";
const char *prog5_12 = "foo>=bar";
const char *prog5_13 = "foo<=bar";
const char *prog5_14 = "(1+3)*5<=(10-6+32)-bar";
const char *prog5_15 = "foo==bar";
const char *prog5_16 = "foo<=bar";
const char *prog5_17 = "foo&bar";
const char *prog5_18 = "foo^bar";
const char *prog5_19 = "foo|bar";
const char *prog5_20 = "foo&&bar";
const char *prog5_21 = "foo||bar";
const char *prog5_22 = "(foo<bar)?coin=pouf:paf=pim";
const char *prog7_0 = "foo<t1, t2, t3>";
const char *prog7_1 = "foo<(t1>t2)>";
const char *prog7_2 = "Y<X<1> >";

const char *expressions[] =
{
    "(int)5",
    "5",
    "foo.*bar",
    "foo->*bar",
    "foo*bar",
    "foo/bar",
    "foo%bar",
    "foo+bar-baz",
    "foo<<bar",
    "foo>>bar",
    "foo<bar",
    "foo>bar",
    "foo>=bar",
    "foo<=bar",
    "(1+3)*5<=(10-6+32)-bar",
    "foo==bar",
    "foo<=bar",
    "foo&bar",
    "foo^bar",
    "foo|bar",
    "foo&&bar",
    "foo||bar",
    "(foo<bar)?coin=pouf:paf=pim",
    0
};

using std::cout;
using std::endl;
using nemiver::cpp::Parser;
using nemiver::cpp::ParserPtr;
using nemiver::cpp::SimpleDeclarationPtr;
using nemiver::cpp::DeclSpecifier;
using nemiver::cpp::DeclSpecifierPtr;
using nemiver::cpp::InitDeclaratorPtr;
using nemiver::cpp::InitDeclarator;
using nemiver::cpp::Expr;
using nemiver::cpp::CastExprPtr;
using nemiver::cpp::CastExpr;
using nemiver::cpp::PMExprPtr;
using nemiver::cpp::MultExprPtr;
using nemiver::cpp::AddExprPtr;
using nemiver::cpp::ShiftExprPtr;
using nemiver::cpp::RelExprPtr;
using nemiver::cpp::EqExprPtr;
using nemiver::cpp::AndExprPtr;
using nemiver::cpp::XORExprPtr;
using nemiver::cpp::ORExprPtr;
using nemiver::cpp::LogAndExprPtr;
using nemiver::cpp::LogOrExprPtr;
using nemiver::cpp::CondExprPtr;
using nemiver::cpp::ExprPtr;
using nemiver::cpp::TemplateIDPtr;
using nemiver::common::Initializer ;
namespace cpp=nemiver::cpp;

void
test_parser0 ()
{
    Parser parser (prog0);
    Parser parser2 (prog0);
    string str;
    SimpleDeclarationPtr simple_decl;
    list<DeclSpecifierPtr> decls;

    if (!parser.parse_decl_specifier_seq (decls)) {
        BOOST_FAIL ("parsing failed");
    }
    if (decls.size () != 1) {
        BOOST_FAIL ("found " << decls.size () << "decl specifiers instead of 1");
    }
    DeclSpecifier::list_to_string (decls, str);
    cout << "decl specifier was: '" << str <<  "'." << endl;

    if (!parser2.parse_simple_declaration (simple_decl)) {
        BOOST_FAIL ("parsing failed");
    }
    if (!simple_decl) {
        BOOST_FAIL ("got empty simple declaration");
        return;
    }
    simple_decl->to_string (str);
    cout << "simple declaration was: '" << str <<  "'." << endl;

    DeclSpecifier::list_to_string (simple_decl->get_decl_specifiers (), str);
    cout << "Decl specifier was: " << str << endl;
    InitDeclarator::list_to_string (simple_decl->get_init_declarators (), str);
    cout << "Init declarators were: " << str << endl;
}

void
test_parser1 ()
{
    string str, prog_str=prog1;
    Parser parser (prog_str);
    SimpleDeclarationPtr simple_decl;

    if (!parser.parse_simple_declaration (simple_decl)) {
        BOOST_FAIL ("parsing failed");
    }
    if (!simple_decl) {
        cout << "got empty simple declaration" << endl;
        return;
    }
    simple_decl->to_string (str);
    if (prog_str != str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str = prog1_1;
    Parser parser1 (prog_str);
    if (!parser1.parse_simple_declaration (simple_decl)) {
        BOOST_FAIL ("parsing failed");
    }
    simple_decl->to_string (str);
    if (prog_str != str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }
}

void
test_parser2 ()
{
    Parser parser (prog2);
    string str;
    SimpleDeclarationPtr simple_decl;

    if (!parser.parse_simple_declaration (simple_decl)) {
        BOOST_FAIL ("parsing failed");
    }
    if (!simple_decl) {
        cout << "got empty simple declaration" << endl;
        return;
    }
    simple_decl->to_string (str);
    if (str != prog2) {
        BOOST_FAIL ("parsed '" <<prog2 << "' into '" << str << "'");
    }
}

void
test_parser3 ()
{
    SimpleDeclarationPtr simple_decl;
    vector<string> inputs;
    vector<ParserPtr> parsers;
    ParserPtr parser;
    string prog_str, str;
    
    prog_str = prog3;
    inputs.push_back (prog_str);
    parser.reset (new Parser (prog_str));
    parsers.push_back (parser);

    prog_str = prog3_1;
    inputs.push_back (prog_str);
    parser.reset (new Parser (prog_str));
    parsers.push_back (parser);

    for (unsigned i=0; i < parsers.size (); i++) {
        if (!parsers[i]->parse_simple_declaration (simple_decl)) {
            BOOST_FAIL ("parsing of '" << inputs[i] << "' failed");
        }
        if (!simple_decl) {
            cout << "got empty simple declaration" << endl;
            return;
        }
        simple_decl->to_string (str);
        if (inputs[i] != str) {
            BOOST_FAIL ("parsed '" <<inputs[i] << "' into '" << str << "'");
        }
    }
}

void
test_parser4 ()
{
    vector<string> inputs;
    vector<ParserPtr> parsers;
    ParserPtr parser;
    string prog_str, str;
    
    prog_str = prog4;
    inputs.push_back (prog_str);
    parser.reset (new Parser (prog_str));
    parsers.push_back (parser);

    prog_str = prog4_1;
    inputs.push_back (prog_str);
    parser.reset (new Parser (prog_str));
    parsers.push_back (parser);

    list<DeclSpecifierPtr> decls;
    for (unsigned i=0; i < parsers.size (); ++i) {
        if (!parsers[i]->parse_decl_specifier_seq (decls)) {
            BOOST_FAIL ("parsing of '" << inputs[i] << "' failed");
        }
        if (decls.empty ()) {
            cout << "got empty decl specifier sequence" << endl;
            return;
        }
        DeclSpecifier::list_to_string (decls, str);
        if (str != inputs[i]) {
            BOOST_FAIL ("parsed '" << inputs[i] << "' into '" << str << "'");
        }
    }
}

void
test_parser5 ()
{
    string str, prog_str;
    Parser parser (prog5);
    CastExprPtr cast_expr;
    if (!parser.parse_cast_expr (cast_expr) || !cast_expr) {
        BOOST_FAIL ("failed to parse cast expresssion: " << prog5);
    }
    cast_expr->to_string (str);
    if (str != prog5) {
        BOOST_FAIL ("parsed " << prog5 << "and got: " << str);
    }

    Parser parser1 (prog5_1);
    PMExprPtr pm_expr;
    if (!parser1.parse_pm_expr (pm_expr) || !pm_expr) {
        BOOST_FAIL ("failed to parse: " << prog5_1);
    }
    pm_expr->to_string (str);
    if (str != prog5_1) {
        BOOST_FAIL ("parsed '" <<prog5_1 << "' into '" << str << "'");
    }

    Parser parser2 (prog5_2);
    Parser parser3 (prog5_3);
    if (!parser2.parse_pm_expr (pm_expr) || !pm_expr) {
        BOOST_FAIL ("failed to parse: " << prog5_2);
    }
    pm_expr->to_string (str);
    if (str != prog5_2) {
        BOOST_FAIL ("parsed '" <<prog5_2 << "' into '" << str << "'");
    }
    if (!parser3.parse_pm_expr (pm_expr) || !pm_expr) {
        BOOST_FAIL ("failed to parse: " << prog5_3);
    }
    pm_expr->to_string (str);
    if (str != prog5_3) {
        BOOST_FAIL ("parsed '" <<prog5_3 << "' into '" << str << "'");
    }

    Parser parser4 (prog5_4);
    Parser parser5 (prog5_5);
    Parser parser6 (prog5_6);
    MultExprPtr mult_expr;
    if (!parser4.parse_mult_expr (mult_expr) || !mult_expr) {
        BOOST_FAIL ("failed to parse " << prog5_4);
    }
    mult_expr->to_string (str);
    if (str != prog5_4) {
        BOOST_FAIL ("parsed '" <<prog5_4 << "' into '" << str << "'");
    }
    if (!parser5.parse_mult_expr (mult_expr) || !mult_expr) {
        BOOST_FAIL ("failed to parse " << prog5_5);
    }
    mult_expr->to_string (str);
    if (str != prog5_5) {
        BOOST_FAIL ("parsed '" <<prog5_5 << "' into '" << str << "'");
    }
    if (!parser6.parse_mult_expr (mult_expr) || !mult_expr) {
        BOOST_FAIL ("failed to parse " << prog5_6);
    }
    mult_expr->to_string (str);
    if (str != prog5_6) {
        BOOST_FAIL ("parsed '" <<prog5_6 << "' into '" << str << "'");
    }

    Parser parser7 (prog5_7);
    AddExprPtr add_expr;
    if (!parser7.parse_add_expr (add_expr) || !add_expr) {
        BOOST_FAIL ("failed to parse " << prog5_7);
    }
    add_expr->to_string (str);
    if (str != prog5_7) {
        BOOST_FAIL ("parsed '" <<prog5_7 << "' into '" << str << "'");
    }

    Parser parser8 (prog5_8);
    ShiftExprPtr shift_expr;
    if (!parser8.parse_shift_expr (shift_expr) || !shift_expr) {
        BOOST_FAIL ("failed to parse " << prog5_8);
    }
    shift_expr->to_string (str);
    if (str != prog5_8) {
        BOOST_FAIL ("parsed '" <<prog5_8 << "' into '" << str << "'");
    }
    if (shift_expr->get_operator () != Expr::LEFT_SHIFT) {
        BOOST_FAIL ("expected left operator, but failed");
    }
    shift_expr->get_lhs ()->to_string (str);
    if (str != "foo") {
        BOOST_FAIL ("expecting lhs to be 'foo', found '" << str << "'");
    }
    shift_expr->get_rhs ()->to_string (str);
    if (str != "bar") {
        BOOST_FAIL ("expecting lhs to be 'bar', found '" << str << "'");
    }
    Parser parser9 (prog5_9);
    if (!parser9.parse_shift_expr (shift_expr) || !shift_expr) {
        BOOST_FAIL ("failed to parse " << prog5_9);
    }
    shift_expr->to_string (str);
    if (str != prog5_9) {
        BOOST_FAIL ("parsed '" <<prog5_9 << "' into '" << str << "'");
    }
    prog_str=prog5_10;
    Parser parser10 (prog_str);
    RelExprPtr rel_expr;
    if (!parser10.parse_rel_expr (rel_expr) || !rel_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    rel_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }
    prog_str=prog5_11;
    Parser parser11 (prog_str);
    if (!parser11.parse_rel_expr (rel_expr) || !rel_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    rel_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }
    prog_str=prog5_12;
    Parser parser12 (prog_str);
    if (!parser12.parse_rel_expr (rel_expr) || !rel_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    rel_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }
    prog_str=prog5_13;
    Parser parser13 (prog_str);
    if (!parser13.parse_rel_expr (rel_expr) || !rel_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    rel_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }
    prog_str=prog5_14;
    Parser parser14 (prog_str);
    if (!parser14.parse_rel_expr (rel_expr) || !rel_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    rel_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }
    rel_expr->get_lhs ()->to_string (str);
    cout << "lhs: " << str << "\n";
    rel_expr->get_rhs ()->to_string (str);
    cout << "rhs: " << str << "\n";

    prog_str=prog5_15;
    Parser parser15 (prog_str);
    EqExprPtr eq_expr;
    if (!parser15.parse_eq_expr (eq_expr) || !eq_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    eq_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str=prog5_16;
    Parser parser16 (prog_str);
    if (!parser16.parse_eq_expr (eq_expr) || !eq_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    eq_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str=prog5_17;
    Parser parser17 (prog_str);
    AndExprPtr and_expr;
    if (!parser17.parse_and_expr (and_expr) || !and_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    and_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str=prog5_18;
    Parser parser18 (prog_str);
    XORExprPtr xor_expr;
    if (!parser18.parse_xor_expr (xor_expr) || !xor_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    xor_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str=prog5_19;
    Parser parser19 (prog_str);
    ORExprPtr or_expr;
    if (!parser19.parse_or_expr (or_expr) || !or_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    or_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str=prog5_20;
    Parser parser20 (prog_str);
    LogAndExprPtr log_and_expr;
    if (!parser20.parse_log_and_expr (log_and_expr) || !log_and_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    log_and_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str=prog5_21;
    Parser parser21 (prog_str);
    LogOrExprPtr log_or_expr;
    if (!parser21.parse_log_or_expr (log_or_expr) || !log_or_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    log_or_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }

    prog_str=prog5_22;
    Parser parser22 (prog_str);
    CondExprPtr cond_expr;
    if (!parser22.parse_cond_expr (cond_expr) || !cond_expr) {
        BOOST_FAIL ("failed to parse " << prog_str);
    }
    cond_expr->to_string (str);
    if (str != prog_str) {
        BOOST_FAIL ("parsed '" <<prog_str << "' into '" << str << "'");
    }
}

void
test_parser6 ()
{
    string str;
    for (int i = 0; expressions[i]; ++i) {
        Parser parser (expressions[i]);
        ExprPtr expr;
        if (!parser.parse_expr (expr) || !expr) {
            BOOST_FAIL ("failed to parse expr No "
                        << i << ": " << expressions[i]);
        }
        expr->to_string (str);
        if (str != expressions[i]) {
            BOOST_FAIL ("parsed expr No "
                        << i
                        << ", '"
                        << expressions[i]
                        << "' into '"
                        << str
                        << "'");
        }
    }
}

void
test_parser7 ()
{
    string str, prog_str;
    TemplateIDPtr template_id;
    ParserPtr parser;
    vector<string> inputs;
    vector<ParserPtr> parsers;

    prog_str = prog7_0;
    inputs.push_back (prog_str);
    parser.reset (new Parser (prog_str));
    parsers.push_back (parser);

    prog_str = prog7_1;
    inputs.push_back (prog_str);
    parser.reset (new Parser (prog_str));
    parsers.push_back (parser);

    prog_str = prog7_2;
    inputs.push_back (prog_str);
    parser.reset (new Parser (prog_str));
    parsers.push_back (parser);

    for (unsigned i=0; i < parsers.size (); ++i) {
        if (!parsers[i]->parse_template_id (template_id)) {
            BOOST_FAIL ("failed to parse '" + inputs[i]);
        }
        template_id->to_string (str);
        if (str != inputs[i]) {
            BOOST_FAIL ("parsed expr '" + inputs[i] + "into '" + str + "'");
        }
    }
}

using boost::unit_test::test_suite ;

test_suite*
init_unit_test_suite (int argc, char** argv)
{
    if (argc || argv) {/*keep compiler happy*/}

    NEMIVER_TRY

    Initializer::do_init () ;

    test_suite *suite = BOOST_TEST_SUITE ("c++ lexer tests") ;
    suite->add (BOOST_TEST_CASE (&test_parser0)) ;
    suite->add (BOOST_TEST_CASE (&test_parser1)) ;
    suite->add (BOOST_TEST_CASE (&test_parser2)) ;
    suite->add (BOOST_TEST_CASE (&test_parser3)) ;
    suite->add (BOOST_TEST_CASE (&test_parser4)) ;
    suite->add (BOOST_TEST_CASE (&test_parser5)) ;
    suite->add (BOOST_TEST_CASE (&test_parser6)) ;
    suite->add (BOOST_TEST_CASE (&test_parser7)) ;
    return suite;

    NEMIVER_CATCH_NOX

    return 0;
}
