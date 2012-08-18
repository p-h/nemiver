#include "config.h"
#include <iostream>
#include <string>
#include <boost/test/unit_test.hpp>
#include <glibmm.h>
#include "common/nmv-ustring.h"
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "common/nmv-env.h"

using namespace std;
using nemiver::common::UString;
using nemiver::common::Initializer;
using boost::unit_test::test_suite;
namespace env=nemiver::common::env;


test_suite*
init_unit_test_suite (int, char**)
{
    NEMIVER_TRY

    Initializer::do_init ();

    // test_suite *suite = BOOST_TEST_SUITE ("nemiver env tests");
    // suite->add (BOOST_TEST_CASE (&test_function_to_be_defined));
    // return suite;

    NEMIVER_CATCH_NOX

    return 0;
}
