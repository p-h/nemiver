sqlitedriver_LTLIBRARIES = libnemiverdbdsqlite.la
sqlitedriverdir=@NEMIVER_MODULES_DIR@

sources=nmv-sqlite-cnx-mgr-drv.cc \
nmv-sqlite-cnx-mgr-drv.h \
nmv-sqlite-cnx-drv.cc \
nmv-sqlite-cnx-drv.h \
nmv-libsqlite-macros.h

libnemiverdbdsqlite_la_SOURCES=$(sources)

libnemiverdbdsqlite_la_LDFLAGS= -module -avoid-version
libnemiverdbdsqlite_la_LIBADD= \
@SQLITE3_LIBS@ @NEMIVER_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

INCLUDES=@NEMIVER_CFLAGS@ -I$(top_srcdir)/src/common \
-DIN_LIBSQLITE_DSO

config_DATA=org.nemiver.db.sqlitedriver.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

