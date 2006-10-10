SUBDIRS=glade menus icons sqlscripts schemas

install-data-local: install-schemas

uninstall-local: uninstall-schemas

install-schemas:
	$(MAKE) -C schemas $(AM_MAKEFLAGS) install-schemas

uninstall-schemas:
	$(MAKE) -C schemas $(AM_MAKEFLAGS)  uninstall-schemas

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
nmv-dialog.cc \
nmv-dialog.h \
nmv-run-program-dialog.cc \
nmv-run-program-dialog.h \
nmv-load-core-dialog.cc \
nmv-load-core-dialog.h \
nmv-saved-sessions-dialog.h \
nmv-saved-sessions-dialog.cc \
nmv-preferences-dialog.h \
nmv-preferences-dialog.cc \
nmv-sess-mgr.cc \
nmv-sess-mgr.h \
nmv-proc-mgr.cc \
nmv-proc-mgr.h \
nmv-proc-list-dialog.cc \
nmv-proc-list-dialog.h \
nmv-call-stack.cc \
nmv-call-stack.h \
nmv-throbber.cc \
nmv-throbber.h \
nmv-vars-editor.cc \
nmv-vars-editor.h \
nmv-terminal.h \
nmv-terminal.cc \
nmv-breakpoints-view.cc \
nmv-breakpoints-view.h \
nmv-popup-tip.h \
nmv-popup-tip.cc

libdbgperspectiveplugin_la_SOURCES=$(sources)
libdbgperspectiveplugin_la_LDFLAGS= -module -avoid-version
libdbgperspectiveplugin_la_LIBADD= \
@NEMIVER_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

INCLUDES=@NEMIVER_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/confmgr \
-I$(top_srcdir)/src/dbgengine \
-I$(top_srcdir)/src/uicommon \
-I$(top_srcdir)/src/workbench \
-I$(top_srcdir)/src/DBGPerspective

