workbenchmod_LTLIBRARIES=libworkbenchmod.la
workbenchmoddir=@NEMIVER_MODULES_DIR@

headers= \
nmv-i-workbench.h \
nmv-i-perspective.h \
nmv-i-pref-mgr.h

libworkbenchmod_la_SOURCES= \
$(headers) \
nmv-workbench.cc \
nmv-pref-mgr.cc

publicheaders_DATA=$(headers)
publicheadersdir=$(NEMIVER_INCLUDE_DIR)/workbench

libworkbenchmod_la_LDFLAGS=@NEMIVERUICOMMON_LIBS@ -module -avoid-version

config_DATA=workbench.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

INCLUDES=@NEMIVERUICOMMON_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/confmgr \
-I$(top_srcdir)/src/dbgengine \
-I$(top_srcdir)/src/uicommon

EXTRA_DIST=workbench.conf.in

