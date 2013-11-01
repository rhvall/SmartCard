INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) -I.. -I../.. -I../../.. -I$(top_builddir)

typefiles=pininfo.xml

DEFS += -DBUILDING_CHIPCARD

#AM_CFLAGS=$(visibility_cflags)

EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libsharedstuff.la

iheaderdir=@libchipcard_headerdir_am@/sharedstuff
iheader_HEADERS=\
  driverinfo.h \
  msgengine.h \
  misc.h \
  pininfo.h

noinst_HEADERS=\
  driverinfo_p.h \
  msgengine_l.h \
  msgengine_p.h \
  pininfo_l.h \
  pininfo_p.h

libsharedstuff_la_SOURCES=\
  driverinfo.c \
  misc.c \
  msgengine.c \
  pininfo.c


types: $(typefiles)
	$(gwenhywfar_dir)/bin/typemaker -d CHIPCARD_API $(typefiles)


MAINTAINERCLEANFILES=Makefile.in

