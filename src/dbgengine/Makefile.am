gdbmod_LTLIBRARIES=libgdbmod.la
gdbmoddir=@NEMIVER_MODULES_DIR@
varlistmod_LTLIBRARIES=libvarlistmod.la
varlistmoddir=@NEMIVER_MODULES_DIR@

idebuggerheaders= \
nmv-i-debugger.h

ivarlistheaders= \
nmv-i-var-list.h

dynmodheaders = $(idebugheaders) $(ivarlistheaders)
libgdbmod_la_SOURCES= \
$(idebuggerheaders)  \
nmv-gdb-engine.cc

libvarlistmod_la_SOURCES= \
$(ivarlistheaders) \
nmv-var-list.cc

publicheaders_DATA=$(dynmodheaders)
publicheadersdir=$(NEMIVER_INCLUDE_DIR)/dynmods

libgdbmod_la_LDFLAGS=@NEMIVERCOMMON_LIBS@ -module -avoid-version

config_DATA=gdbengine.conf varlist.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

INCLUDES=@NEMIVERCOMMON_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src/common

