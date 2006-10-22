PLUGIN_NAME=DBGPerspective

gdbmod_LTLIBRARIES=libgdbmod.la
gdbmoddir=@NEMIVER_MODULES_DIR@

headers= \
nmv-i-debugger.h

libgdbmod_la_SOURCES= \
$(headers) \
nmv-gdb-engine.cc

publicheaders_DATA=$(headers)
publicheadersdir=$(prefix)/include/nemiver/dbgengine

libgdbmod_la_LDFLAGS=@NEMIVER_LIBS@ -module -avoid-version

config_DATA=gdbengine.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

INCLUDES=@NEMIVER_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src/common

EXTRA_DIST=gdbengine.conf.in

