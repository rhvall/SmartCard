SUBDIRS=base cards ct

EXTRA_DIST=ressource.rc.in

DEFS += \
  -DBUILDING_CHIPCARD

AM_CFLAGS=$(visibility_cflags)

INCLUDES = -I$(libchipcard_symlinkdir) $(gwenhywfar_includes)

lib_LTLIBRARIES=libchipcard.la
noinst_PROGRAMS=libtest


libchipcard_la_SOURCES=dummy.c
libchipcard_la_LIBADD= \
  base/libbase.la \
  cards/libcards.la \
  ct/liblcct.la \
  ../sharedstuff/libsharedstuff.la $(mingw_ldflags) @gwenhywfar_libs@ @zlib_libs@ $(pcsc_libraries) $(pcsc_lib)

libchipcard_la_LDFLAGS= \
  @STRIPALL@ -no-undefined -version-info \
  @LIBCHIPCARD_SO_CURRENT@:@LIBCHIPCARD_SO_REVISION@:@LIBCHIPCARD_SO_AGE@


libtest_SOURCES=libtest.c
libtest_LDADD=@gwenhywfar_libs@ libchipcard.la


