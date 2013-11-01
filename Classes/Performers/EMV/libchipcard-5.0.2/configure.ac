# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([chipcard-config.in.in])
AC_CONFIG_HEADER([config.h])



###-------------------------------------------------------------------------
#
# versions
#

CHIPCARD_VERSION_MAJOR=5
CHIPCARD_VERSION_MINOR=0
CHIPCARD_VERSION_PATCHLEVEL=2
CHIPCARD_VERSION_BUILD=0
CHIPCARD_VERSION_TAG="stable"

CHIPCARD_VERSION_FULL_STRING="$CHIPCARD_VERSION_MAJOR.$CHIPCARD_VERSION_MINOR.$CHIPCARD_VERSION_PATCHLEVEL.$CHIPCARD_VERSION_BUILD${CHIPCARD_VERSION_TAG}"
CHIPCARD_VERSION_STRING="$CHIPCARD_VERSION_MAJOR.$CHIPCARD_VERSION_MINOR.$CHIPCARD_VERSION_PATCHLEVEL"

#
# SO version
#
LIBCHIPCARD_SO_CURRENT=6
LIBCHIPCARD_SO_AGE=0
LIBCHIPCARD_SO_REVISION=2
LIBCHIPCARD_SO_EFFECTIVE="`echo \$(($LIBCHIPCARD_SO_CURRENT-$LIBCHIPCARD_SO_AGE))`"

#
# Create release string
#

case "$CHIPCARD_VERSION_TAG" in
  svn)
     rev=`LC_ALL="C" $SVN info|grep 'Last Changed Rev'|cut -d' ' -f4`
     if test "x$rev" != "x" ; then
       CHIPCARD_VERSION_RELEASE_STRING="$CHIPCARD_VERSION_MAJOR.$CHIPCARD_VERSION_MINOR.${CHIPCARD_VERSION_PATCHLEVEL}${CHIPCARD_VERSION_TAG}-r${rev}"
     else
       CHIPCARD_VERSION_RELEASE_STRING="$CHIPCARD_VERSION_FULL_STRING"
     fi
    ;;
  git|cvs)
     CHIPCARD_VERSION_RELEASE_STRING="$CHIPCARD_VERSION_MAJOR.$CHIPCARD_VERSION_MINOR.${CHIPCARD_VERSION_PATCHLEVEL}${CHIPCARD_VERSION_TAG}-b${CHIPCARD_VERSION_BUILD}"
    ;;
  stable)
    CHIPCARD_VERSION_RELEASE_STRING="$CHIPCARD_VERSION_STRING"
    ;;
  *)
    CHIPCARD_VERSION_RELEASE_STRING="$CHIPCARD_VERSION_MAJOR.$CHIPCARD_VERSION_MINOR.${CHIPCARD_VERSION_PATCHLEVEL}${CHIPCARD_VERSION_TAG}"
    ;;
esac



AC_SUBST(CHIPCARD_VERSION_MAJOR)
AC_SUBST(CHIPCARD_VERSION_MINOR)
AC_SUBST(CHIPCARD_VERSION_PATCHLEVEL)
AC_SUBST(CHIPCARD_VERSION_BUILD)
AC_SUBST(CHIPCARD_VERSION_TAG)
AC_SUBST(CHIPCARD_VERSION_FULL_STRING)
AC_SUBST(CHIPCARD_VERSION_STRING)
AC_SUBST(CHIPCARD_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_MAJOR,$CHIPCARD_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_MINOR,$CHIPCARD_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_PATCHLEVEL,$CHIPCARD_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_BUILD,$CHIPCARD_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_STRING,"$CHIPCARD_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_FULL_STRING,"$CHIPCARD_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_RELEASE_STRING,"$CHIPCARD_VERSION_RELEASE_STRING", 
                   [release string])
AC_DEFINE_UNQUOTED(CHIPCARD_VERSION_TAG,$CHIPCARD_VERSION_TAG, [tag])

AC_SUBST(LIBCHIPCARD_SO_CURRENT)
AC_SUBST(LIBCHIPCARD_SO_REVISION)
AC_SUBST(LIBCHIPCARD_SO_AGE)
AC_SUBST(LIBCHIPCARD_SO_EFFECTIVE)


AM_INIT_AUTOMAKE(libchipcard, $CHIPCARD_VERSION_RELEASE_STRING)



