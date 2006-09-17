bin_PROGRAMS=nmvterm
nmvterm_SOURCES=nmv-terminal.cc
nmvterm_LDADD= \
@NEMIVER_LIBS@  \
$(top_builddir)/src/common/libnemivercommon.la \
$(top_builddir)/src/uicommon/libnemiveruicommon.la

INCLUDES=\
-DNEMIVERLOCALEDIR=\"$(nemiverlocaledir)\" \
@NEMIVER_CFLAGS@ \
-I$(top_srcdir)/src/common \
-I$(top_srcdir)/src/uicommon

