if AUTOTESTS
AUTOMATED_TESTS=runtestgdbmi runtestunicode
else
AUTOMATED_TESTS=
endif

noinst_PROGRAMS= \
$(AUTOMATED_TESTS) \
runtestbreakpoint \
runtestcore runteststdout \
runtestvarlist \
docore inout \
pointerderef \
fooprog \
templatedvar

runtestgdbmi_SOURCES=test-gdbmi.cc
runtestgdbmi_LDADD= \
@NEMIVERCOMMON_LIBS@ @CPPUNIT_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/dbgengine/libgdbmiparser.la

fooprog_SOURCES=fooprog.cc
fooprog_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

runtestbreakpoint_SOURCES=test-breakpoint.cc
runtestbreakpoint_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

runtestcore_SOURCES=test-core.cc
runtestcore_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

runteststdout_SOURCES=test-stdout.cc
runteststdout_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

templatedvar_SOURCES=templated-var.cc
templatedvar_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

runtestunicode_SOURCES=test-unicode.cc
runtestunicode_LDADD=@NEMIVERCOMMON_LIBS@ @CPPUNIT_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

runtestvarlist_SOURCES=test-var-list.cc
runtestvarlist_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

docore_SOURCES=do-core.cc
docore_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

inout_SOURCES=inout.cc
inout_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

pointerderef_SOURCES=pointer-deref.cc
pointerderef_LDADD=@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la


INCLUDES=-I$(top_srcdir)/src \
-I$(top_srcdir)/src/dbgengine \
@NEMIVERCOMMON_CFLAGS@

LDADD=$(top_builddir)/src/common/libnemivercommon.la