###-------------------------------------------------------------------------
# check for OS
#

AQ_CHECK_OS



###-------------------------------------------------------------------------
# prefix handling
#

AC_MSG_CHECKING(for installation prefix)
AC_PREFIX_DEFAULT(/usr)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AC_MSG_RESULT($prefix)
AC_SUBST(prefix)



###-------------------------------------------------------------------------
# /etc/init.d cannot be written by ordinary users, but Martin wants to have 
# this as default even for prefix != /usr (e.g. /usr/local), so we need an 
# option to override it
#
AC_MSG_CHECKING(for init script directory)
AC_ARG_WITH(init-script-dir, 
  [  --with-init-script-dir=DIR directory to use instead of /etc/init.d (esp. for non-root installs)],
  [lc_init_script_dir="$withval"],
  [lc_init_script_dir="/etc/init.d"])
AC_SUBST(lc_init_script_dir)



###-------------------------------------------------------------------------
#
# static/shared handling
#
AC_ENABLE_STATIC(yes)
AC_ENABLE_SHARED(yes)



###-------------------------------------------------------------------------
# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_RC
AC_PROG_LIBTOOL
AC_CHECK_PROG(SED,sed,sed)



###-------------------------------------------------------------------------
# Checks for libraries.



###-------------------------------------------------------------------------
# Checks for header files.

AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/wait.h])
AC_CHECK_HEADERS([sys/time.h])



###-------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.

AC_C_CONST



###-------------------------------------------------------------------------
# Checks for library functions.

AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(strcasecmp strncasecmp)
AC_CHECK_FUNCS(fork)
AC_CHECK_FUNCS(time)
AC_CHECK_FUNCS(getpid)
AC_CHECK_FUNCS(sigaction)
AC_CHECK_FUNCS(random)
AC_CHECK_FUNCS(wait)
AC_CHECK_FUNCS(chmod)
AC_CHECK_FUNCS(symlink)



# ---------------------------------------------------------------------------
# check for endianness
#

AC_C_BIGENDIAN([aq_endian="big"], [aq_endian="little"])
if test "x$aq_endian" == "xlittle"; then
 AC_DEFINE(LC_ENDIAN_LITTLE, 1, [if system is little endian])
elif test "x$aq_endian" == "xbig"; then
 AC_DEFINE(LC_ENDIAN_BIG, 1, [if system is big endian])
elif test "x$aq_endian" == "xuniversal"; then
 AC_DEFINE(LC_ENDIAN_BIG, 1, [if system is big endian])
else
  AC_MSG_ERROR([Unknown endian type $aq_endian])
fi



###-------------------------------------------------------------------------
# check for Distribution

AQ_DISTRIBUTION



###-------------------------------------------------------------------------
# check for Gwenhywfar
#

AC_GWENHYWFAR(4,0,0,0)

all_includes="$all_includes $gwenhywfar_includes"


###-------------------------------------------------------------------------
# OS dependent settings
#

libchipcard_includes="-I\${prefix}/include/libchipcard5"
libchipcardc_libs="-L\${libdir} -lchipcard"

libchipcard_headerdir_am="\$(includedir)/libchipcard5/chipcard"
libchipcard_headerdir="\${includedir}/libchipcard5/chipcard"
libchipcard_symlinkdir="\$(top_builddir)/libchipcard5"

case "$OS_TYPE" in
    posix)
        lc_sys_is_windows=0
        
        lc_client_config_dir="\${sysconfdir}/chipcard"
        lc_client_config_file="chipcardc.conf"
        lc_client_xml_dir="\${datadir}/chipcard"
        lc_os_driver_dir="posix"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_LIBCHIPCARD2_DLL,1, [Define if DLL is built])
        lc_sys_is_windows=1

        lc_client_config_dir="\${sysconfdir}/chipcard"
        lc_client_config_file="chipcardc.conf"
        lc_client_xml_dir="\${datadir}/chipcard"
        lc_os_driver_dir="win32"
    	;;
esac

AM_CONDITIONAL(IS_WINDOWS, [test "$OS_TYPE" = "windows"])
AC_SUBST(lc_sys_is_windows)

AC_SUBST(lc_client_config_dir)
AC_SUBST(lc_client_config_file)
AC_SUBST(lc_client_xml_dir)

