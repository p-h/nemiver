noinst_PROGRAMS=fooprog runtestbreakpoint

fooprog_SOURCES=fooprog.cc
fooprog_LDFLAGS=@NEMIVER_LIBS@

runtestbreakpoint_SOURCES=test-breakpoint.cc
runtestbreakpoint_LDFLAGS=@NEMIVER_LIBS@

INCLUDES=-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/DBGPerspective/dbgengine \
-DDBG_PERSPECTIVE_PLUGIN_PATH=\"@NEMIVER_PLUGINS_DIR@/DBGPerspective\" \
@NEMIVER_CFLAGS@

LDADD=$(top_builddir)/src/common/libnemivercommon.la
