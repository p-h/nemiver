sqlitedriver_LTLIBRARIES = libnemiverdbdsqlite.la
sqlitedriverdir=@NEMIVER_MODULES_DIR@

h=$(abs_srcdir)

sources=\
$(h)/nmv-sqlite-cnx-mgr-drv.cc \
$(h)/nmv-sqlite-cnx-mgr-drv.h \
$(h)/nmv-sqlite-cnx-drv.cc \
$(h)/nmv-sqlite-cnx-drv.h

libnemiverdbdsqlite_la_SOURCES=$(sources)

libnemiverdbdsqlite_la_LDFLAGS= -module -avoid-version -Wl,--as-needed
libnemiverdbdsqlite_la_LIBADD= \
@SQLITE3_LIBS@ @NEMIVERCOMMON_LIBS@ \
$(abs_top_builddir)/src/common/libnemivercommon.la

AM_CPPFLAGS=@SQLITE3_CFLAGS@ @NEMIVERCOMMON_CFLAGS@ -I$(abs_top_srcdir)/src \
-DIN_LIBSQLITE_DSO

config_DATA=org.nemiver.db.sqlitedriver.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

