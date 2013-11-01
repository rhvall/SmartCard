/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#include <unistd.h>

#include "global.h"
#include <gwenhywfar/args.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>

#define I18N(msg) msg


#define PROGRAM_VERSION "2.9"

#define k_PRG_VERSION_INFO \
    "kvkcard v2.9  (part of libchipcard v"k_CHIPCARD_VERSION_STRING")\n"\
    "(c) 2006 Martin Preuss<martin@libchipcard.de>\n" \
    "This program is free software licensed under GPL.\n"\
    "See COPYING for details.\n"

#ifdef OS_WIN32
# include <windows.h>
#
# define usleep(x) Sleep((x/1000))
#endif


const GWEN_ARGS prg_args[]={
{
  0,                            /* flags */
  GWEN_ArgsType_Int,             /* type */
  "verbosity",                  /* name */
  0,                            /* minnum */
  10,                           /* maxnum */
  "v",                          /* short option */
  "verbous",                    /* long option */
  "Increase the verbosity",     /* short description */
  "Every occurrence of this option increases the verbosity."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "filename",                   /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  "f",                          /* short option */
  "filename",                   /* long option */
  "File to write to (stdout if omitted)",    /* short description */
  "File to write to. If omitted stdout will be used."
},
{
  0,                            /* flags */
  GWEN_ArgsType_Int,            /* type */
  "beep",                       /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  "b",                          /* short option */
  "beep",                       /* long option */
  "Beep after reading a card",  /* short description */
  "Beep after reading a card."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT,
  GWEN_ArgsType_Char,
  "infilename",
  0,
  1,
  "i",
  "infilename",
  "File to read from. Only needed for psvd and pspd.",
  "File to read from. Only needed for psvd and pspd."
},
{
  0,
  GWEN_ArgsType_Int,
  "dosmode",
  0,
  1,
  "d",
  "dosmode",
  "Store data in DOS mode",
  "Store data in DOS mode"
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT,
  GWEN_ArgsType_Char,
  "program",
  0,
  1,
  "p",
  "program",
  "Program to call on cards found",
  "Program to call on cards found."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT,
  GWEN_ArgsType_Char,
  "args",
  0,
  1,
  "a",
  "args",
  "Arguments for the program to be called",
  "Arguments for the program to be called"
},
{
  GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
  GWEN_ArgsType_Int,             /* type */
  "help",                       /* name */
  0,                            /* minnum */
  0,                            /* maxnum */
  "h",                          /* short option */
  "help",                       /* long option */
  "Show help",                  /* short description */
  "Shows this help."            /* long description */
  }
};


void usage(const char *name, const char *ustr) {
  fprintf(stdout,
          I18N("KVKCard3 - A tool to read information from a German medical card.\n"
               "(c) 2007 Martin Preuss<martin@libchipcard.de>\n"
               "This library is free software; you can redistribute it and/or\n"
               "modify it under the terms of the GNU Lesser General Public\n"
               "License as published by the Free Software Foundation; either\n"
               "version 2.1 of the License, or (at your option) any later version.\n"
               "\n"
               "Usage: %s COMMAND [OPTIONS]\n"
               "\n"
               "Available commands:\n"
               "  read   : read data from a German medical card\n"
               "\n"
               "Available options:\n"
               "%s\n"),
          name,
          ustr);
}



