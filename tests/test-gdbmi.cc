#include <list>
#include <map>
#include <boost/test/unit_test.hpp>
#include "dbgengine/nmv-gdbmi-parser.h"
#include "common/nmv-exception.h"
#include "common/nmv-initializer.h"

using namespace std ;
using namespace nemiver ;

static const char* gv_str0 = "\"abracadabra\"" ;
static const char* gv_str1 = "\"/home/dodji/misc/no\\303\\253l-\\303\\251-\\303\\240/test.c\"" ;
static const char* gv_str2 = "\"No symbol \\\"events_ecal\\\" in current context.\\n\"" ;

static const char* gv_attrs0 = "msg=\"No symbol \\\"g_return_if_fail\\\" in current context.\"" ;

static const char* gv_stopped_async_output =
"*stopped,reason=\"breakpoint-hit\",bkptno=\"1\",thread-id=\"1\",frame={addr=\"0x0804afb0\",func=\"main\",args=[{name=\"argc\",value=\"1\"},{name=\"argv\",value=\"0xbfc79ed4\"}],file=\"today-main.c\",fullname=\"/home/dodji/devel/gitstore/omoko.git/applications/openmoko-today/src/today-main.c\",line=\"285\"}\n" ;

//the partial result of a gdbmi command: -stack-list-argument 1 command
//this command is used to implement IDebugger::list_frames_arguments()
static const char* gv_stacks_arguments =
"stack-args=[frame={level=\"0\",args=[{name=\"a_param\",value=\"(Person &) @0xbf88fad4: {m_first_name = {static npos = 4294967295, _M_dataplus = {<std::allocator<char>> = {<__gnu_cxx::new_allocator<char>> = {<No data fields>}, <No data fields>}, _M_p = 0x804b144 \\\"Ali\\\"}}, m_family_name = {static npos = 4294967295, _M_dataplus = {<std::allocator<char>> = {<__gnu_cxx::new_allocator<char>> = {<No data fields>}, <No data fields>}, _M_p = 0x804b12c \\\"BABA\\\"}}, m_age = 15}\"}]},frame={level=\"1\",args=[]}]" ;

static const char* gv_local_vars =
"locals=[{name=\"person\",type=\"Person\"}]";

static const char* gv_emb_str =
"\\\"\\\\311\\\\303\\\\220U\\\\211\\\\345S\\\\203\\\\354\\\\024\\\\213E\\\\b\\\\211\\\\004$\\\\350\\\\202\\\\373\\\\377\\\\377\\\\213E\\\\b\\\\203\\\\300\\\\004\\\\211\\\\004$\\\\350t\\\\373\\\\377\\\\377\\\\213U\\\\b\\\\213E\\\\f\\\\211D$\\\\004\\\\211\\\\024$\\\\350\\\\002\\\\373\\\\377\\\\377\\\\213U\\\\b\\\\203\\\\302\\\\004\\\\213E\\\\020\\\\211D$\\\\004\\\\211\\\\024$\\\\350\\\\355\\\\372\\\\377\\\\377\\\\213U\\\\b\\\\213E\\\\024\\\\211B\\\\b\\\\3538\\\\211E\\\\370\\\\213]\\\\370\\\\213E\\\\b\\\\203\\\\300\\\\004\\\\211\\\\004$\\\\350\\\\276\\\\372\\\\377\\\\377\\\\211]\\\\370\\\\353\\\\003\\\\211E\\\\370\\\\213]\\\\370\\\\213E\\\\b\\\\211\\\\004$\\\\350\\\\250\\\\372\\\\377\\\\377\\\\211]\\\\370\\\\213E\\\\370\\\\211\\\\004$\\\\350\\\\032\\\\373\\\\377\\\\377\\\\203\\\\304\\\\024[]\\\\303U\\\\211\\\\345S\\\\203\\\\354$\\\\215E\\\\372\\\\211\\\\004$\\\\350\\\\\\\"\\\\373\\\\377\\\\377\\\\215E\\\\372\\\\211D$\\\\b\\\\307D$\\\\004\\\\370\\\\217\\\\004\\\\b\\\\215E\\\\364\\\\211\\\\004$\\\\350\\\\230\\\\372\\\\377\\\\377\\\\215E\\\\372\\\\211\\\\004$\\\\350}\\\\372\\\""

