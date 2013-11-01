# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for the debugmode


AC_DEFUN([AQ_DEBUGMODE],[
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     DEBUGMODE: number of the debug level (subst)
dnl   Defines:
dnl     DEBUGMODE: number of the debug level (subst)

dnl check for debugmode
AC_MSG_CHECKING(for debug mode)
AC_ARG_WITH(debug-mode,
  [  --with-debug-mode=MODE  debug mode],
  [DEBUGMODE="$withval"],
  [DEBUGMODE="0"])
AC_SUBST(DEBUGMODE)
AC_DEFINE_UNQUOTED(DEBUGMODE,$DEBUGMODE,[debug mode to be used])
AC_MSG_RESULT($DEBUGMODE)
])
