SUBDIRS=ui menus icons sqlscripts

plugin_config_files= \
plugin-descriptor.xml \
dbgperspective.conf

EXTRA_DIST=$(plugin_config_files)

pluginconfig_DATA=$(plugin_config_files)

PLUGIN_NAME=dbgperspective
pluginconfigdir=@NEMIVER_PLUGINS_DIR@/$(PLUGIN_NAME)

plugin_LTLIBRARIES=libdbgperspectiveplugin.la
plugindir=@NEMIVER_PLUGINS_DIR@/$(PLUGIN_NAME)
sources= \
nmv-run-program-dialog.cc \
nmv-run-program-dialog.h \
nmv-load-core-dialog.cc \
nmv-load-core-dialog.h \
nmv-saved-sessions-dialog.h \
nmv-saved-sessions-dialog.cc \
nmv-preferences-dialog.h \
nmv-preferences-dialog.cc \
nmv-proc-list-dialog.cc \
nmv-proc-list-dialog.h \
nmv-expr-inspector-dialog.cc \
nmv-expr-inspector-dialog.h \
nmv-find-text-dialog.cc \
nmv-find-text-dialog.h \
nmv-sess-mgr.cc \
nmv-sess-mgr.h \
nmv-call-stack.cc \
nmv-call-stack.h \
nmv-variables-utils.cc \
nmv-variables-utils.h \
nmv-local-vars-inspector.cc \
nmv-local-vars-inspector.h \
nmv-global-vars-inspector-dialog.cc \
nmv-global-vars-inspector-dialog.h \
nmv-expr-inspector.cc \
nmv-expr-inspector.h \
nmv-expr-monitor.cc \
nmv-expr-monitor.h \
nmv-breakpoints-view.cc \
nmv-breakpoints-view.h \
nmv-registers-view.cc \
nmv-registers-view.h \
nmv-thread-list.h \
nmv-thread-list.cc \
nmv-file-list.cc \
nmv-file-list.h \
nmv-open-file-dialog.cc \
nmv-open-file-dialog.h \
nmv-set-breakpoint-dialog.cc \
nmv-set-breakpoint-dialog.h \
nmv-watchpoint-dialog.h \
nmv-watchpoint-dialog.cc \
nmv-choose-overloads-dialog.cc \
nmv-choose-overloads-dialog.h \
nmv-remote-target-dialog.cc \
nmv-remote-target-dialog.h \
nmv-dbg-perspective.cc \
nmv-dbg-perspective.h \
nmv-vars-treeview.h \
nmv-vars-treeview.cc \
nmv-call-function-dialog.h \
nmv-call-function-dialog.cc \
nmv-set-jump-to-dialog.h \
nmv-set-jump-to-dialog.cc \
nmv-dbg-perspective-default-layout.cc \
nmv-dbg-perspective-default-layout.h \
nmv-dbg-perspective-two-pane-layout.cc \
nmv-dbg-perspective-two-pane-layout.h \
nmv-dbg-perspective-wide-layout.cc \
nmv-dbg-perspective-wide-layout.h

if BUILD_DYNAMICLAYOUT
dynamiclayout_sources = \
nmv-dbg-perspective-dynamic-layout.cc \
nmv-dbg-perspective-dynamic-layout.h
else
dynamiclayout_sources =
endif

if BUILD_MEMORYVIEW
memoryview_sources = \
nmv-memory-view.cc \
nmv-memory-view.h
else
memoryview_sources =
endif

libdbgperspectiveplugin_la_SOURCES=$(sources) $(memoryview_sources) \
$(dynamiclayout_sources)
libdbgperspectiveplugin_la_LDFLAGS= -module -avoid-version -Wl,--as-needed
libdbgperspectiveplugin_la_LIBADD= \
@NEMIVERDBGPERSP_LIBS@ \
$(abs_top_builddir)/src/common/libnemivercommon.la \
$(abs_top_builddir)/src/uicommon/libnemiveruicommon.la \
$(abs_top_builddir)/src/dbgengine/libdebuggerutils.la

AM_CPPFLAGS=@NEMIVERDBGPERSP_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(abs_top_srcdir)/src \
-I$(abs_top_srcdir)/src/confmgr \
-I$(abs_top_srcdir)/src/dbgengine \
-I$(abs_top_srcdir)/src/uicommon \
-I$(abs_top_srcdir)/src/workbench \
-I$(abs_top_srcdir)/src/persp \
-I$(abs_top_srcdir)/src/dbgperspective
