bin_PROGRAMS=memcard

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) 

noinst_HEADERS=global.h

memcard_SOURCES=\
  main.c
memcard_LDADD=../../lib/client/libchipcard.la $(mingw_ldflags) @gwenhywfar_libs@
memcard_LDFLAGS=@STRIPALL@

if IS_WINDOWS
memcard_LDFLAGS+=-mconsole
endif