;
static const char* gv_member_var = 
"{static npos = 4294967295, _M_dataplus = {<std::allocator<char>> = {<__gnu_cxx::new_allocator<char>> = {<No data fields>}, <No data fields>}, _M_p = 0x8048ce1 \"\\311\\303\\220U\\211\\345S\\203\\354\\024\\213E\\b\\211\\004$\\350\\202\\373\\377\\377\\213E\\b\\203\\300\\004\\211\\004$\\350t\\373\\377\\377\\213U\\b\\213E\\f\\211D$\\004\\211\\024$\\350\\002\\373\\377\\377\\213U\\b\\203\\302\\004\\213E\\020\\211D$\\004\\211\\024$\\350\\355\\372\\377\\377\\213U\\b\\213E\\024\\211B\\b\\3538\\211E\\370\\213]\\370\\213E\\b\\203\\300\\004\\211\\004$\\350\\276\\372\\377\\377\\211]\\370\\353\\003\\211E\\370\\213]\\370\\213E\\b\\211\\004$\\350\\250\\372\\377\\377\\211]\\370\\213E\\370\\211\\004$\\350\\032\\373\\377\\377\\203\\304\\024[]\\303U\\211\\345S\\203\\354$\\215E\\372\\211\\004$\\350\\\"\\373\\377\\377\\215E\\372\\211D$\\b\\307D$\\004\\370\\217\\004\\b\\215E\\364\\211\\004$\\350\\230\\372\\377\\377\\215E\\372\\211\\004$\\350}\\372\"...}}" ;

static const char* gv_var_with_member = "value=\"{static npos = 4294967295, _M_dataplus = {<std::allocator<char>> = {<__gnu_cxx::new_allocator<char>> = {<No data fields>}, <No data fields>}, _M_p = 0x8048ce1 \"\\311\\303\\220U\\211\\345S\\203\\354\\024\\213E\\b\\211\\004$\\350\\202\\373\\377\\377\\213E\\b\\203\\300\\004\\211\\004$\\350t\\373\\377\\377\\213U\\b\\213E\\f\\211D$\\004\\211\\024$\\350\\002\\373\\377\\377\\213U\\b\\203\\302\\004\\213E\\020\\211D$\\004\\211\\024$\\350\\355\\372\\377\\377\\213U\\b\\213E\\024\\211B\\b\\3538\\211E\\370\\213]\\370\\213E\\b\\203\\300\\004\\211\\004$\\350\\276\\372\\377\\377\\211]\\370\\353\\003\\211E\\370\\213]\\370\\213E\\b\\211\\004$\\350\\250\\372\\377\\377\\211]\\370\\213E\\370\\211\\004$\\350\\032\\373\\377\\377\\203\\304\\024[]\\303U\\211\\345S\\203\\354$\\215E\\372\\211\\004$\\350\\\"\\373\\377\\377\\215E\\372\\211D$\\b\\307D$\\004\\370\\217\\004\\b\\215E\\364\\211\\004$\\350\\230\\372\\377\\377\\215E\\372\\211\\004$\\350}\\372\"...}}\"" ;

void
test_str0 ()
{
    bool is_ok =false ;

    UString res ;
    UString::size_type to=0 ;
    is_ok = parse_c_string (gv_str0, 0, to, res) ;
    BOOST_REQUIRE (is_ok) ;
    BOOST_REQUIRE (res == "abracadabra") ;
}

void
test_str1 ()
{
    bool is_ok =false ;

    UString res ;
    UString::size_type to=0 ;
    is_ok = parse_c_string (gv_str1, 0, to, res) ;
    BOOST_REQUIRE (is_ok) ;
    MESSAGE ("got string: '" << Glib::locale_from_utf8 (res) << "'") ;
    BOOST_REQUIRE_MESSAGE (res.size () == 32, "res size was: " << res.size ());
}

void
test_str2 ()
{
    bool is_ok =false ;

    UString res ;
    UString::size_type to=0 ;
    is_ok = parse_c_string (gv_str2, 0, to, res) ;
    BOOST_REQUIRE (is_ok) ;
    MESSAGE ("got string: '" << Glib::locale_from_utf8 (res) << "'") ;
    BOOST_REQUIRE_MESSAGE (res.size (), "res size was: " << res.size ());
}

