
DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)


INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I../

EXTRA_DIST=app.xml card.xml


noinst_LTLIBRARIES=libcards-egk.la
noinst_HEADERS=\
  egkcard_p.h


iheaderdir=@libchipcard_headerdir_am@/cards
iheader_HEADERS=\
  egkcard.h

libcards_egk_la_SOURCES=\
  egkcard.c

appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards

install-app-data:
	$(INSTALL) -d $(DESTDIR)$(appdatadir)
	cp app.xml $(DESTDIR)$(appdatadir)/egk.xml

install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp card.xml $(DESTDIR)$(carddatadir)/egk.xml

install-data-local: install-app-data install-card-data