AC_SUBST(libchipcardc_libs)
AC_SUBST(libchipcard_includes)

AC_SUBST(libchipcard_headerdir_am)
AC_SUBST(libchipcard_headerdir)

AC_SUBST(lc_os_driver_dir)

AC_SUBST(libchipcard_symlinkdir)

AC_SUBST(all_includes)
AC_SUBST(all_libraries)



###-------------------------------------------------------------------------
#
# check whether local installation mode is enabled
#

AC_MSG_CHECKING(whether local installation mode is wanted)
AC_ARG_ENABLE(local-install,
  [  --enable-local-install         allow local installation mode (default=no)],
  enable_local_install="$enableval",
  enable_local_install="no")
AC_MSG_RESULT($enable_local_install)

if test "$enable_local_install" != "no"; then
  AC_DEFINE(ENABLE_LOCAL_INSTALL, 1, [whether local install is wanted])
fi



###-------------------------------------------------------------------------
#
# check data search dir
#
AC_MSG_CHECKING(data search dir)
AC_ARG_WITH(data-searchdir, [  --with-data-searchdir=DIR where to search for data files],
  [lc_data_searchdir="$withval"],
  [lc_data_searchdir=""])

if test -z "${lc_data_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    lc_data_searchdir="share/chipcard"
  elif test "$OSYSTEM" = "osx"; then
    if test "$enable_local_install" != "no"; then
      lc_data_searchdir="Frameworks/AqBanking/share/chipcard"
    else
      lc_data_searchdir="\$(datadir)"
    fi
  else
    if test "$enable_local_install" != "no"; then
      lc_data_searchdir="share/chipcard"
    else
      lc_data_searchdir="$lc_client_xml_dir"
    fi
  fi
fi


AC_SUBST(lc_data_searchdir)
AC_MSG_RESULT($lc_data_searchdir)



###-------------------------------------------------------------------------
#
# check locale search dir
#
AC_MSG_CHECKING(locale search dir)
AC_ARG_WITH(locale-searchdir, [  --with-locale-searchdir=DIR where to search for locale files],
  [lc_locale_searchdir="$withval"],
  [lc_locale_searchdir=""])

if test -z "${lc_locale_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    lc_locale_searchdir="share/locale"
  elif test "$OSYSTEM" = "osx"; then
    if test "$enable_local_install" != "no"; then
      lc_locale_searchdir="Frameworks/AqBanking/share/locale"
    else
      lc_locale_searchdir="\$(localedir)"
    fi
  else
    if test "$enable_local_install" != "no"; then
      lc_locale_searchdir="share/locale"
    else
      lc_locale_searchdir="\$(localedir)"
    fi
  fi
fi

AC_SUBST(lc_locale_searchdir)
AC_MSG_RESULT($lc_locale_searchdir)



###-------------------------------------------------------------------------
#
# check cfg search dir
#
AC_MSG_CHECKING(cfg search dir)
AC_ARG_WITH(cfg-searchdir, [  --with-cfg-searchdir=DIR where to search for cfg files],
  [lc_cfg_searchdir="$withval"],
  [lc_cfg_searchdir=""])

if test -z "${lc_cfg_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    lc_cfg_searchdir="etc/chipcard"
  elif test "$OSYSTEM" = "osx"; then
    if test "$enable_local_install" != "no"; then
      lc_cfg_searchdir="Frameworks/AqBanking/etc/chipcard"
    else
      lc_cfg_searchdir="$lc_client_config_dir"
    fi
  else
    if test "$enable_local_install" != "no"; then
      lc_cfg_searchdir="etc/chipcard"
    else
      lc_cfg_searchdir="$lc_client_config_dir"
    fi
  fi
fi


AC_SUBST(lc_cfg_searchdir)
AC_MSG_RESULT($lc_cfg_searchdir)



# ---------------------------------------------------------------------------
# check for PC/SC
#


AQ_CHECK_PCSC
if test "$have_pcsc" != "yes"; then
      AC_MSG_ERROR([
***** No pcsc libraries found.
***** Please install the pcsclite package including development packages.])
fi

if test "$OSYSTEM" = "osx" ; then
  pcsc_includes="-I\$(libchipcard_symlinkdir)/PCSC"
fi



# ---------------------------------------------------------------------------
# check for zlib
# (actually we assume that it is present)
#
zlib_libs="-lz"
AC_SUBST(zlib_libs)



