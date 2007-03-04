#include <list>
#include <map>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include "dbgengine/nmv-gdbmi-parser.h"
#include "common/nmv-exception.h"
#include "common/nmv-initializer.h"

using namespace std ;
using CppUnit::TestFixture ;
using CppUnit::TestSuite;
using CppUnit::TestCaller ;
using CppUnit::TextUi::TestRunner ;
using namespace nemiver ;

//the partial result of a gdbmi command: -stack-list-argument 1 command
//this command is used to implement IDebugger::list_frames_arguments()
static const char* gv_stacks_arguments =
"stack-args=[frame={level=\"0\",args=[{name=\"a_param\",value=\"(Person &) @0xbf88fad4: {m_first_name = {static npos = 4294967295, _M_dataplus = {<std::allocator<char>> = {<__gnu_cxx::new_allocator<char>> = {<No data fields>}, <No data fields>}, _M_p = 0x804b144 \\\"Ali\\\"}}, m_family_name = {static npos = 4294967295, _M_dataplus = {<std::allocator<char>> = {<__gnu_cxx::new_allocator<char>> = {<No data fields>}, <No data fields>}, _M_p = 0x804b12c \\\"BABA\\\"}}, m_age = 15}\"}]},frame={level=\"1\",args=[]}]" ;

static const char* gv_local_vars =
"locals=[{name=\"person\",type=\"Person\"}]";

class GDBMITests : public TestFixture {

public:

    GDBMITests ()
    {
    }

    void setup ()
    {
    }

    void tearDown ()
    {
    }

    void test_stack_arguments ()
    {
        bool is_ok=false ;
        UString::size_type to ;
        map<int, list<IDebugger::VariableSafePtr> >params ;
        is_ok = nemiver::parse_stack_arguments (gv_stacks_arguments,
                                                0, to, params) ;
        CPPUNIT_ASSERT (is_ok) ;
        CPPUNIT_ASSERT (params.size () == 2) ;
        map<int, list<IDebugger::VariableSafePtr> >::iterator param_iter ;
        param_iter = params.find (0) ;
        CPPUNIT_ASSERT (param_iter != params.end ()) ;
        IDebugger::VariableSafePtr variable = *(param_iter->second.begin ()) ;
        CPPUNIT_ASSERT (variable) ;
        CPPUNIT_ASSERT (variable->name () == "a_param") ;
        CPPUNIT_ASSERT (!variable->members ().empty ()) ;
    }

    void test_local_vars ()
    {
        bool is_ok=false ;
        UString::size_type to=0 ;
        list<IDebugger::VariableSafePtr> vars ;
        is_ok = parse_local_var_list (gv_local_vars, 0, to, vars) ;
        CPPUNIT_ASSERT (is_ok) ;
        CPPUNIT_ASSERT (vars.size () == 1) ;
        IDebugger::VariableSafePtr var = *(vars.begin ()) ;
        CPPUNIT_ASSERT (var) ;
        CPPUNIT_ASSERT (var->name () == "person") ;
        CPPUNIT_ASSERT (var->type () == "Person") ;
    }
};//end GDBMITests

void
setup_test_runner (TestRunner &a_runner)
{
    TestSuite *suite = new TestSuite ("GDBMITests") ;
    suite->addTest (new TestCaller<GDBMITests> ("test_stack_arguments",
                                              &GDBMITests::test_stack_arguments)) ;
    suite->addTest (new TestCaller<GDBMITests> ("test_local_vars",
                                              &GDBMITests::test_local_vars)) ;
    a_runner.addTest (suite) ;
}

int
main ()
{
    NEMIVER_TRY

    nemiver::common::Initializer::do_init () ;
    TestRunner runner ;
    setup_test_runner (runner) ;
    runner.run () ;

    NEMIVER_CATCH_NOX

    return 0 ;
}

