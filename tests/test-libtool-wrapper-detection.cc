#include "config.h"
#include <iostream>
#include <string>
#include <vector>
#include <boost/test/unit_test.hpp>
#include "common/nmv-exception.h"
#include "common/nmv-initializer.h"
#include "common/nmv-proc-utils.h"

using namespace nemiver::common;
using boost::unit_test::test_suite;
using std::vector;
using std::string;

BOOST_AUTO_TEST_SUITE (test_libtool_wrapper_detection)

BOOST_AUTO_TEST_CASE (test0)
{
    vector<string> path_elems;
    path_elems.push_back (NEMIVER_BUILDDIR);
    path_elems.push_back ("tests");
    path_elems.push_back ("runtestcore");
    string path = Glib::build_filename (path_elems);
    BOOST_REQUIRE (Glib::file_test (path, Glib::FILE_TEST_EXISTS));
    BOOST_REQUIRE (is_libtool_executable_wrapper (path));
}

BOOST_AUTO_TEST_CASE (test_filename_with_dashes)
{
    vector<string> path_elems;
    path_elems.push_back (NEMIVER_SRCDIR);
    path_elems.push_back ("tests");
    path_elems.push_back ("libtool-wrapper-with-dashes");
    string path = Glib::build_filename (path_elems);
    std::cout << "path: '" << path << "'" << std::endl;
    BOOST_REQUIRE (Glib::file_test (path, Glib::FILE_TEST_EXISTS));
    BOOST_REQUIRE (is_libtool_executable_wrapper (path));

}

bool
init_unit_test ()
{
    NEMIVER_TRY

    Initializer::do_init ();

    NEMIVER_CATCH_NOX

    return 0;
}

BOOST_AUTO_TEST_SUITE_END()
