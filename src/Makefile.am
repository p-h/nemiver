SUBDIRS=common dbdimpl langs dbgengine confmgr uicommon workbench persp

h=$(abs_srcdir)

bin_PROGRAMS=nemiver
nemiver_SOURCES=$(h)/main.cc
nemiver_LDADD= \
@NEMIVER_LIBS@  \
$(abs_top_builddir)/src/common/libnemivercommon.la \
$(abs_top_builddir)/src/uicommon/libnemiveruicommon.la

INCLUDES=\
-DNEMIVERLOCALEDIR=\"$(nemiverlocaledir)\" \
-DPREFIX=\""$(prefix)"\" \
-DSYSCONFDIR=\""$(sysconfdir)"\" \
-DLIBDIR=\""$(libdir)"\" \
-DDATADIR=\""$(datadir)"\" \
@NEMIVER_CFLAGS@ \
-I$(abs_top_srcdir)/src/common \
-I$(abs_top_srcdir)/src/confmgr \
-I$(abs_top_srcdir)/src/dbgengine \
-I$(abs_top_srcdir)/src/uicommon \
-I$(abs_top_srcdir)/src/workbench \
-I$(abs_top_srcdir)/src/persp \
-I$(abs_top_srcdir)/src/persp/dbgperspective
