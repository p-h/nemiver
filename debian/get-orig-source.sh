#! /bin/sh

LOCATION="http://ftp.acc.umu.se/pub/GNOME/sources/nemiver"
PACKAGE="nemiver"
VERSION=$(head -n 1 debian/changelog | sed -e 's/^[^(]*(\([^)]*\)-.*).*/\1/')
MAJOR=$(echo $VERSION | sed -e 's/^\(.*\)\..*/\1/')

echo  "Downloading $LOCATION/$MAJOR/$PACKAGE-$VERSION.tar.gz"

wget -q -O $PACKAGE\_$VERSION.orig.tar.gz \
	   $LOCATION/$MAJOR/$PACKAGE-$VERSION.tar.gz

echo Upstream archive can be found at $PACKAGE\_$VERSION.orig.tar.gz
