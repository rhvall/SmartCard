INCLUDES=-I$(libchipcard_symlinkdir) $(gwenhywfar_includes) 

EXTRA_DIST=README examplecard.xml tutorials.h

CLIENT_LIB=../src/lib/client/libchipcard.la

noinst_PROGRAMS=\
 tutorial1a tutorial1b tutorial1c \
 tutorial2 \
 tutorial3

# if IS_WINDOWS
if IS_WINDOWS
 mingw_ldflags=-L/mingw/lib
endif


appdatadir=@lc_client_xml_dir@/apps
appdata_DATA=examplecard.xml

noinst_HEADERS=examplecard.h examplecard_p.h

tutorial1a_SOURCES=tutorial1a.c
tutorial1a_LDADD=-L../src/lib/client $(CLIENT_LIB) $(mingw_ldflags) @gwenhywfar_libs@

tutorial1b_SOURCES=tutorial1b.c
tutorial1b_LDADD=-L../src/lib/client $(CLIENT_LIB) $(mingw_ldflags) @gwenhywfar_libs@

tutorial1c_SOURCES=tutorial1c.c
tutorial1c_LDADD=-L../src/lib/client $(CLIENT_LIB) $(mingw_ldflags) @gwenhywfar_libs@

tutorial2_SOURCES=tutorial2.c
tutorial2_LDADD=-L../src/lib/client $(CLIENT_LIB) $(mingw_ldflags) @gwenhywfar_libs@

tutorial3_SOURCES=tutorial3.c examplecard.c
tutorial3_LDADD=-L../src/lib/client $(CLIENT_LIB) $(mingw_ldflags) @gwenhywfar_libs@


