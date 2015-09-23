if AUTOTESTS
# 'make check' automatically runs programs listed in the TESTS variable

#runtestoverloads
#runtestglobalvariables
#runtestenv

#runtestvars is not run as part of the tests because of a GDB bug: https://sourceware.org/bugzilla/show_bug.cgi?id=17124.  Otherwise, it loops indefinitly.
#likewise for runtestvarpathexpr

TESTS=\
runtestgdbmi runtestunicode \
runtestcpptrait runtestvarlist \
runtestvarwalker runtestvarobjwalker runtestbreakpoint \
runtestwatchpoint runtestderef \
runtestlocalvarslist runtestcpplexer \
runtestcppparser  \
runtestlibtoolwrapperdetection \
runtesttypes runtestdisassemble \
runtestvariableformat runtestprettyprint \
runtestthreads

else

TESTS=

endif

noinst_PROGRAMS= \
$(TESTS) \
runtestcore  runteststdout  docore inout \
pointerderef fooprog localsinmiddle templatedvar \
gtkmmtest dostackoverflow bigvar threads \
forkparent forkchild prettyprint

runtestgdbmi_SOURCES=test-gdbmi.cc
runtestgdbmi_LDADD= @NEMIVERCOMMON_LIBS@ \
@BOOST_UNIT_TEST_FRAMEWORK_STATIC_LIB@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libgdbmiparser.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

gtkmmtest_SOURCES=gtkmm-test.cc
gtkmmtest_CXXFLAGS= @NEMIVERUICOMMON_CFLAGS@
gtkmmtest_LDADD= @NEMIVERUICOMMON_LIBS@

fooprog_SOURCES=fooprog.cc
fooprog_LDADD=

localsinmiddle_SOURCES=locals-in-middle.cc
localsinmiddle_LDADD=

forkparent_SOURCES=fork-parent.cc
forkparent_LDADD=

forkchild_SOURCES=fork-child.cc
forkchild_LDADD=

prettyprint_SOURCES=pretty-print.cc
prettyprint_LDADD=

runtestbreakpoint_SOURCES=test-breakpoint.cc
runtestbreakpoint_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestwatchpoint_SOURCES=test-watchpoint.cc
runtestwatchpoint_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

#runtestvarpathexpr_SOURCES=test-var-path-expr.cc
#runtestvarpathexpr_LDADD=@NEMIVERCOMMON_LIBS@ \
#$(top_builddir)/src/common/libnemivercommon.la \
#$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestvarobjwalker_SOURCES=test-varobj-walker.cc
runtestvarobjwalker_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestvariableformat_SOURCES=test-variable-format.cc
runtestvariableformat_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestprettyprint_SOURCES=test-pretty-print.cc
runtestprettyprint_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestthreads_SOURCES=test-threads.cc
runtestthreads_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

#runtestoverloads_SOURCES=test-overloads.cc
#runtestoverloads_LDADD=@NEMIVERCOMMON_LIBS@ \
#$(top_builddir)/src/common/libnemivercommon.la

runtestcore_SOURCES=test-core.cc
runtestcore_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runteststdout_SOURCES=test-stdout.cc
runteststdout_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

runtestcpptrait_SOURCES=test-cpptrait.cc
runtestcpptrait_LDADD=@NEMIVERCOMMON_LIBS@ \
@BOOST_UNIT_TEST_FRAMEWORK_STATIC_LIB@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestunicode_SOURCES=test-unicode.cc
runtestunicode_LDADD=@NEMIVERCOMMON_LIBS@ \
@BOOST_UNIT_TEST_FRAMEWORK_STATIC_LIB@ \
$(top_builddir)/src/common/libnemivercommon.la

#runtestvars_SOURCES=test-vars.cc
#runtestvars_LDADD=@NEMIVERCOMMON_LIBS@ \
#$(top_builddir)/src/common/libnemivercommon.la \
#$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestvarlist_SOURCES=test-var-list.cc
runtestvarlist_LDADD=@NEMIVERCOMMON_LIBS@  \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestvarwalker_SOURCES=test-var-walker.cc
runtestvarwalker_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestderef_SOURCES=test-deref.cc
runtestderef_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestlocalvarslist_SOURCES=test-local-vars-list.cc
runtestlocalvarslist_LDADD=@NEMIVERCOMMON_LIBS@  \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

#runtestglobalvariables_SOURCES=test-global-variables.cc
#runtestglobalvariables_LDADD=@NEMIVERCOMMON_LIBS@ \
#$(top_builddir)/src/common/libnemivercommon.la

runtestcpplexer_SOURCES=test-cpp-lexer.cc
runtestcpplexer_LDADD=@NEMIVERCOMMON_LIBS@ \
@BOOST_UNIT_TEST_FRAMEWORK_STATIC_LIB@ \
$(top_builddir)/src/langs/libnemivercparser.la \
$(top_builddir)/src/common/libnemivercommon.la

runtestcppparser_SOURCES=test-cpp-parser.cc
runtestcppparser_LDADD=@NEMIVERCOMMON_LIBS@ \
@BOOST_UNIT_TEST_FRAMEWORK_STATIC_LIB@ \
$(top_builddir)/src/langs/libnemivercparser.la \
$(top_builddir)/src/common/libnemivercommon.la

runtestlibtoolwrapperdetection_SOURCES=test-libtool-wrapper-detection.cc
runtestlibtoolwrapperdetection_LDADD=@NEMIVERCOMMON_LIBS@ \
@BOOST_UNIT_TEST_FRAMEWORK_STATIC_LIB@ \
$(top_builddir)/src/langs/libnemivercparser.la \
$(top_builddir)/src/common/libnemivercommon.la

#runtestenv_SOURCES=test-env.cc
#runtestenv_LDADD=@NEMIVERCOMMON_LIBS@ \
#@BOOST_UNIT_TEST_FRAMEWORK_STATIC_LIB@ \
#$(top_builddir)/src/common/libnemivercommon.la

runtesttypes_SOURCES=test-types.cc
runtesttypes_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

runtestdisassemble_SOURCES=test-disassemble.cc
runtestdisassemble_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libdebuggerutils.la

docore_SOURCES=do-core.cc
docore_LDADD=@NEMIVERCOMMON_LIBS@

inout_SOURCES=inout.cc
inout_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

pointerderef_SOURCES=pointer-deref.cc
pointerderef_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

templatedvar_SOURCES=templated-var.cc
templatedvar_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

dostackoverflow_SOURCES=do-stack-overflow.cc
dostackoverflow_LDADD=@NEMIVERCOMMON_LIBS@

bigvar_SOURCES=big-var.c
bigvar_LDADD=@NEMIVERCOMMON_LIBS@

threads_SOURCES=threads.cc
threads_LDADD=@NEMIVERCOMMON_LIBS@

AM_CPPFLAGS=-I$(top_srcdir)/src \
-I$(top_srcdir)/src/confmgr \
-I$(top_srcdir)/src/dbgengine \
@NEMIVERCOMMON_CFLAGS@ -DBOOST_TEST_DYN_LINK \
-DNEMIVER_SRCDIR=\"$(top_srcdir)\" \
-DNEMIVER_BUILDDIR=\"$(top_builddir)\"

LDADD=$(top_builddir)/src/common/libnemivercommon.la

EXTRA_DIST=libtool-wrapper-with-dashes
