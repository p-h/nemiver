PLUGIN_NAME=DBGPerspective

gdbmod_LTLIBRARIES=libgdbmod.la
gdbmoddir=@NEMIVER_PLUGINS_DIR@/$(PLUGIN_NAME)

headers= \
nmv-i-debugger.h

libgdbmod_la_SOURCES= \
$(headers) \
nmv-gdb-engine.cc

libgdbmod_la_LDFLAGS=@NEMIVER_LIBS@

config_DATA=gdbengine.conf
configdir=@NEMIVER_PLUGINS_DIR@/$(PLUGIN_NAME)

INCLUDES=@NEMIVER_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src/common

EXTRA_DIST=gdbengine.conf.in

