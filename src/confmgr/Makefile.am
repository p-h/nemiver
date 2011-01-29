gconfmgrmod_LTLIBRARIES=libgconfmgrmod.la
gconfmgrmoddir=@NEMIVER_MODULES_DIR@

h=$(abs_srcdir)

public_headers=$(h)/nmv-i-conf-mgr.h
private_headers=$(h)/nmv-conf-keys.h
headers= $(public_headers) $(private_headers)

libgconfmgrmod_la_SOURCES= \
$(headers) \
$(h)/nmv-gconf-mgr.cc \
$(h)/nmv-conf-keys.cc

publicheaders_DATA=$(public_headers)
publicheadersdir=$(NEMIVER_INCLUDE_DIR)/dynmods

libgconfmgrmod_la_LDFLAGS=-module -avoid-version -Wl,--as-needed
libgconfmgrmod_la_LIBADD=@NEMIVERCOMMON_LIBS@ @GCONF_LIBS@ \
        $(abs_top_builddir)/src/common/libnemivercommon.la

config_DATA=gconfmgr.conf
configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

INCLUDES=@NEMIVERCOMMON_CFLAGS@ @GCONF_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(abs_top_srcdir)/src

EXTRA_DIST=gconfmgr.conf.in

