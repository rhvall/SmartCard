bin_PROGRAMS=geldkarte

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) 

noinst_HEADERS=global.h

geldkarte_SOURCES=\
  main.c
geldkarte_LDADD=../../lib/client/libchipcard.la $(mingw_ldflags) @gwenhywfar_libs@
geldkarte_LDFLAGS=@STRIPALL@

if IS_WINDOWS
geldkarte_LDFLAGS+=-mconsole
endif
