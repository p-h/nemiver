#include <iostream>
#include <string>
#include <boost/test/unit_test.hpp>
#include <glibmm.h>
#include "common/nmv-ustring.h"
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "langs/nmv-cpp-parser.h"

const char *prog0 = "foo bar;" ;
const char *prog1 = "foo *bar;" ;
const char *prog2 = "const foo *bar;" ;
const char *prog3 = "static const foo *bar;" ;
const char *prog4 = "static const unsigned int foo;" ;

using std::cout;
using std::endl;
using nemiver::cpp::Parser;
using nemiver::cpp::SimpleDeclarationPtr;
using nemiver::cpp::DeclSpecifier;
using nemiver::cpp::DeclSpecifierPtr;
using nemiver::cpp::InitDeclaratorPtr;
using nemiver::cpp::InitDeclarator;
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
    cout << "parsed: '" << str <<  "'." << endl;
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
    cout << "parsed: '" << str <<  "'." << endl;
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
    cout << "parsed: '" << str <<  "'." << endl;
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
    cout << "parsed: '" << str <<  "'." << endl;
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
    return suite;

    NEMIVER_CATCH_NOX

    return 0;
}
