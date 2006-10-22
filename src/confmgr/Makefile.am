gconfmgrmod_LTLIBRARIES=libgconfmgrmod.la
gconfmgrmoddir=@NEMIVER_MODULES_DIR@

headers= nmv-i-conf-mgr.h

libgconfmgrmod_la_SOURCES= \
$(headers) \
nmv-gconf-mgr.cc

publicheaders_DATA=$(headers)
publicheadersdir=$(prefix)/include/nemiver/confmgr

libgconfmgrmod_la_LDFLAGS=@NEMIVER_LIBS@ -module -avoid-version

config_DATA=gconfmgr.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

INCLUDES=@NEMIVER_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src/common

EXTRA_DIST=gconfmgr.conf.in

