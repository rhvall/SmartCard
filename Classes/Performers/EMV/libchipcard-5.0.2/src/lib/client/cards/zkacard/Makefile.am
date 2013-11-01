
DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)

EXTRA_DIST=app.xml card.xml

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I../

typefiles=types.xml


noinst_LTLIBRARIES=libcards-zkacard.la
noinst_HEADERS=\
  zkacard_p.h


iheaderdir=@libchipcard_headerdir_am@/cards
iheader_HEADERS=\
  zkacard.h

libcards_zkacard_la_SOURCES=\
  zkacard.c


types: $(typefiles)
	typemaker -d CHIPCARD_API $(typefiles)



appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards

install-app-data:
	$(INSTALL) -d $(DESTDIR)$(appdatadir)
	cp app.xml $(DESTDIR)$(appdatadir)/zkacard.xml

install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp card.xml $(DESTDIR)$(carddatadir)/zkacard.xml

install-data-local: install-app-data install-card-data

