#include <iostream>
#include <string>
#include <boost/test/unit_test.hpp>
#include <glibmm.h>
#include "common/nmv-ustring.h"
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "langs/nmv-cpp-parser.h"

const char *prog0 = "foo bar" ;
const char *prog1 = "foo * bar" ;
const char *prog2 = "const foo * bar" ;
const char *prog3 = "static const foo * bar" ;
const char *prog4 = "static const unsigned int foo" ;
const char *prog5= "(int)5";
const char *prog5_1= "5";
const char *prog5_2= "foo.*bar";
const char *prog5_3= "foo->*bar";
const char *prog5_4= "foo*bar";
const char *prog5_5= "foo/bar";
const char *prog5_6= "foo%bar";
const char *prog5_7= "foo+bar-baz";

using std::cout;
using std::endl;
using nemiver::cpp::Parser;
using nemiver::cpp::SimpleDeclarationPtr;
using nemiver::cpp::DeclSpecifier;
using nemiver::cpp::DeclSpecifierPtr;
using nemiver::cpp::InitDeclaratorPtr;
using nemiver::cpp::InitDeclarator;
using nemiver::cpp::CastExprPtr;
using nemiver::cpp::CastExpr;
using nemiver::cpp::PMExprPtr;
using nemiver::cpp::MultExprPtr;
using nemiver::cpp::AddExprPtr;
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
    Parser parser (prog1);
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
    if (prog1 != str) {
        BOOST_FAIL ("parsed '" <<prog1 << "' into '" << str << "'");
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
    Parser parser (prog3);
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
    if (prog3 != str) {
        BOOST_FAIL ("parsed '" <<prog3 << "' into '" << str << "'");
    }
}

void
test_parser4 ()
{
    Parser parser (prog4);
    string str;
    list<DeclSpecifierPtr>decls;

    if (!parser.parse_decl_specifier_seq (decls)) {
        BOOST_FAIL ("parsing failed");
    }
    if (decls.empty ()) {
        cout << "got empty decl specifier sequence" << endl;
        return;
    }
    DeclSpecifier::list_to_string (decls, str);
    if (str != "static const unsigned int") {
        BOOST_FAIL ("parsed '" <<prog4 << "' into '" << str << "'");
    }
}

void
test_parser5 ()
{
    Parser parser (prog5);
    CastExprPtr cast_expr;
    if (!parser.parse_cast_expr (cast_expr) || !cast_expr) {
        BOOST_FAIL ("failed to parse cast expresssion: " << prog5);
    }
    string str;
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
    return suite;

    NEMIVER_CATCH_NOX

    return 0;
}
