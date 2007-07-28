gdbmod_LTLIBRARIES=libgdbmod.la
gdbmoddir=@NEMIVER_MODULES_DIR@

varlistmod_LTLIBRARIES=libvarlistmod.la
varlistmoddir=@NEMIVER_MODULES_DIR@

cpptraitmod_LTLIBRARIES=libcpptraitmod.la
cpptraitmoddir=@NEMIVER_MODULES_DIR@

noinst_LTLIBRARIES=libgdbmiparser.la libdbgcommon.la

idebuggerheaders= \
nmv-i-debugger.h

ivarlistheaders= \
nmv-i-var-list.h

ilangtraitheaders= \
nmv-i-lang-trait.h

dynmodheaders = $(idebuggerheaders) $(ivarlistheaders) $(ilangtraitheaders)

libgdbmod_la_SOURCES= \
$(idebuggerheaders)  \
nmv-gdb-engine.cc

libvarlistmod_la_SOURCES= \
$(ivarlistheaders) \
nmv-var-list.cc

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

publicheaders_DATA=$(dynmodheaders)
publicheadersdir=$(NEMIVER_INCLUDE_DIR)/dynmods

libgdbmod_la_LDFLAGS=-module -avoid-version
libgdbmod_la_LIBADD=libdbgcommon.la libgdbmiparser.la @NEMIVERCOMMON_LIBS@ \
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

config_DATA=gdbengine.conf varlist.conf cpptrait.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

INCLUDES=@NEMIVERCOMMON_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(top_srcdir)/src

