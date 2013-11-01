# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for opensc

AC_DEFUN([AC_OPENSC], [
dnl searches for opensc
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: opensc_dir
dnl          opensc_includes
dnl          opensc_libs
dnl          have_opensc

v1="$1"
v2="$2"
v3="$3"
if test -z "$v1"; then wantedversion="0."; else wantedversion="${v1}."; fi
if test -n "$v2"; then wantedversion="${wantedversion}${v2}."; fi
if test -n "$v3"; then wantedversion="${wantedversion}${v3}"; fi

AC_MSG_CHECKING(if opensc support desired)
AC_ARG_ENABLE(opensc,
  [  --enable-opensc      enable opensc support (default=yes)],
  enable_opensc="$enableval",
  enable_opensc="yes")
AC_MSG_RESULT($enable_opensc)

have_opensc="no"
opensc_dir=""
opensc_includes=""
opensc_libs=""
if test "$enable_opensc" != "no"; then
  AC_MSG_CHECKING(for opensc)
  AC_ARG_WITH(opensc-dir, [  --with-opensc-dir=DIR
                            uses opensc from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
	     /opensc \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/opensc-config"; then
          opensc_dir="$li";
          break
      fi
  done
  if test -z "$opensc_dir"; then
      AC_MSG_RESULT([not found ])
  else
      AC_MSG_RESULT($opensc_dir)
      AC_MSG_CHECKING(for opensc includes)
      
      opensc_includes="`$opensc_dir/bin/opensc-config --cflags`"
      #oix="`$opensc_dir/bin/opensc-config --cflags`"
      #case $oix in
      #  */opensc)
      #    oix="${oix}X"
      #    opensc_includes="`echo $oix | sed -e s:/openscX::`"
      #    ;;
      #  *)
      #    opensc_includes="$oix"
      #    ;;
      #esac
      AC_MSG_RESULT($opensc_includes)
      AC_MSG_CHECKING(for opensc libs)
      opensc_libs="`$opensc_dir/bin/opensc-config --libs`"
      AC_MSG_RESULT($opensc_libs)
      AC_MSG_CHECKING(for opensc version)
      opensc_version="`$opensc_dir/bin/opensc-config --version`"
      AC_MSG_RESULT($opensc_version)

      AC_MSG_CHECKING(if opensc test desired)
      AC_ARG_ENABLE(opensc-test,
        [  --enable-opensc-test   enable opensc-test (default=yes)],
         enable_opensc_test="$enableval",
         enable_opensc_test="yes")
      AC_MSG_RESULT($enable_opensc_test)
      AC_MSG_CHECKING(for OpenSC version >=$wantedversion)
      if test "$enable_opensc_test" != "no"; then
        opensc_versionstring="`$opensc_dir/bin/opensc-config --version`"
        AC_MSG_RESULT([found $opensc_versionstring])
        if test "$wantedversion" \> "${opensc_versionstring}"; then
          AC_MSG_ERROR([Your OpenSC version is too old (found ${opensc_versionstring}, need $wantedversion).
          Please update from http://www.opensc.org/])
        fi
        have_opensc="yes"
      else
        have_opensc="yes"
        AC_MSG_RESULT(assuming yes)
      fi
  fi
fi

AC_SUBST(opensc_dir)
AC_SUBST(opensc_includes)
AC_SUBST(opensc_libs)
AC_SUBST(opensc_version)
])