void
test_attr0 ()
{
    bool is_ok =false ;

    UString name,value ;
    UString::size_type to=0 ;
    is_ok = parse_attribute (gv_attrs0, 0, to, name, value) ;
    BOOST_REQUIRE (is_ok) ;
    BOOST_REQUIRE_MESSAGE (name == "msg", "got name: " << name) ;
    BOOST_REQUIRE_MESSAGE (value.size(), "got empty value") ;
}

void
test_stoppped_async_output ()
{
    bool is_ok=false,got_frame=false ;
    UString::size_type to=0 ;
    IDebugger::Frame frame ;
    map<UString, UString> attrs ;

    is_ok = parse_stopped_async_output (gv_stopped_async_output, 0, to,
                                        got_frame, frame, attrs) ;
    BOOST_REQUIRE (is_ok) ;
    BOOST_REQUIRE (got_frame) ;
    BOOST_REQUIRE (attrs.size ()) ;
}

void
test_stack_arguments ()
{
    bool is_ok=false ;
    UString::size_type to ;
    map<int, list<IDebugger::VariableSafePtr> >params ;
    is_ok = nemiver::parse_stack_arguments (gv_stacks_arguments,
                                            0, to, params) ;
    BOOST_REQUIRE (is_ok) ;
    BOOST_REQUIRE (params.size () == 2) ;
    map<int, list<IDebugger::VariableSafePtr> >::iterator param_iter ;
    param_iter = params.find (0) ;
    BOOST_REQUIRE (param_iter != params.end ()) ;
    IDebugger::VariableSafePtr variable = *(param_iter->second.begin ()) ;
    BOOST_REQUIRE (variable) ;
    BOOST_REQUIRE (variable->name () == "a_param") ;
    BOOST_REQUIRE (!variable->members ().empty ()) ;
}

void
test_local_vars ()
{
    bool is_ok=false ;
    UString::size_type to=0 ;
    list<IDebugger::VariableSafePtr> vars ;
    is_ok = parse_local_var_list (gv_local_vars, 0, to, vars) ;
    BOOST_REQUIRE (is_ok) ;
    BOOST_REQUIRE (vars.size () == 1) ;
    IDebugger::VariableSafePtr var = *(vars.begin ()) ;
    BOOST_REQUIRE (var) ;
    BOOST_REQUIRE (var->name () == "person") ;
    BOOST_REQUIRE (var->type () == "Person") ;
}

void
test_member_variable ()
{
    UString::size_type to = 0;
    IDebugger::VariableSafePtr var (new IDebugger::Variable) ;
    BOOST_REQUIRE (parse_member_variable (gv_member_var, 0, to, var)) ;
    BOOST_REQUIRE (var) ;
    BOOST_REQUIRE (!var->members ().empty ()) ;
}

void
test_var_with_member_variable ()
{
    UString::size_type to = 0;
    IDebugger::VariableSafePtr var (new IDebugger::Variable) ;
    BOOST_REQUIRE (parse_variable_value (gv_var_with_member, 0, to, var)) ;
    BOOST_REQUIRE (var) ;
    BOOST_REQUIRE (!var->members ().empty ()) ;
}

void
test_embedded_string ()
{
    UString::size_type to = 0 ;
    UString str ;
    BOOST_REQUIRE (parse_embedded_c_string (gv_emb_str, 0, to, str)) ;
}

using boost::unit_test::test_suite ;

NEMIVER_API test_suite*
init_unit_test_suite (int argc, char **argv)
{
    if (argc || argv) {/*keep compiler happy*/}

    NEMIVER_TRY

    nemiver::common::Initializer::do_init () ;

    test_suite *suite = BOOST_TEST_SUITE ("GDBMI tests") ;
    suite->add (BOOST_TEST_CASE (&test_str0)) ;
    suite->add (BOOST_TEST_CASE (&test_str1)) ;
    suite->add (BOOST_TEST_CASE (&test_str2)) ;
    suite->add (BOOST_TEST_CASE (&test_attr0)) ;
    suite->add (BOOST_TEST_CASE (&test_stoppped_async_output)) ;
    suite->add (BOOST_TEST_CASE (&test_stack_arguments)) ;
    suite->add (BOOST_TEST_CASE (&test_local_vars)) ;
    suite->add (BOOST_TEST_CASE (&test_member_variable)) ;
    suite->add (BOOST_TEST_CASE (&test_var_with_member_variable)) ;
    suite->add (BOOST_TEST_CASE (&test_embedded_string)) ;
    return suite ;

    NEMIVER_CATCH_NOX

    return 0 ;
}

