lib_LTLIBRARIES=libnemiveruicommon.la

public_headers= \
nmv-ui-utils.h

libnemiveruicommon_la_SOURCES= \
$(public_headers) \
nmv-ui-utils.cc

libnemiveruicommon_la_LIBADD=@NEMIVERUICOMMON_LIBS@ \
$(top_builddir)/src/common/libnemivercommon.la

uicommonheaders_DATA=$(public_headers)
uicommonheadersdir=$(NEMIVER_INCLUDE_DIR)/uicommon

INCLUDES=@NEMIVERUICOMMON_CFLAGS@ \
-I$(top_srcdir) \
-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/uicommon

