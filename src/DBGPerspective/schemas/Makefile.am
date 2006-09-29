files=nemiver-dbgperspective.schemas

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(files)

GCONFTOOL=@GCONFTOOL@

export GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE)
export files

install-schemas:
	for s in `echo $$files | tr ' ' '\n'` ; do $(GCONFTOOL) --makefile-install-rule $(top_srcdir)/src/DBGPerspective/schemas/$$s ; done && killall -HUP gconfd-2


uninstall-schemas:
	for s in `echo $$files | tr ' ' '\n'` ; do $(GCONFTOOL) --makefile-uninstall-rule $(top_srcdir)/src/DBGPerspective/schemas/$$s ; done

EXTRA_DIST=$(files)

