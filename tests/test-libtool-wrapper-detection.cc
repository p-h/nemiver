#include <iostream>
#include <string>
#include <boost/test/unit_test.hpp>
#include "common/nmv-exception.h"
#include "common/nmv-initializer.h"
#include "common/nmv-proc-utils.h"

using namespace nemiver::common;
using boost::unit_test::test_suite ;

void
test0 ()
{
    UString real_path;
    BOOST_REQUIRE (is_libtool_executable_wrapper ("./fooprog"));
}

test_suite*
init_unit_test_suite (int argc, char** argv)
{
    if (argc || argv) {/*keep compiler happy*/}

    NEMIVER_TRY

    Initializer::do_init () ;

    test_suite *suite = BOOST_TEST_SUITE ("libtool wrapper detect tests") ;
    suite->add (BOOST_TEST_CASE (&test0)) ;
    return suite;

    NEMIVER_CATCH_NOX

    return 0;
}

