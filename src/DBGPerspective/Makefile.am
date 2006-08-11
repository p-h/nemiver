SUBDIRS=glade menus icons dbgengine sqlscripts

plugin_config_files= \
plugin-descriptor.xml \
DBGPerspective.conf

EXTRA_DIST=$(plugin_config_files)

pluginconfig_DATA=$(plugin_config_files)

PLUGIN_NAME=DBGPerspective
pluginconfigdir=@NEMIVER_PLUGINS_DIR@/$(PLUGIN_NAME)

plugin_LTLIBRARIES=libdbgperspectiveplugin.la
plugindir=@NEMIVER_PLUGINS_DIR@/$(PLUGIN_NAME)
sources= \
nmv-dbg-perspective.cc \
nmv-dbg-perspective.h \
nmv-source-editor.cc \
nmv-source-editor.h \
nmv-run-program-dialog.cc \
nmv-run-program-dialog.h \
nmv-sess-mgr.cc \
nmv-sess-mgr.h \
nmv-proc-mgr.cc \
nmv-proc-mgr.h \
nmv-proc-list-dialog.cc \
nmv-proc-list-dialog.h

libdbgperspectiveplugin_la_SOURCES=$(sources)
libdbgperspectiveplugin_la_LIBADD= \
@NEMIVER_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

INCLUDES=@NEMIVER_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/DBGPerspective \
-I$(top_srcdir)/src/DBGPerspective/dbgengine \
-I$(top_srcdir)/src/uicommon \
-I$(top_srcdir)/src/workbench

