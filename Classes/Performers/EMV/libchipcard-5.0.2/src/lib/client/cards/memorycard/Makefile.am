
DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)


INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I../

EXTRA_DIST=app.xml card.xml


noinst_LTLIBRARIES=libcards-memorycard.la
noinst_HEADERS=\
  memorycard_p.h


iheaderdir=@libchipcard_headerdir_am@/cards
iheader_HEADERS=\
  memorycard.h

libcards_memorycard_la_SOURCES=\
  memorycard.c

appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards

install-app-data:
	$(INSTALL) -d $(DESTDIR)$(appdatadir)
	cp app.xml $(DESTDIR)$(appdatadir)/memorycard.xml

install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp card.xml $(DESTDIR)$(carddatadir)/memorycard.xml

install-data-local: install-app-data install-card-data

