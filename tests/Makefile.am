noinst_PROGRAMS=fooprog runtestbreakpoint \
runtestcore runteststdout \
docore inout

fooprog_SOURCES=fooprog.cc
fooprog_LDFLAGS=@NEMIVERCOMMON_LIBS@

runtestbreakpoint_SOURCES=test-breakpoint.cc
runtestbreakpoint_LDFLAGS=@NEMIVERCOMMON_LIBS@

runtestcore_SOURCES=test-core.cc
runtestcore_LDFLAGS=@NEMIVERCOMMON_LIBS@

runteststdout_SOURCES=test-stdout.cc
runteststdout_LDFLAGS=@NEMIVERCOMMON_LIBS@

docore_SOURCES=do-core.cc
docore_LDFLAGS=@NEMIVERCOMMON_LIBS@

inout_SOURCES=inout.cc
inout_LDFLAGS=@NEMIVERCOMMON_LIBS@

INCLUDES=-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/dbgengine \
@NEMIVERCOMMON_CFLAGS@

LDADD=$(top_builddir)/src/common/libnemivercommon.la

