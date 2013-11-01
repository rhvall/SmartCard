# pcsc.m4
# (c) 2010 Martin Preuss<martin@libchipcard.de>
# This function checks if PC/SC is wanted

AC_DEFUN([AQ_CHECK_PCSC],[
dnl PREREQUISITES:
dnl   - AQ_CHECK_OS must becalled before
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     pcsc_libraries: Path to the PC/SC libraries 
dnl     pcsc_lib: PC/SC libraries to link against
dnl     pcsc_includes: Path to the PC/SC includes
dnl     have_pcsc: "yes" if pc/sc is available
dnl   Defines:


if test "$OSYSTEM" = "windows" ; then
  pcsc_libraries=""
  pcsc_lib="-lwinscard"
  have_pcsc="yes"
elif test "$OSYSTEM" = "osx" ; then
  AC_MSG_CHECKING(for pcsc includes)
  pcsc_includes="-I/System/Library/Frameworks/PCSC.framework/Headers"
  AC_MSG_RESULT($pcsc_includes)
  AC_MSG_CHECKING(for pcsc libs)
  pcsc_libraries=""
  pcsc_lib="-framework PCSC"
  AC_MSG_RESULT($pcsc_libraries ${pcsc_lib})
  have_pcsc="yes"
else
  AC_MSG_CHECKING(if PC/SC should be used)
  AC_ARG_ENABLE(pcsc,
    [  --enable-pcsc             enable PC/SC driver (default=yes)],
    enable_pcsc="$enableval",
    enable_pcsc="yes")
  AC_MSG_RESULT($enable_pcsc)

  if test "$enable_pcsc" != "no"; then

    dnl ******* pcsc includes ***********
    AC_MSG_CHECKING(for pcsc includes)
    AC_ARG_WITH(pcsc-includes, [  --with-pcsc-includes=DIR adds pcsc include path],
      [pcsc_search_inc_dirs="$withval"],
      [pcsc_search_inc_dirs="/usr/include\
                     /usr/local/include\
                     /usr/local/pcsc/include\
                     /usr/pcsc/include\
                     "])

    dnl search for pcsc
    AQ_SEARCH_FOR_PATH([PCSC/winscard.h],[$pcsc_search_inc_dirs])
    if test -n "$found_dir" ; then
      pcsc_includes="-I$found_dir -I$found_dir/PCSC"
    fi
    AC_MSG_RESULT($pcsc_includes)


    dnl ******* pcsc lib ***********
    AC_MSG_CHECKING(for pcsc libs)
    AC_ARG_WITH(pcsc-libname, [  --with-pcsc-libname=NAME  specify the name of the pcsc library],
      [pcsc_search_lib_names="$withval"],
      [pcsc_search_lib_names="libpcsclite.so \
                             libpcsclite.so.* \
                             libpcsc.a"])

    AC_ARG_WITH(pcsc-libs, [  --with-pcsc-libs=DIR  adds pcsc library path],
      [pcsc_search_lib_dirs="$withval"],
      [pcsc_search_lib_dirs="/usr/lib \
            	          /usr/local/lib \
                          /usr/lib/pcsc/lib \
	                  /usr/local/pcsc/lib \
	                  /lib"])
    dnl search for pcsc libs
    for d in $pcsc_search_lib_dirs; do
       AQ_SEARCH_FILES("$d",$pcsc_search_lib_names)
       if test -n "$found_file" ; then
          pcsc_libraries="-L$d"
          pcsc_lib="-l`echo $found_file | sed 's/lib//;s/\.so*//;s/\.a//'`"
          break
       fi
    done

    if test -z "$pcsc_libraries" -o -z "$pcsc_lib" -o -z "$pcsc_includes"; then
        AC_MSG_WARN([No pcsc libraries found, SCard driver will not be available.])
        have_pcsc="no"
    else
        AC_MSG_RESULT($pcsc_libraries ${pcsc_lib})
        have_pcsc="yes"
    fi
  # end of "if enable-pcsc"
  fi
# end of "if linux"
fi

AC_SUBST(pcsc_includes)
AC_SUBST(pcsc_libraries)
AC_SUBST(pcsc_lib)
])

