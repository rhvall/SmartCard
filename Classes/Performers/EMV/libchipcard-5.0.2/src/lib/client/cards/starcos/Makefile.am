
DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)

EXTRA_DIST=app-hvb.xml app-vr.xml app.xml card.xml types.xml

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I../

typefiles=types.xml


noinst_LTLIBRARIES=libcards-starcos.la
noinst_HEADERS=\
  starcos_p.h \
  starcos_keydescr_l.h \
  starcos_keydescr_p.h


iheaderdir=@libchipcard_headerdir_am@/cards
iheader_HEADERS=\
  starcos.h \
  starcos_keydescr.h

libcards_starcos_la_SOURCES=\
  starcos.c \
  starcos_keydescr.c


types: $(typefiles)
	typemaker -d CHIPCARD_API $(typefiles)



appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards

install-app-data:
	$(INSTALL) -d $(DESTDIR)$(appdatadir)
	cp app.xml $(DESTDIR)$(appdatadir)/starcoscard.xml
	cp app-hvb.xml $(DESTDIR)$(appdatadir)/starcoscard-hvb.xml
	cp app-vr.xml $(DESTDIR)$(appdatadir)/starcoscard-vr.xml

install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp card.xml $(DESTDIR)$(carddatadir)/starcoscard.xml

install-data-local: install-app-data install-card-data

