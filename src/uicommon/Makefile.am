noinst_LTLIBRARIES=libnemiveruicommon.la

sources = \
$(public_headers) \
nmv-ui-utils.h \
nmv-ui-utils.cc \
nmv-dialog.cc \
nmv-dialog.h \
nmv-locate-file-dialog.cc \
nmv-locate-file-dialog.h \
nmv-spinner-tool-item.h \
nmv-spinner-tool-item.cc \
nmv-popup-tip.cc \
nmv-popup-tip.h \
nmv-terminal.cc \
nmv-terminal.h \
nmv-source-editor.cc \
nmv-source-editor.h \
nmv-layout.h \
nmv-layout-manager.cc \
nmv-layout-manager.h \
nmv-layout-selector.cc \
nmv-layout-selector.h

if BUILD_MEMORYVIEW
memoryview_sources = \
nmv-hex-editor.h \
nmv-hex-editor.cc \
nmv-hex-document.h \
nmv-hex-document.cc
else
memoryview_sources =
endif

libnemiveruicommon_la_SOURCES= $(sources) $(memoryview_sources)
libnemiveruicommon_la_LIBADD= @NEMIVERUICOMMON_LIBS@ -lutil
libnemiveruicommon_la_CFLAGS= -fPIC -DPIC

AM_CPPFLAGS=@NEMIVERUICOMMON_CFLAGS@ \
-I$(abs_top_srcdir) \
-I$(abs_top_srcdir)/src \
-I$(abs_top_srcdir)/src/confmgr \
-I$(abs_top_srcdir)/src/uicommon \
-I$(abs_top_srcdir)/src/workbench
