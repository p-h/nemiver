SUBDIRS = src glade menus tests po

desktopdir = $(datadir)/applications
desktop_in_files = nemiver.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST= \
README \
INSTALL \
COPYING \
COPYRIGHT \
sed.sh \
intltool-extract.in \
intltool-merge.in \
intltool-update.in \
$(desktop_in_files)

