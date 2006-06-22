lib_LTLIBRARIES=libnemiveruicommon.la

public_headers= \
nmv-ui-utils.h

libnemiveruicommon_la_SOURCES= \
$(public_headers) \
nmv-ui-utils.cc

libnemiveruicommon_la_LIBADD=@NEMIVER_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

uicommonheaders_DATA=$(public_headers)
uicommonheadersdir=$(prefix)/include/uicommon

INCLUDES=@NEMIVER_CFLAGS@ \
-I$(top_srcdir) \
-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/uicommon

