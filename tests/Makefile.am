noinst_PROGRAMS=fooprog runtestbreakpoint runtestcore docore inout

fooprog_SOURCES=fooprog.cc
fooprog_LDFLAGS=@NEMIVER_LIBS@

runtestbreakpoint_SOURCES=test-breakpoint.cc
runtestbreakpoint_LDFLAGS=@NEMIVER_LIBS@

runtestcore_SOURCES=test-core.cc
runtestcore_LDFLAGS=@NEMIVER_LIBS@

docore_SOURCES=do-core.cc
docore_LDFLAGS=@NEMIVER_LIBS@

inout_SOURCES=inout.cc
inout_LDFLAGS=@NEMIVER_LIBS@

INCLUDES=-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/dbgengine \
@NEMIVER_CFLAGS@

LDADD=$(top_builddir)/src/common/libnemivercommon.la

