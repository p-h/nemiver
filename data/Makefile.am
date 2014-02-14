SUBDIRS=icons schemas

desktopdir = $(datadir)/applications
desktop_in_files = nemiver.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_XML_RULE@
appdatadir = $(datadir)/appdata
dist_appdata_DATA = $(appdata_in_files:.xml.in=.xml)
appdata_in_files = nemiver.appdata.xml.in

CLEANFILES =			\
	nemiver.appdata.xml


man_MANS = nemiver.1

EXTRA_DIST= \
$(desktop_in_files) \
$(man_MANS)

DISTCLEANFILES = $(desktop_DATA)
