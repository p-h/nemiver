public_headers=nmv-i-conf-mgr.h
private_headers=nmv-conf-keys.h
headers= $(public_headers) $(private_headers)

publicheaders_DATA=$(public_headers)
publicheadersdir=$(NEMIVER_INCLUDE_DIR)/dynmods

configdir=@NEMIVER_SYSTEM_CONFIG_DIR@

AM_CPPFLAGS=@NEMIVERCOMMON_CFLAGS@ -DENABLE_NLS=1 -DDATADIR=\"${datadir}\" \
-I$(abs_top_srcdir)/src

if BUILD_GSETTINGS
    libgsettingsmgrmod_la_SOURCES=$(headers) nmv-gsettings-mgr.cc\
            nmv-gsettings-keys-defs.cc
    libgsettingsmgrmod_la_LDFLAGS=-module -avoid-version -Wl,--as-needed
    libgsettingsmgrmod_la_LIBADD=@NEMIVERCOMMON_LIBS@ @NEMIVERGSETTINGS_LIBS@ \
            $(abs_top_builddir)/src/common/libnemivercommon.la

    gsettingsmgrmod_LTLIBRARIES=libgsettingsmgrmod.la
    gsettingsmgrmoddir=@NEMIVER_MODULES_DIR@

    config_DATA=gsettingsmgr.conf
    AM_CPPFLAGS+= @NEMIVERGSETTINGS_CFLAGS@
else
    libgconfmgrmod_la_SOURCES=$(headers) nmv-gconf-mgr.cc\
            nmv-gconf-keys-defs.cc
    libgconfmgrmod_la_LDFLAGS=-module -avoid-version -Wl,--as-needed
    libgconfmgrmod_la_LIBADD=@NEMIVERCOMMON_LIBS@ @GCONF_LIBS@ \
            $(abs_top_builddir)/src/common/libnemivercommon.la

    gconfmgrmod_LTLIBRARIES=libgconfmgrmod.la
    gconfmgrmoddir=@NEMIVER_MODULES_DIR@

    config_DATA=gconfmgr.conf
    AM_CPPFLAGS+= @GCONF_CFLAGS@
endif

EXTRA_DIST=gconfmgr.conf.in gsettingsmgr.conf.in
