#include <iostream>
#include <string>
#include <boost/test/unit_test.hpp>
#include <glibmm.h>
#include "common/nmv-ustring.h"
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "langs/nmv-cpp-parser.h"

const char *prog = "foo bar;" ;

using nemiver::cpp::Parser;
using nemiver::common::Initializer ;
namespace cpp=nemiver::cpp;

void
test_parser ()
{
    Parser parser (prog);
    string str;
    if (!parser.parse_simple_declaration (str)) {
        BOOST_FAIL ("parsing failed");
    }
    std::cout << "parsing went okay" << std::endl;
}


using boost::unit_test::test_suite ;

test_suite*
init_unit_test_suite (int argc, char** argv)
{
    if (argc || argv) {/*keep compiler happy*/}

    NEMIVER_TRY

    Initializer::do_init () ;

    test_suite *suite = BOOST_TEST_SUITE ("c++ lexer tests") ;
    suite->add (BOOST_TEST_CASE (&test_parser)) ;
    return suite;

    NEMIVER_CATCH_NOX

    return 0;
}
