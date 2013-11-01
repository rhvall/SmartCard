bin_PROGRAMS=kvkcard

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) 

EXTRA_DIST=README read.c rdvd.c rdpd.c psvd.c

noinst_HEADERS=global.h

kvkcard_SOURCES=\
  main.c
kvkcard_LDADD=../../lib/client/libchipcard.la $(mingw_ldflags) @gwenhywfar_libs@
kvkcard_LDFLAGS=@STRIPALL@

if IS_WINDOWS
kvkcard_LDFLAGS+=-mconsole
endif


MAINTAINERCLEANFILES=Makefile.in

