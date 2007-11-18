noinst_LTLIBRARIES=libnemivercparser.la

libnemivercparser_la_SOURCES= \
nmv-cpp-lexer.h \
nmv-cpp-lexer.cc \
nmv-cpp-lexer-utils.cc \
nmv-cpp-lexer-utils.h

libnemivercparser_la_LIBADD=$(top_builddir)/src/common/libnemivercommon.la
libnemivercparser_la_CFLAGS= -fPIC -DPIC

INCLUDES= \
-I$(top_srcdir) \
-I$(top_srcdir)/src \
@NEMIVERCOMMON_CFLAGS@

