
INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I.. -I$(srcdir)/.. 

DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)

noinst_LTLIBRARIES=liblcct.la

iheaderdir=@libchipcard_headerdir_am@/ct
iheader_HEADERS=\
  ct_card.h

liblcct_la_SOURCES=\
  ct_card.c

