INCLUDES=-I$(libchipcard_symlinkdir) $(pcsc_includes) $(gwenhywfar_includes) -I.. -I$(srcdir)/.. -I$(top_srcdir) @gwenhywfar_includes@

noinst_LTLIBRARIES=libbase.la

EXTRA_DIST=\
  card_iso.c \
  client_cmd.c \
  client_xml.c \
  README.apps README.cards


DEFS += \
  -DBUILDING_CHIPCARD \
  -DLC_CLIENT_CONFIG_DIR=\"@lc_cfg_searchdir@\" \
  -DLC_CLIENT_CONFIG_FILE=\"@lc_client_config_file@\" \
  -DLC_CLIENT_XML_DIR=\"@lc_data_searchdir@\"

AM_CFLAGS=$(visibility_cflags)

iheaderdir=@libchipcard_headerdir_am@
iheader_HEADERS=\
 client.h \
 card.h card_imp.h card_l.h \
 tlv.h

noinst_HEADERS=\
 client_l.h client_p.h \
 card_p.h \
 tlv_p.h

libbase_la_SOURCES=\
 client.c \
 card.c \
 tlv.c

appdatadir=@lc_client_xml_dir@/apps
carddatadir=@lc_client_xml_dir@/cards

install-app-data:
	$(INSTALL) -d $(DESTDIR)$(appdatadir)
	cp README.apps $(DESTDIR)$(appdatadir)/README

install-card-data:
	$(INSTALL) -d $(DESTDIR)$(carddatadir)
	cp README.cards $(DESTDIR)$(carddatadir)/README

install-data-local: install-app-data install-card-data

