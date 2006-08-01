SUBDIRS=common uicommon workbench DBGPerspective

bin_PROGRAMS=nemiver
nemiver_SOURCES=main.cc
nemiver_LDADD= \
@NEMIVER_LIBS@  \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/uicommon/libnemiveruicommon.la

INCLUDES=\
@NEMIVER_CFLAGS@ \
-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/uicommon \
-I$(top_srcdir)/src/workbench \
-I$(top_srcdir)/src/DBGPerspective \
-I$(top_srcdir)/src/DBGPerspective/dbgengine

