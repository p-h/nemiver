#include <iostream>
#include <string>
#include <boost/test/unit_test.hpp>
#include <glibmm.h>
#include "common/nmv-ustring.h"
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "langs/nmv-cpp-lexer.h"

const char *prog = "if (foo ()) {printf (\"bar\");}" ;

using nemiver::cpp::Lexer;
using nemiver::cpp::Token;
using nemiver::common::Initializer ;

void test_lexer ()
{
    Lexer lexer (prog) ;
    Token token ;
    int i=0;
    while (lexer.get_next_token (token)) {
        ++i ;
        BOOST_MESSAGE ("got token nb: " << i) ;
        token.clear () ;
    }
    BOOST_REQUIRE_MESSAGE (i == 12, "got " << i << " number of tokens");
}

using boost::unit_test::test_suite ;

test_suite*
init_unit_test_suite (int argc, char** argv)
{
    if (argc || argv) {/*keep compiler happy*/}

    NEMIVER_TRY

    Initializer::do_init () ;

    test_suite *suite = BOOST_TEST_SUITE ("c++ lexer tests") ;
    suite->add (BOOST_TEST_CASE (&test_lexer)) ;
    return suite;

    NEMIVER_CATCH_NOX

    return 0;
}
