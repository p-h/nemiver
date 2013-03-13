noinst_LTLIBRARIES=libnemivercparser.la

h=$(abs_srcdir)

libnemivercparser_la_SOURCES= \
$(h)/nmv-cpp-lexer.h \
$(h)/nmv-cpp-lexer.cc \
$(h)/nmv-cpp-lexer-utils.cc \
$(h)/nmv-cpp-lexer-utils.h \
$(h)/nmv-cpp-ast.h \
$(h)/nmv-cpp-ast.cc \
$(h)/nmv-cpp-ast-utils.h \
$(h)/nmv-cpp-ast-utils.cc \
$(h)/nmv-cpp-parser.h \
$(h)/nmv-cpp-parser.cc

libnemivercparser_la_LIBADD=$(abs_top_builddir)/src/common/libnemivercommon.la
libnemivercparser_la_CFLAGS= -fPIC -DPIC

AM_CPPFLAGS = 	\
-I$(abs_top_srcdir) \
-I$(abs_top_srcdir)/src \
@NEMIVERCOMMON_CFLAGS@

