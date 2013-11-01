
DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)


INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I../

EXTRA_DIST=\
  app.xml app-ddv0.xml app-ddv1.xml \
  card-ddv0.xml card-ddv1.xml


noinst_LTLIBRARIES=libcards-ddv.la
noinst_HEADERS=\
  ddvcard_p.h


iheaderdir=@libchipcard_headerdir_am@/cards
iheader_HEADERS=\
  ddvcard.h

libcards_ddv_la_SOURCES=\
  ddvcard.c

appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards

install-app-data:
	$(INSTALL) -d $(DESTDIR)$(appdatadir)
	cp app.xml $(DESTDIR)$(appdatadir)/ddv.xml
	cp app-ddv0.xml $(DESTDIR)$(appdatadir)/ddv0.xml
	cp app-ddv1.xml $(DESTDIR)$(appdatadir)/ddv1.xml

install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp card-ddv0.xml $(DESTDIR)$(carddatadir)/ddv0.xml
	cp card-ddv1.xml $(DESTDIR)$(carddatadir)/ddv1.xml

install-data-local: install-app-data install-card-data

