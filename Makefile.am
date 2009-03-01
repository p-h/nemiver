SUBDIRS = src glade menus tests po data docs help
ACLOCAL_AMFLAGS = -I m4
headers= config.h

publicheaders_DATA=$(headers)
publicheadersdir=$(NEMIVER_INCLUDE_DIR)/common

INTLTOOL_FILES= intltool-extract.in \
intltool-merge.in \
intltool-update.in

EXTRA_DIST= \
README \
INSTALL \
COPYING \
COPYRIGHT \
$(headers) \
$(INTLTOOL_FILES) \
gnome-doc-utils.make

doc:
	$(MAKE) -C docs doc

###############################
# RELEASE TARGETS
###############################
# Add the following lines to your ~/.ssh/config file if necessary so you don't
# need to specify your username manually:
#
# Host *.gnome.org
# 	User your-gnome-username

RELEASE_UPLOAD_HOST = master.gnome.org
RELEASE_UPLOAD_BASE = /home/users/$(USER)
RELEASE_URL_BASE = http://ftp.gnome.org/pub/GNOME/sources/nemiver/
RELEASE_ANNOUNCE_LIST = nemiver-list@gnome.org (cc gnome-announce-list@gnome.org)
tar_gz_file = $(PACKAGE)-$(VERSION).tar.gz
sig_file = $(tar_gz_file).sig

# sign the release with gpg
$(sig_file): $(tar_gz_file)
	gpg --output $@ --detach $^

RELEASED_BRANCH=master

echo-released-branch:
	echo "released-branch=$(RELEASED_BRANCH)"

tag-release: dist tag-release-only
tag-release-only:
	git tag -m "$(PACKAGE_NAME) release $(VERSION)" \
	"$(PACKAGE_NAME)-$(VERSION)" $(RELEASED_BRANCH)

release-upload: tag-release $(tar_gz_file) release-upload-only
release-upload-only:
	scp $(tar_gz_file) $(RELEASE_UPLOAD_HOST):$(RELEASE_UPLOAD_BASE)/

release : release-upload cat-announcement
cat-announcement:
	@echo ""
	@echo "Please send an announcement to $(RELEASE_ANNOUNCE_LIST)"
	@echo "including the following:"
	@echo ""
	@echo "Subject: $(PACKAGE_NAME) release $(VERSION) now available"
	@echo ""
	@echo "============================== CUT HERE =============================="
	@echo "A new $(PACKAGE_NAME) release $(VERSION) is now available."
	@echo ""
	@echo "What is Nemiver?"
	@echo "==========="
	@echo "$(PACKAGE_NAME) is a standalone graphical debugger that integrates well in the GNOME desktop environment."
	@echo "It currently features a backend which uses the GNU debugger gdb for debugging C/C++ programs."
	@echo ""
	@echo "Where to get it?"
	@echo "================"
	@echo "This release is available as a source package in tar.gz format"
	@echo "and can be downloaded from: $(RELEASE_URL_BASE)"
	@echo ""
	@echo "The home page of the Nemiver project is at http://projects.gnome.org/projects/nemiver"
	@echo ""
	@echo "What's Changed in this release?"
	@echo "================================"
	@echo ""
	@echo "============================== CUT HERE =============================="
	@echo "Also, please include the new entries from the NEWS file."
	@echo ""
	@echo "Make sure to update the Website with the new version."
	@echo ""
	@echo "Last but not least, do not forget to bump up the micro"
	@echo "version component to the next (odd) number and commit."

DISTCLEANFILES = $(INTLTOOL_FILES:.in=) \
	gnome-doc-utils.make

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper
