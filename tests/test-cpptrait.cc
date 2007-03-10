#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include "common/nmv-initializer.h"
#include "common/nmv-exception.h"
#include "nmv-i-lang-trait.h"
#include "nmv-i-debugger.h"

using namespace std ;
using CppUnit::TestFixture ;
using CppUnit::TestSuite;
using CppUnit::TestCaller ;
using CppUnit::TextUi::TestRunner ;
using nemiver::common::Initializer;
using nemiver::common::DynamicModuleManager ;
using nemiver::ILangTraitSafePtr ;
using nemiver::ILangTrait;

class tests : public TestFixture {
public:
    void setup ()
    {
    }

    void tearDown ()
    {
    }

    void test_load_dynmod ()
    {
        ILangTraitSafePtr trait =
            DynamicModuleManager::load_iface_with_default_manager<ILangTrait>
                                                            ("cpptrait",
                                                             "ILangTrait");
        //CPPUNIT_ASSERT (trait) ;
    }

    void test_name ()
    {
        ILangTraitSafePtr trait =
            DynamicModuleManager::load_iface_with_default_manager<ILangTrait>
                                                            ("cpptrait",
                                                             "ILangTrait");
        CPPUNIT_ASSERT (trait->get_name () == "cpptrait") ;
    }

    void test_is_pointer ()
    {
        ILangTraitSafePtr trait =
            DynamicModuleManager::load_iface_with_default_manager<ILangTrait>
                                                            ("cpptrait",
                                                             "ILangTrait");
        CPPUNIT_ASSERT (trait->has_pointers () == true) ;

        CPPUNIT_ASSERT (trait->is_type_a_pointer ("char *")) ;
        CPPUNIT_ASSERT (trait->is_type_a_pointer ("AType * const")) ;
    }

    void test_debugger ()
    {
        using nemiver::IDebugger ;
        using nemiver::IDebuggerSafePtr ;
        IDebuggerSafePtr debugger =
            DynamicModuleManager::load_iface_with_default_manager<IDebugger>
                ("gdbengine", "IDebugger") ;
        CPPUNIT_ASSERT (debugger) ;
        ILangTraitSafePtr trait = debugger->get_language_trait () ;
        CPPUNIT_ASSERT (trait) ;
        CPPUNIT_ASSERT (trait->get_name () == "cpptrait") ;
    }
};//end tests

int
main ()
{
    NEMIVER_TRY

    Initializer::do_init () ;

    TestSuite *suite  = new TestSuite ("CPPTraitTests") ;
    suite->addTest (new TestCaller<tests> ("test_load_dynmod",
                                           &tests::test_load_dynmod)) ;
    suite->addTest (new TestCaller<tests> ("test_name",
                                           &tests::test_name)) ;
    suite->addTest (new TestCaller<tests> ("test_is_pointer",
                                           &tests::test_is_pointer)) ;
    suite->addTest (new TestCaller<tests> ("test_debugger",
                                           &tests::test_debugger)) ;

    TestRunner runner ;
    runner.addTest (suite) ;
    runner.run () ;

    NEMIVER_CATCH_NOX
    return 0 ;
}
