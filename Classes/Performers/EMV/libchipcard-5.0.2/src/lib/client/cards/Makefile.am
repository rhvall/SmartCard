SUBDIRS=memorycard processorcard ddvcard geldkarte kvkcard egkcard starcos zkacard

DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)

typefiles=hidata.xml


EXTRA_DIST=card.xml $(typefiles)

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I../ -I../client -I$(srcdir)/../ -I$(srcdir)/../client

noinst_LTLIBRARIES=libcards.la

libcards_la_SOURCES=\
  hiinsurancedata.c \
  hipersonaldata.c

iheaderdir=@libchipcard_headerdir_am@/cards
iheader_HEADERS=\
  hiinsurancedata.h \
  hipersonaldata.h

noinst_HEADERS=\
  hiinsurancedata_l.h \
  hiinsurancedata_p.h \
  hipersonaldata_l.h \
  hipersonaldata_p.h

libcards_la_LIBADD=\
  ddvcard/libcards-ddv.la \
  egkcard/libcards-egk.la \
  geldkarte/libcards-geldkarte.la \
  kvkcard/libcards-kvk.la \
  starcos/libcards-starcos.la \
  zkacard/libcards-zkacard.la \
  memorycard/libcards-memorycard.la \
  processorcard/libcards-processorcard.la

appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards


install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp card.xml $(DESTDIR)$(carddatadir)/card.xml

install-data-local: install-card-data

types: $(typefiles)
	typemaker -d CHIPCARD_API $(typefiles)

