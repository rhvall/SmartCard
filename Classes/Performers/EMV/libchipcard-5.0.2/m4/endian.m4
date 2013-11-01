# $Id: endian.m4 595 2004-11-19 16:17:17Z aquamaniac $
# (c) 2006 Martin Preuss<martin@libchipcard.de>
# This function check for endianness

AC_DEFUN([AQ_ENDIAN],[
dnl PREREQUISITES:
dnl   none
dnl IN: 
dnl   - default type for crosscompiling
dnl OUT:
dnl   Variables:
dnl     aq_endian
dnl   Defines:

# check for type to use
AC_MSG_CHECKING(for endianness)
rm -f conf.endian
AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>

int main (){
  const char *s;
  FILE *f;
  union {
    unsigned int i;
    unsigned char c[4];
  } u;
  
  u.i=0x12345678;
  if (u.c[0]==0x78)
   s="little";
  else
   s="big";
  
  f=fopen("conf.endian", "w+");
  if (!f) {
    printf("Could not create file conf.endian\n");
    exit(1);
  }
  fprintf(f, "%s\n", s);
  if (fclose(f)) {
   printf("Could not close file.\n");
   exit(1);
  }
  exit(0);
}
 ],
 [aq_endian=`cat conf.endian`; AC_MSG_RESULT($aq_endian)],
 [AC_MSG_ERROR(Could not determine endianness)],
 [aq_endian="$1"; AC_MSG_RESULT([Crosscompiling, assuming $1])]
)
rm -f conf.endian
])

