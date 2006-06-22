lib_LTLIBRARIES=libnemivercommon.la

headers= \
nmv-api-macros.h \
nmv-ustring.h \
nmv-libxml-utils.h \
nmv-safe-ptr-utils.h \
nmv-object.h \
nmv-safe-ptr.h \
nmv-log-stream.h \
nmv-scope-logger.h \
nmv-exception.h \
nmv-initializer.h \
nmv-dynamic-module.h \
nmv-log-stream-utils.h \
nmv-date-utils.h \
nmv-i-input-stream.h \
nmv-env.h \
nmv-plugin.h \
nmv-sequence.h

libnemivercommon_la_SOURCES= $(headers) \
nmv-ustring.cc \
nmv-object.cc \
nmv-libxml-utils.cc \
nmv-log-stream.cc \
nmv-scope-logger.cc \
nmv-exception.cc \
nmv-initializer.cc \
nmv-dynamic-module.cc \
nmv-date-utils.cc \
nmv-env.cc \
nmv-plugin.cc \
nmv-sequence.cc

publicheaders_DATA=$(headers)
publicheadersdir=$(prefix)/include/libnemivercommon
libnemivercommon_la_LDFLAGS=@NEMIVER_LIBS@

INCLUDES=@NEMIVER_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-DNEMIVER_INSTALL_PREFIX=\"@NEMIVER_INSTALL_PREFIX@\" \
-DNEMIVER_SYSTEM_CONFIG_DIR=\"@NEMIVER_SYSTEM_CONFIG_DIR@\" \
-DNEMIVER_MODULES_DIR=\"@NEMIVER_MODULES_DIR@\" \
-DNEMIVER_PLUGINS_DIR=\"@NEMIVER_PLUGINS_DIR@\" \
-DNEMIVER_VERSION=\"@NEMIVER_VERSION@\" \
-DGDB_PROG=\"@GDB_PROG@\"

