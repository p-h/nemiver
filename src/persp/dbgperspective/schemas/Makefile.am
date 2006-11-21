files=nemiver-dbgperspective.schemas

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(files)

GCONFTOOL=@GCONFTOOL@

export GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE)
export files

install-schemas:
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for s in `echo $$files | tr ' ' '\n'` ; do \
	$(GCONFTOOL) --makefile-install-rule $(top_srcdir)/src/persp/dbgperspective/schemas/$$s ; done && if test x$$NO_RESTART_GCONF = x ; then $(GCONFTOOL) --shutdown; fi ; \
	fi
endif


uninstall-schemas:
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for s in `echo $$files | tr ' ' '\n'` ; do $(GCONFTOOL) --makefile-uninstall-rule $(top_srcdir)/src/persp/dbgperspective/schemas/$$s ; done \
	fi
endif

EXTRA_DIST=$(files)