###-------------------------------------------------------------------------
# check for release
#
AC_MSG_CHECKING(whether this is an official release)
AC_ARG_ENABLE(release,
  [  --enable-release         make this an official release (default=no)],
  [ case "${enableval}" in
    yes) enable_release="yes";;
    no)  enable_release="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-release);;
    esac
  ],
  enable_release="no")

if test "$enable_release" = "yes"; then
  STRIPALL="-s"
else
  STRIPALL=""
fi
AC_SUBST(STRIPALL)
AC_MSG_RESULT($enable_release)


###-------------------------------------------------------------------------
#
# check docpath
#
AC_MSG_CHECKING(docpath)
AC_ARG_WITH(docpath, [  --with-docpath=DIR where to store the apidoc],
  [docpath="$withval"],
  [docpath="${HOME}/apidoc"])
AC_SUBST(docpath)
AC_MSG_RESULT($docpath)



###-------------------------------------------------------------------------
# check for doc type
#
AC_MSG_CHECKING(if full docu should be created)
AC_ARG_ENABLE(full-doc,
  [  --enable-full-doc         enable creating full apidoc (default=no)],
  [ case "${enableval}" in
    yes) enable_fulldoc="yes";;
    no)  enable_fulldoc="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-full-doc);;
    esac
  ],
  enable_fulldoc="no")

if test "$enable_fulldoc" = "yes"; then
	DOXYGEN_INPUT="src/lib"
        DOXYGEN_DEFINE=""
else
	DOXYGEN_INPUT="chipcard"
	DOXYGEN_DEFINE="DOXYGEN_HIDE"
fi
AC_SUBST(DOXYGEN_INPUT)
AC_SUBST(DOXYGEN_DEFINE)
AC_MSG_RESULT($enable_fulldoc)



###-------------------------------------------------------------------------
# GCC version (check for usability)
#

AC_MSG_CHECKING(if symbol visibility is supported)
visibility_cflags=""
case `basename $CC` in 
  gcc*)
     dnl Determine version number (watch out: the outside square
     dnl parenteses are vitally important here!)
     gccversion=["` ${CC} --version | head -1 | ${SED} 's/.*\([0-9]\.[0-9]\.[0-9]*\).*/\1/' `"]
     case $gccversion in
       4.*)
	 # Activate -fvisibility=hidden only for explicit 4.x gcc
	 # versions. We never now what future gcc's might do with
	 # that argument, and we also can't be sure that other
	 # gcc's happen to show very weird version
	 # numbers. Therefore leave the default at non-activated.
         visibility_supported="yes, gcc-${gccversion}"
         AC_DEFINE(GCC_WITH_VISIBILITY_ATTRIBUTE, 1, [visibility])
         visibility_cflags="-fvisibility=hidden"
         ;;
       *)
	 # Default: -fvisibility=hidden not activated.
         visibility_supported="no (needs gcc >=4.0, you have gcc-${gccversion})"
         ;;
      esac
      ;;
  *)
    visibility_supported="no (needs gcc >=4.0, you use $CC)"
    ;;
esac

# -fvisibility is at least not available on MinGW/gcc-3.4.4 (will
# give an "unrecognized cmdline option"). Also, unfortunately I
# don't know an easy way to ask the compiler here. Maybe
# http://autoconf-archive.cryp.to/ax_cflags_gcc_option.html
case "$OS_TYPE" in
  windows)
        visibility_supported="no (not yet on MinGW/Windows)"
        visibility_cflags=""
	;;
  *)
	;;
esac

AC_MSG_RESULT(${visibility_supported})
AC_SUBST(visibility_cflags)



###-------------------------------------------------------------------------
# search for tag files
#

AC_MSG_CHECKING(doxygen tag files)
DOXYGEN_TAGFILES=""
if test -d "${docpath}"; then
  DOXYGEN_TAGFILES="`cd ${docpath} && ls *.tag`"
  if test -n "${DOXYGEN_TAGFILES}"; then
   DOXYGEN_TAGFILES="`echo ${DOXYGEN_TAGFILES} | ${SED} -e s/${PACKAGE}.tag//`"
  fi
  realfiles=""
  for ff in ${DOXYGEN_TAGFILES}; do
    realfiles="${realfiles} ${docpath}/${ff}"
  done
  DOXYGEN_TAGFILES="${realfiles}"
