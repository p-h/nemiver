#include <iostream>
#include <string>
#include <glibmm.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include "common/nmv-ustring.h"
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"

using namespace std ;
using CppUnit::TestFixture ;
using CppUnit::TestSuite;
using CppUnit::TestCaller ;
using CppUnit::TextUi::TestRunner ;
using namespace nemiver ;
using nemiver::common::Initializer ;
using nemiver::common::UString ;
using nemiver::common::WString ;
using nemiver::common::ustring_to_wstring ;
using nemiver::common::wstring_to_ustring ;

gunichar s_wstr[] = {230, 231, 232, 233, 234, 0} ;

class UnicodeTests : public TestFixture {
    UString m_utf8_str ;
public:

    void setup ()
    {
    }

    void tearDown ()
    {
    }

    void test_wstring_to_ustring ()
    {
        CPPUNIT_ASSERT (wstring_to_ustring (s_wstr, m_utf8_str)) ;
        unsigned int s_wstr_len = (sizeof (s_wstr)/sizeof (gunichar))-1 ;
        CPPUNIT_ASSERT (s_wstr_len == 5) ;
        CPPUNIT_ASSERT (s_wstr_len == m_utf8_str.size ()) ;
        for (unsigned int i=0 ; i < m_utf8_str.size () ; ++i) {
            CPPUNIT_ASSERT (s_wstr[i] == m_utf8_str[i]) ;
        }
    }

    void test_ustring_to_wstring ()
    {
        CPPUNIT_ASSERT (wstring_to_ustring (s_wstr, m_utf8_str)) ;
        WString wstr ;
        CPPUNIT_ASSERT (ustring_to_wstring (m_utf8_str, wstr)) ;
        CPPUNIT_ASSERT (wstr.size () == m_utf8_str.size ()) ;
        CPPUNIT_ASSERT (wstr.size () == 5) ;
        CPPUNIT_ASSERT (!wstr.compare (0, wstr.size (), s_wstr)) ;
    }
};//end UnicodeTests

int
main ()
{
    NEMIVER_TRY
    Initializer::do_init () ;
    TestSuite *suite = new TestSuite ("UnicodeTests") ;
    suite->addTest (new TestCaller<UnicodeTests>
                                    ("test_wstring_to_ustring",
                                     &UnicodeTests::test_wstring_to_ustring)) ;
    suite->addTest (new TestCaller<UnicodeTests>
                                    ("test_ustring_to_wstring",
                                     &UnicodeTests::test_ustring_to_wstring)) ;
    TestRunner runner ;
    runner.addTest (suite) ;
    runner.run () ;
    NEMIVER_CATCH_NOX
    return 0 ;
}

