
DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)

EXTRA_DIST=app.xml card.xml types.xml

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I../

typefiles=types.xml


noinst_LTLIBRARIES=libcards-geldkarte.la
noinst_HEADERS=\
  geldkarte_p.h \
  geldkarte_blog_l.h \
  geldkarte_blog_p.h \
  geldkarte_llog_l.h \
  geldkarte_llog_p.h \
  geldkarte_values_l.h \
  geldkarte_values_p.h


iheaderdir=@libchipcard_headerdir_am@/cards
iheader_HEADERS=\
  geldkarte.h \
  geldkarte_blog.h \
  geldkarte_llog.h \
  geldkarte_values.h

libcards_geldkarte_la_SOURCES=\
  geldkarte.c \
  geldkarte_blog.c \
  geldkarte_llog.c \
  geldkarte_values.c

types: $(typefiles)
	typemaker -d CHIPCARD_API $(typefiles)

appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards

install-app-data:
	$(INSTALL) -d $(DESTDIR)$(appdatadir)
	cp app.xml $(DESTDIR)$(appdatadir)/geldkarte.xml

install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp card.xml $(DESTDIR)$(carddatadir)/geldkarte.xml

install-data-local: install-app-data install-card-data


