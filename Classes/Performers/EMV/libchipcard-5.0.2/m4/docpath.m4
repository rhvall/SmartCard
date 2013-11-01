# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for the output path for the documentation

AC_DEFUN([AQ_DOCPATH],[
dnl PREREQUISITES:
dnl   none
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     docpath: Output path for the documentation (subst)
dnl   Defines:

# check for docpath
AC_MSG_CHECKING(for docpath)
AC_ARG_WITH(docpath, [  --with-docpath=DIR      where to store the apidoc],
  [docpath="$withval"],
  [docpath="apidoc"])
AC_SUBST(docpath)
AC_MSG_RESULT($docpath)
])
