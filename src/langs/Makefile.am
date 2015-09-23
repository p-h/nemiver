noinst_LTLIBRARIES=libnemivercparser.la

libnemivercparser_la_SOURCES= \
nmv-cpp-lexer.h \
nmv-cpp-lexer.cc \
nmv-cpp-lexer-utils.cc \
nmv-cpp-lexer-utils.h \
nmv-cpp-ast.h \
nmv-cpp-ast.cc \
nmv-cpp-ast-utils.h \
nmv-cpp-ast-utils.cc \
nmv-cpp-parser.h \
nmv-cpp-parser.cc

libnemivercparser_la_LIBADD=$(abs_top_builddir)/src/common/libnemivercommon.la
libnemivercparser_la_CFLAGS= -fPIC -DPIC

AM_CPPFLAGS = 	\
-I$(abs_top_srcdir) \
-I$(abs_top_srcdir)/src \
@NEMIVERCOMMON_CFLAGS@

