#!/bin/sh

PROJECT=nemiver

topsrcdir=`dirname $0`
if test x$topsrcdir = x ; then
	topsrcdir=.
fi

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or see http://www.gnu.org/software/autoconf"
	DIE=1
}

(libtool --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have libtool installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or see http://www.gnu.org/software/libtool"
	DIE=1
}

(intltoolize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "  You must have intltool installed to compile $PROJECT."
    echo "  Get the latest version from"
    echo "  ftp://ftp.gnome.org/pub/GNOME/sources/intltool/"
    echo
    DIE=1
}

(glib-gettextize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "  You must have glib-gettextize installed to compile $PROJECT."
    echo "  glib-gettextize is part of glib-2.0, so you should already"
    echo "  have it. Make sure it is in your PATH."
    echo
    DIE=1
}

(automake-1.9 --version) < /dev/null > /dev/null 2>&1 || {
	echo
	DIE=1
	echo "You must have automake installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or see http://www.gnu.org/software/automake"
}

test -f src/common/nmv-object.cc || {
	echo "You must run this script in the top-level Nemiver directory"
	exit 1
}

if test -z "$*"; then
	echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
fi


libtoolize --copy --force || exit $?
glib-gettextize --force --copy || exit $?
intltoolize --force --copy --automake || exit $?
aclocal $ACLOCAL_FLAGS || exit $?
automake-1.9 --add-missing || exit $?
autoconf || exit $?

$topsrcdir/configure "$@"
