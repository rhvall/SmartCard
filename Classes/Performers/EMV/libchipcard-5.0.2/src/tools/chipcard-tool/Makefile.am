bin_PROGRAMS=chipcard-tool

INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) 

noinst_HEADERS=global.h

chipcard_tool_SOURCES=\
  main.c \
  getatr.c
chipcard_tool_LDADD=../../lib/client/libchipcard.la $(mingw_ldflags) @gwenhywfar_libs@
chipcard_tool_LDFLAGS=@STRIPALL@

if IS_WINDOWS
chipcard_tool_LDFLAGS+=-mconsole
endif

MAINTAINERCLEANFILES=Makefile.in

