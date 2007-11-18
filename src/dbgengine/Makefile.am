gdbmod_LTLIBRARIES=libgdbmod.la
gdbmoddir=@NEMIVER_MODULES_DIR@

varlistmod_LTLIBRARIES=libvarlistmod.la
varlistmoddir=@NEMIVER_MODULES_DIR@

varwalkermod_LTLIBRARIES=libvarwalkermod.la
varwalkermoddir=@NEMIVER_MODULES_DIR@

varwalkerlistmod_LTLIBRARIES=libvarwalkerlistmod.la
varwalkerlistmoddir=@NEMIVER_MODULES_DIR@

cpptraitmod_LTLIBRARIES=libcpptraitmod.la
cpptraitmoddir=@NEMIVER_MODULES_DIR@

noinst_LTLIBRARIES=libgdbmiparser.la libdbgcommon.la libgdbengine.la

idebuggerheaders= \
nmv-i-debugger.h

ivarlistheaders= \
nmv-i-var-list.h

ilangtraitheaders= \
nmv-i-lang-trait.h

ivarwalkerheaders= nmv-i-var-walker.h nmv-var.h

ivarwalkerlistheaders= nmv-i-var-walker-list.h

dynmodheaders = \
$(idebuggerheaders) \
$(ivarlistheaders) \
$(ilangtraitheaders) \
$(ivarwalkerheaders)

libgdbmod_la_SOURCES= \
$(idebuggerheaders)

libvarlistmod_la_SOURCES= \
$(ivarlistheaders) \
nmv-var-list.cc

libvarwalkermod_la_SOURCES= \
$(ivarwalkerheaders) \
nmv-var-walker.cc

libvarwalkerlistmod_la_SOURCES= \
$(ivarwalkerlistheaders) \
nmv-var-walker-list.cc

libgdbmiparser_la_SOURCES= \
nmv-gdbmi-parser.cc \
nmv-gdbmi-parser.h

libgdbmiparser_la_CFLAGS=-fPIC -DPIC

libcpptraitmod_la_SOURCES= \
$(ilangtraitheaders) \
nmv-cpp-trait.cc

libdbgcommon_la_SOURCES= \
nmv-dbg-common.cc \
nmv-dbg-common.h

libdbgcommon_la_CFLAGS=-fPIC -DPIC

libgdbengine_la_SOURCES= \
nmv-gdb-engine.cc \
nmv-gdb-engine.h

libgdbengine_la_CFLAGS=-fPIC -DPIC

publicheaders_DATA=$(dynmodheaders)
publicheadersdir=$(NEMIVER_INCLUDE_DIR)/dynmods

libgdbmod_la_LDFLAGS=-module -avoid-version
libgdbmod_la_LIBADD=libdbgcommon.la libgdbmiparser.la libgdbengine.la \
@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

libvarlistmod_la_LDFLAGS=-module -avoid-version
libvarlistmod_la_LIBADD=@NEMIVERCOMMON_LIBS@ \
	$(top_builddir)/src/common/libnemivercommon.la

libcpptraitmod_la_LDFLAGS=-module -avoid-version
libcpptraitmod_la_LIBADD=@NEMIVERCOMMON_LIBS@ \
	$(top_builddir)/src/common/libnemivercommon.la

libgdbmiparser_la_LIBADD= \
@NEMIVERCOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

config_DATA=gdbengine.conf varlist.conf varwalker.conf varwalkerlist.conf cpptrait.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

INCLUDES=@NEMIVERCOMMON_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src -I$(top_srcdir)/src/confmgr