fi
if test -z "${DOXYGEN_TAGFILES}"; then
  AC_MSG_RESULT(none)
else
  AC_MSG_RESULT(found)
fi
AC_SUBST(DOXYGEN_TAGFILES)



###-------------------------------------------------------------------------
# Search for gettext
#

AC_CHECK_HEADERS([libintl.h],[have_libintl_h=yes],[have_libintl_h=no])
LIBS=""
I18N_CATALOG=""
AC_CHECK_PROGS(XGETTEXT, xgettext, missing)
if test "$XGETTEXT" != "missing"; then
  AC_SEARCH_LIBS(gettext,intl c,[have_gettext_fn=yes], [have_gettext_fn=no])
  i18n_library=$LIBS
  LIBS=""
  AC_MSG_CHECKING(for working i18n environment)
  if test "$have_libintl_h" = "yes" -a "$have_gettext_fn" = "yes" -a "$XGETTEXT" != "missing"; then
    I18N_PATH="`eval echo $prefix/share`/locale"
    AC_DEFINE_UNQUOTED(I18N_PATH,"$I18N_PATH",[Path for locales])
    AC_DEFINE(HAVE_GETTEXT_ENVIRONMENT,1,[if gettext is available])
    I18N_CATALOG="catalog"
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
fi
AC_SUBST(I18N_CATALOG)



###-------------------------------------------------------------------------
# check for MEMTRACE
#

AC_MSG_CHECKING(if MEMTRACE is wanted)
AC_ARG_ENABLE(memtrace,
  [  --enable-memtrace             enable memory trace (default=no)],
  enable_memtrace="$enableval",
  enable_memtrace="no")
AC_MSG_RESULT($enable_memtrace)

if test "$enable_memtrace" != "no"; then
  AC_DEFINE(GWEN_MEMORY_DEBUG, 1, [if memory debugging is wanted])
fi




###-------------------------------------------------------------------------
# Create output files
#

AC_CONFIG_FILES([Makefile
                 chipcard-config.in
                 Doxyfile
                 libchipcard.spec
                 bin/Makefile
                 data/Makefile
                 data/drivers/Makefile
                 data/drivers/posix/Makefile
                 data/drivers/posix/generic_pcsc/Makefile
                 data/drivers/posix/cyberjack_pcsc/Makefile
                 data/drivers/win32/Makefile
                 data/drivers/win32/cyberjack_ctapi/Makefile
                 doc/Makefile
                 etc/Makefile
                 src/Makefile
                 src/PCSC/Makefile
                 src/lib/Makefile
                 src/lib/version.h
                 src/lib/chipcard.h
                 src/lib/sharedstuff/Makefile
                 src/lib/client/Makefile
                 src/lib/client/ressource.rc
                 src/lib/client/base/Makefile
                 src/lib/client/cards/Makefile
                 src/lib/client/cards/ddvcard/Makefile
                 src/lib/client/cards/egkcard/Makefile
                 src/lib/client/cards/geldkarte/Makefile
                 src/lib/client/cards/memorycard/Makefile
                 src/lib/client/cards/kvkcard/Makefile
                 src/lib/client/cards/processorcard/Makefile
                 src/lib/client/cards/starcos/Makefile
                 src/lib/client/cards/zkacard/Makefile
                 src/lib/client/ct/Makefile
                 src/ct/Makefile
                 src/ct/ddvcard/Makefile
                 src/ct/ddvcard/ddvcard.xml
                 src/ct/starcoscard/Makefile
                 src/ct/starcoscard/starcoscard.xml
                 src/tools/Makefile
                 src/tools/geldkarte/Makefile
                 src/tools/memcard/Makefile
                 src/tools/chipcard-tool/Makefile
                 src/tools/kvkcard/Makefile
                 src/tools/cardcommander/Makefile
                 tutorials/Makefile
                 m4/Makefile
                ])
AC_OUTPUT



###-------------------------------------------------------------------------
# create header directory for all, make symlinks
#

rm -Rf libchipcard5
mkdir libchipcard5
mkdir libchipcard5/chipcard