void showError(LC_CARD *card, LC_CLIENT_RESULT res, const char *x) {
  const char *s;

  switch(res) {
  case LC_Client_ResultOk:
    s="Ok.";
    break;
  case LC_Client_ResultWait:
    s="Timeout.";
    break;
  case LC_Client_ResultIpcError:
    s="IPC error.";
    break;
  case LC_Client_ResultCmdError:
    s="Command error.";
    break;
  case LC_Client_ResultDataError:
    s="Data error.";
    break;
  case LC_Client_ResultAborted:
    s="Aborted.";
    break;
  case LC_Client_ResultInvalid:
    s="Invalid argument to command.";
    break;
  case LC_Client_ResultInternal:
    s="Internal error.";
    break;
  case LC_Client_ResultGeneric:
    s="Generic error.";
    break;
  default:
    s="Unknown error.";
    break;
  }

  fprintf(stderr, "Error in \"%s\": %s\n", x, s);
  if (res==LC_Client_ResultCmdError && card) {
    int sw1;
    int sw2;

    sw1=LC_Card_GetLastSW1(card);
    sw2=LC_Card_GetLastSW2(card);
    fprintf(stderr, "  Last card command result:\n");
    if (sw1!=-1 && sw2!=-1)
      fprintf(stderr, "   SW1=%02x, SW2=%02x\n", sw1, sw2);
    s=LC_Card_GetLastResult(card);
    if (s)
      fprintf(stderr, "   Result: %s\n", s);
    s=LC_Card_GetLastText(card);
    if (s)
      fprintf(stderr, "   Text  : %s\n", s);
  }
}



int writeFile(FILE *f, const char *p, int len) {
  while(len>0) {
    ssize_t l;
    ssize_t s;

    l=1024;
    if (l>len)
      l=len;
    s=fwrite(p, 1, l, f);
    if (s==(ssize_t)-1 || s==0) {
      DBG_INFO(LC_LOGDOMAIN,
	       "fwrite: %s",
	       strerror(errno));
      return GWEN_ERROR_IO;
    }
    p+=s;
    len-=s;
  }

  return 0;
}



void errorBeep() {
  fprintf(stderr, "\007");
  usleep(250000);
  fprintf(stderr, "\007");
  usleep(250000);
  fprintf(stderr, "\007");
}



void okBeep() {
  fprintf(stderr, "\007");
}




int main(int argc, char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  const char *s;
  LC_CLIENT *cl;
  LC_CLIENT_RESULT res;
  GWEN_GUI *gui;
  int v;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
                     GWEN_ARGS_MODE_ALLOW_FREEPARAM,
                     prg_args,
                     db);
  if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Args_Usage(prg_args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "Could not generate usage string.\n");
      GWEN_Buffer_free(ubuf);
      return RETURNVALUE_PARAM;
    }
    usage(argv[0], GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }
  if (rv<1) {
    fprintf(stderr, "ERROR: Error in argument list (%d)\n", rv);
    return RETURNVALUE_PARAM;
  }

  v=GWEN_DB_GetIntValue(db, "verbosity", 0, 0);
  if (v<2)
    GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_NONINTERACTIVE);

  /* get command */
  s=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!s) {
    fprintf(stderr, "No command given.\n");
    GWEN_DB_Group_free(db);
    return RETURNVALUE_PARAM;
  }

  cl=LC_Client_new("kvkcard", PROGRAM_VERSION);
  res=LC_Client_Init(cl);
  if (res!=LC_Client_ResultOk) {
    showError(0, res, "Init");
    return RETURNVALUE_SETUP;
  }

  /* handle command */
  if (strcasecmp(s, "read")==0) {
    rv=kvkRead(cl, db);
  }
  else if (strcasecmp(s, "daemon")==0) {
    fprintf(stderr, "KVK daemon no longer supported.\n");
    return RETURNVALUE_SETUP;
  }
  else if (strcasecmp(s, "rdvd")==0) {
    rv=rdvd(cl, db);
  }
  else if (strcasecmp(s, "rdpd")==0) {
    rv=rdpd(cl, db);
  }
  else if (strcasecmp(s, "psvd")==0) {
    rv=psvd(cl, db);
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"", s);
    rv=RETURNVALUE_PARAM;
  }

  LC_Client_free(cl);
  GWEN_DB_Group_free(db);
  return 0;
}





#include "read.c"
#include "rdvd.c"
#include "rdpd.c"
#include "psvd.c"


