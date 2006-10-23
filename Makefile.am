SUBDIRS = src glade menus tests po

desktopdir = $(datadir)/applications
desktop_in_files = nemiver.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
INTLTOOL_FILES= intltool-extract.in \
				intltool-merge.in \
				intltool-update.in

EXTRA_DIST= \
README \
INSTALL \
COPYING \
COPYRIGHT \
$(INTLTOOL_FILES) \
$(desktop_in_files)

###############################
# RELEASE TARGETS
###############################
# Add the following lines to your ~/.ssh/config file if necessary so you don't
# need to specify your username manually:
#
# Host *.gna.org
# 	User your-gna-username

RELEASE_UPLOAD_HOST = download.gna.org
RELEASE_UPLOAD_BASE = /upload/nemiver/
RELEASE_URL_BASE = http://download.gna.org/nemiver/
SVN_ROOT = svn+ssh://svn.gna.org/svn/nemiver
RELEASE_ANNOUNCE_LIST = nemiver-list@gna.org (cc gnome-announce-list@gnome.org)
tar_gz_file = $(PACKAGE)-$(VERSION).tar.gz
sig_file = $(tar_gz_file).sig

# sign the release with gpg
$(sig_file): $(tar_gz_file)
	gpg --output $@ --detach $^

tag-release: distcheck
	svn copy -m "$(PACKAGE_NAME) release $(VERSION)" \
		$(SVN_ROOT)/trunk $(SVN_ROOT)/tags/$(PACKAGE_NAME)-$(VERSION)

release-upload: tag-release $(tar_gz_file) $(sig_file)
	scp $(tar_gz_file) $(RELEASE_UPLOAD_HOST):$(RELEASE_UPLOAD_BASE)/
	scp $(sig_file) $(RELEASE_UPLOAD_HOST):$(RELEASE_UPLOAD_BASE)/

release-publish: release-upload
	@echo ""
	@echo "Please send an announcement to $(RELEASE_ANNOUNCE_LIST)"
	@echo "including the following:"
	@echo ""
	@echo "Subject: $(PACKAGE_NAME) release $(VERSION) now available"
	@echo ""
	@echo "============================== CUT HERE =============================="
	@echo "A new $(PACKAGE_NAME) release $(VERSION) is now available."
	@echo ""
	@echo "What is it?"
	@echo "==========="
	@echo "$(PACKAGE_NAME) front-end for the GNU Debugger for the GNOME Desktop"
	@echo ""
	@echo "Where to get it?"
	@echo "================"
	@echo "This release is available as a source package in tar.gz format"
	@echo "and can be downloaded from:"
	@echo ""
	@echo "$(RELEASE_URL_BASE)/$(tar_gz_file)"
	@echo "    which can be verified with:"
	@echo "$(RELEASE_URL_BASE)/$(sig_file)"
	@echo ""
	@echo "What's Changed?"
	@echo "==============="
	@echo ""
	@echo "============================== CUT HERE =============================="
	@echo "Also, please include the new entries from the NEWS file."
	@echo ""
	@echo "Make sure to update the Website with the new version."
	@echo ""
	@echo "Last but not least, do not forget to bump up the micro"
	@echo "version component to the next (odd) number and commit."

DISTCLEANFILES = $(desktop_DATA) $(INTLTOOL_FILES:.in=)