# These two headers here are generated and hence reside in builddir,
# not in srcdir!
lprefix=`pwd`
AQ_HEADER_FILES="${lprefix}/src/lib/version.h ${lprefix}/src/lib/chipcard.h"
ln -s "${lprefix}/src/lib/version.h" libchipcard5/chipcard/version.h
ln -s "${lprefix}/src/lib/chipcard.h" libchipcard5/chipcard/chipcard.h


###-------------------------------------------------------------------------
# create symlinks for PC/SC on MacOSX
#
if test "$OSYSTEM" = "osx" ; then
  ln -s /System/Library/Frameworks/PCSC.framework/Headers "libchipcard5/PCSC"
fi



###-------------------------------------------------------------------------
# create header directory for server, make symlinks
#

lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

# symlink all headers from src/lib/sharedstuff/*
mkdir libchipcard5/chipcard/sharedstuff
hfiles=`( cd "${srcdir}/src/lib/sharedstuff" && ls *.h 2>/dev/null )`
for f in ${hfiles}; do
  AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/lib/sharedstuff/${f}"
  case ${f} in
    *_p.h)
          ;;
    *_l.h)
          ;;
    *)   
          ln -s "${lprefix}/src/lib/sharedstuff/${f}" "libchipcard5/chipcard/sharedstuff/${f}"
          ;;
  esac
done




###-------------------------------------------------------------------------
# create header directory for client, make symlinks
#

lc_modules="cards ct"
lc_client_card_modules="ddvcard egkcard geldkarte kvkcard starcos zkacard memorycard processorcard"


lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

# symlink all headers from src/lib/client
hfiles=`( cd "${srcdir}/src/lib/client" && ls *.h 2>/dev/null )`
for f in ${hfiles}; do
  AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/lib/client/${f}"
  case ${f} in
    *_p.h | *version.h | *chipcard.h)
              ;;
    *)
              ln -s "${lprefix}/src/lib/client/${f}" "libchipcard5/chipcard/${f}"
              ;;
  esac
done


# symlink all headers from src/lib/client/base
hfiles=`( cd "${srcdir}/src/lib/client/base" && ls *.h 2>/dev/null )`
for f in ${hfiles}; do
  AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/lib/client/base/${f}"
  case ${f} in
    *_p.h | *_l.h)
      ;;
    *)   
      ln -s "${lprefix}/src/lib/client/base/${f}" "libchipcard5/chipcard/${f}"
      ;;
  esac
done


# symlink all headers from src/lib/client/*
for sm in $lc_modules; do
  mkdir libchipcard5/chipcard/${sm}
  hfiles=`( cd "${srcdir}/src/lib/client/${sm}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/lib/client/${sm}/${f}"
    case ${f} in
      *_p.h | *_l.h)
        ;;
      *)   
        ln -s "${lprefix}/src/lib/client/${sm}/${f}" "libchipcard5/chipcard/${sm}/${f}"
        ;;
    esac
  done
done


# symlink all headers from src/lib/client/cards
hfiles=`( cd "${srcdir}/src/lib/client/cards" && ls *.h 2>/dev/null )`
for f in ${hfiles}; do
  AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/lib/client/base/${f}"
  case ${f} in
    *_p.h | *_l.h)
      ;;
    *)   
      ln -s "${lprefix}/src/lib/client/cards/${f}" "libchipcard5/chipcard/cards/${f}"
      ;;
  esac
done

# symlink all headers from src/lib/client/cards/*
for sm in $lc_client_card_modules; do
  hfiles=`( cd "${srcdir}/src/lib/client/cards/${sm}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/lib/client/cards/${f}"
    case ${f} in
      *_p.h | *_l.h)
        ;;
      *)   
        ln -s "${lprefix}/src/lib/client/cards/${sm}/${f}" "libchipcard5/chipcard/cards/${f}"
        ;;
    esac
  done
done



###-------------------------------------------------------------------------
# Summary
#

echo
echo "Summary:"
echo "================================="
echo "Libchipcard version     : $CHIPCARD_VERSION_FULL_STRING"
echo "System                  : $OSYSTEM"
echo "Cfg Installation Folder : $lc_client_config_dir"
echo "Cfg Search Folder       : $lc_cfg_searchdir"
echo "Data Installation Folder: $lc_client_xml_dir"
echo "Data Search Folder      : $lc_data_searchdir"
echo "Local Installation Mode : $enable_local_install"
echo ""
echo "You can now build Libchipcard with 'make' (on *BSD use 'gmake')."


