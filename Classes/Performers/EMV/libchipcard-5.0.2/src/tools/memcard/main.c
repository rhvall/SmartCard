/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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

#include "global.h"
#include <gwenhywfar/args.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/cgui.h>

#define I18N(msg) msg


#define PROGRAM_VERSION "2.9"

#define k_PRG_VERSION_INFO \
    "memcard v2.9  (part of libchipcard v"k_CHIPCARD_VERSION_STRING")\n"\
    "(c) 2006 Martin Preuss<martin@libchipcard.de>\n" \
    "This program is free software licensed under GPL.\n"\
    "See COPYING for details.\n"


const GWEN_ARGS prg_args[]={
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "logtype",                    /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  0,                            /* short option */
  "logtype",                    /* long option */
  "Set the logtype",            /* short description */
  "Set the logtype (console, file)."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "loglevel",                   /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  0,                            /* short option */
  "loglevel",                   /* long option */
  "Set the log level",          /* short description */
  "Set the log level (info, notice, warning, error)."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "logfile",                    /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  0,                            /* short option */
  "logfile",                   /* long option */
  "Set the log file",          /* short description */
  "Set the log file (if log type is \"file\")."
},
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
  0,                            /* flags */
  GWEN_ArgsType_Int,             /* type */
  "verify",                     /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  0,                            /* short option */
  "verify",                     /* long option */
  "Verify written data",     /* short description */
  "Verify data after writing it to the card."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Int,             /* type */
  "offset",                     /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  "a",                          /* short option */
  "offset",                     /* long option */
  "offset to start reading/writing at",      /* short description */
  "Offset to start reading/writing at."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Int,             /* type */
  "size",                       /* name */
  1,                            /* minnum */
  1,                            /* maxnum */
  "s",                          /* short option */
  "size",                       /* long option */
  "number of bytes to read/write",  /* short description */
  "Number of bytes to read/write.", /* long description */
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "filename",                   /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  "f",                          /* short option */
  "filename",                   /* long option */
  "File to read/write",         /* short description */
  "File to read/write."
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "pin",                        /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  "p",                          /* short option */
  "pin",                        /* long option */
  "Pin",                        /* short description */
  "Pin."
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


void usage(const char *ustr) {
  fprintf(stdout,"%s%s",
          I18N("MemCard2 - A tool to read/write data from/to a memory chip card\n"
               "(c) 2006 Martin Preuss<martin@libchipcard.de>\n"
               "This library is free software; you can redistribute it and/or\n"
               "modify it under the terms of the GNU Lesser General Public\n"
               "License as published by the Free Software Foundation; either\n"
               "version 2.1 of the License, or (at your option) any later version.\n"
               "\n"),
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
  if (card && res==LC_Client_ResultCmdError) {
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



int memRead(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  int rv;
  int v;
  const char *s;
  int i;
  int offset;
  int size;
  const char *fname;
  FILE *f;
  GWEN_BUFFER *buf;
  int bps;
  time_t t0;
  time_t t1;
  int dt;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  if (v>1)
    fprintf(stderr, "Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "StartWait");
    return RETURNVALUE_WORK;
  }
  if (v>1)
    fprintf(stderr, "Connected.\n");

  for (i=0;;i++) {
    if (v>0)
      fprintf(stderr, "Waiting for card...\n");
    res=LC_Client_GetNextCard(cl, &card, 20);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "GetNextCard");
      return RETURNVALUE_WORK;
    }
    if (v>0)
      fprintf(stderr, "Found a card.\n");

    s=LC_Card_GetCardType(card);
    assert(s);
    if (strcasecmp(s, "memory")==0)
      break;

    if (v>0)
      fprintf(stderr, "Not a memory card, releasing.\n");
    res=LC_Client_ReleaseCard(cl, card);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "ReleaseCard");
      return RETURNVALUE_WORK;
    }
    LC_Card_free(card);

    if (i>15) {
      fprintf(stderr, "ERROR: No card found.\n");
      return RETURNVALUE_WORK;
    }
  } /* for */

  /* extend card */
  rv=LC_MemoryCard_ExtendCard(card);
  if (rv) {
    fprintf(stderr, "Could not extend card as memory card\n");
    return RETURNVALUE_WORK;
  }

  /* open card */
  if (v>0)
    fprintf(stderr, "Opening card.\n");
  res=LC_Card_Open(card);
  if (res!=LC_Client_ResultOk) {
    fprintf(stderr,
            "ERROR: Error executing command CardOpen (%d).\n",
            res);
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Card is a memory card as expected.\n");

  /* stop waiting */
  if (v>1)
    fprintf(stderr, "Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    LC_Client_ReleaseCard(cl, card);
    return RETURNVALUE_WORK;
  }

  /* open file */
  fname=GWEN_DB_GetCharValue(dbArgs, "fileName", 0, 0);
  if (fname==0) {
    f=stdout;
  }
  else {
    f=fopen(fname,"w+");
  }
  if (!f) {
    fprintf(stderr,
            I18N("ERROR: Could not open file (%s).\n"),
            strerror(errno));
    LC_Card_Close(card);
    LC_Client_ReleaseCard(cl, card);

    return RETURNVALUE_WORK;
  }

  /* read data, write to file */
  offset=GWEN_DB_GetIntValue(dbArgs, "offset", 0, 0);
  size=GWEN_DB_GetIntValue(dbArgs, "size", 0, 0);

  if (v>0)
    fprintf(stderr, "Starting to read %d bytes at offset 0x%04x.\n",
            size, offset);
  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  bps=0;
  t0=time(0);
  i=size;
  while(i) {
    int t;

    if (i>1024)
      t=1024;
    else
      t=i;

    if (v>1) {
      if (i>t)
        fprintf(stderr,
                "  Reading %4d bytes at 0x%04x (%d bytes left)\n",
                t, offset, i);
      else
        fprintf(stderr,
                "  Reading %4d bytes at 0x%04x (last chunk)\n",
                t, offset);
    }
    /* read bytes */
    res=LC_MemoryCard_ReadBinary(card,
                                 offset,
                                 t,
                                 buf);

    if (res!=LC_Client_ResultOk) {
      showError(card, res, "ReadBinary");
      if (fname)
        fclose(f);
      LC_Card_Close(card);
      LC_Client_ReleaseCard(cl, card);
      GWEN_Buffer_free(buf);
      return RETURNVALUE_WORK;
    }

    /* write bytes */
    if (v>2)
      fprintf(stderr, "  Writing data to file\n");
    if (fwrite(GWEN_Buffer_GetStart(buf),
               1,
               GWEN_Buffer_GetUsedBytes(buf),
               f)!=GWEN_Buffer_GetUsedBytes(buf)) {
      fprintf(stderr,
              I18N("ERROR: Could not write to file (%s).\n"),
              strerror(errno));
      if (fname)
        fclose(f);
      LC_Card_Close(card);
      LC_Client_ReleaseCard(cl, card);
      GWEN_Buffer_free(buf);
      return RETURNVALUE_WORK;
    }
    GWEN_Buffer_Reset(buf);
    i-=t;
    offset+=t;
  } /* while */
  t1=time(0);
  dt=(int)difftime(t1, t0);
  if (dt==0)
    dt=1;
  bps=size/dt;

  GWEN_Buffer_free(buf);

  if (v>0)
    fprintf(stderr,
            "Reading done [%d bytes/s].\n",
            bps);

  /* close file */
  if (fname) {
    if (fclose(f)) {
      fprintf(stderr,
              I18N("ERROR: Could not close file (%s).\n"),
              strerror(errno));
      LC_Card_Close(card);
      return RETURNVALUE_WORK;
    }
  }

  /* close card */
  if (v>0)
    fprintf(stderr, "Closing card.\n");
  res=LC_Card_Close(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardClose");
    return RETURNVALUE_WORK;
  }
  else
    if (v>1)
      fprintf(stderr, "Card closed.\n");

  if (v>0)
    fprintf(stderr, "Releasing card.\n");
  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReleaseCard");
    return RETURNVALUE_WORK;
  }
  LC_Card_free(card);

  /* finished */
  if (v>1)
    fprintf(stderr, "Finished.\n");
  return 0;
}



int memWrite(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  int rv;
  int v;
  const char *s;
  int i;
  int offset;
  int size;
  const char *fname;
  FILE *f;
  int bps;
  int dt;
  time_t t0;
  time_t t1;
  int wantVerify;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  wantVerify=GWEN_DB_GetIntValue(dbArgs, "verify", 0, 0);

  /* open file */
  fname=GWEN_DB_GetCharValue(dbArgs, "fileName", 0, 0);
  if (fname==0) {
    f=stdin;
  }
  else {
    f=fopen(fname,"r");
  }
  if (!f) {
    fprintf(stderr,
            I18N("ERROR: Could not open file (%s).\n"),
            strerror(errno));
    return RETURNVALUE_WORK;
  }

  if (v>1)
    fprintf(stderr, "Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "StartWait");
    if (fname)
      fclose(f);
    return RETURNVALUE_WORK;
  }
  if (v>1)
    fprintf(stderr, "Connected.\n");

  for (i=0;;i++) {
    if (v>0)
      fprintf(stderr, "Waiting for card...\n");
    res=LC_Client_GetNextCard(cl, &card, 20);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "GetNextCard");
      if (fname)
        fclose(f);
      return RETURNVALUE_WORK;
    }
    if (v>0)
      fprintf(stderr, "Found a card.\n");

    s=LC_Card_GetCardType(card);
    assert(s);
    if (strcasecmp(s, "memory")==0)
      break;

    if (v>0)
      fprintf(stderr, "Not a memory card, releasing.\n");
    res=LC_Client_ReleaseCard(cl, card);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "ReleaseCard");
      if (fname)
        fclose(f);
      return RETURNVALUE_WORK;
    }
    LC_Card_free(card);

    if (i>15) {
      fprintf(stderr, "ERROR: No card found.\n");
      if (fname)
        fclose(f);
      return RETURNVALUE_WORK;
    }
  } /* for */

  /* extend card */
  rv=LC_MemoryCard_ExtendCard(card);
  if (rv) {
    fprintf(stderr, "Could not extend card as memory card\n");
    if (fname)
      fclose(f);
    LC_Client_ReleaseCard(cl, card);
    return RETURNVALUE_WORK;
  }

  /* open card */
  if (v>0)
    fprintf(stderr, "Opening card.\n");
  res=LC_Card_Open(card);
  if (res!=LC_Client_ResultOk) {
    fprintf(stderr,
            "ERROR: Error executing command CardOpen (%d).\n",
            res);
    if (fname)
      fclose(f);
    LC_Client_ReleaseCard(cl, card);
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Card is a memory card as expected.\n");

  /* stop waiting */
  if (v>1)
    fprintf(stderr, "Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    if (fname)
      fclose(f);
    LC_Client_ReleaseCard(cl, card);
    return RETURNVALUE_WORK;
  }

  /* read file, write to card */
  offset=GWEN_DB_GetIntValue(dbArgs, "offset", 0, 0);
  size=GWEN_DB_GetIntValue(dbArgs, "size", 0, 0);

  if (v>0)
    fprintf(stderr, "Starting to write %d bytes at offset 0x%04x.\n",
            size, offset);
  bps=0;
  t0=time(0);
  i=size;
  while(i) {
    int t;
    char buf[1024];

    if (i>sizeof(buf))
      t=sizeof(buf);
    else
      t=i;

    /* read bytes */
    if (v>2)
      fprintf(stderr, "  Reading data from file\n");
    t=fread(buf, 1, t, f);
    if (t<1) {
      fprintf(stderr,
              I18N("ERROR: Could not read from file (%s).\n"),
              strerror(errno));
      if (fname)
        fclose(f);
      LC_Card_Close(card);
      LC_Client_ReleaseCard(cl, card);
      return RETURNVALUE_WORK;
    }

    if (v>1) {
      if (i>t)
        fprintf(stderr,
                "  Writing %4d bytes at 0x%04x (%d bytes left)\n",
                t, offset, i);
      else
        fprintf(stderr,
                "  Writing %4d bytes at 0x%04x (last chunk)\n",
                t, offset);
    }

    /* write bytes */
    res=LC_Client_ResultOk;
    res=LC_MemoryCard_WriteBinary(card,
                                  offset,
                                  buf,
                                  t);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "WriteBinary");
      if (fname)
        fclose(f);
      LC_Card_Close(card);
      LC_Client_ReleaseCard(cl, card);
      return RETURNVALUE_WORK;
    }

    if (wantVerify) {
      GWEN_BUFFER *tbuf;

      if (v>1)
        fprintf(stderr, "  Verifying data\n");
      tbuf=GWEN_Buffer_new(0, t, 0, 1);
      if (v>2)
        fprintf(stderr, "    Reading data for verification\n");
      res=LC_MemoryCard_ReadBinary(card,
                                   offset,
                                   t,
                                   tbuf);

      if (res!=LC_Client_ResultOk) {
        showError(card, res, "ReadBinary");
        if (fname)
          fclose(f);
        LC_Card_Close(card);
        LC_Client_ReleaseCard(cl, card);
        GWEN_Buffer_free(tbuf);
        return RETURNVALUE_WORK;
      }

      if (GWEN_Buffer_GetUsedBytes(tbuf)!=t ||
          memcmp(buf, GWEN_Buffer_GetStart(tbuf), t)!=0) {
        fprintf(stderr, "ERROR: Verification failed at offset 0x%04x\n",
                offset);
        if (fname)
          fclose(f);
        LC_Card_Close(card);
        LC_Client_ReleaseCard(cl, card);
        GWEN_Buffer_free(tbuf);
        return RETURNVALUE_WORK;
      }
      GWEN_Buffer_free(tbuf);
      if (v>2)
        fprintf(stderr, "    Verification ok\n");
    } /* if wantVerify */

    i-=t;
    offset+=t;
  } /* while */
  t1=time(0);
  dt=(int)difftime(t1, t0);
  if (dt==0)
    dt=1;
  bps=size/dt;

  if (v>0)
    fprintf(stderr,
            "Writing done [%d bytes/s].\n",
            bps);

  /* close file */
  if (fname) {
    if (fclose(f)) {
      fprintf(stderr,
              I18N("ERROR: Could not close file (%s).\n"),
              strerror(errno));
      LC_Card_Close(card);
      LC_Client_ReleaseCard(cl, card);
      return RETURNVALUE_WORK;
    }
  }

  /* close card */
  if (v>0)
    fprintf(stderr, "Closing card.\n");
  res=LC_Card_Close(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardClose");
    return RETURNVALUE_WORK;
  }
  else
    if (v>1)
      fprintf(stderr, "Card closed.\n");

  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReleaseCard");
    return RETURNVALUE_WORK;
  }
  else
    if (v>1)
      fprintf(stderr, "Card released.\n");

  /* finished */
  if (v>1)
    fprintf(stderr, "Finished.\n");
  return 0;
}



int main(int argc, char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  const char *s;
  LC_CLIENT *cl;
  GWEN_LOGGER_LOGTYPE logType;
  GWEN_LOGGER_LEVEL logLevel;
  LC_CLIENT_RESULT res;
  GWEN_GUI *gui;

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
    usage(GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }
  if (rv<1) {
    fprintf(stderr, "ERROR: Error in argument list (%d)\n", rv);
    return RETURNVALUE_PARAM;
  }

  /* setup logging */
  s=GWEN_DB_GetCharValue(db, "loglevel", 0, "warning");
  logLevel=GWEN_Logger_Name2Level(s);
  if (logLevel==GWEN_LoggerLevel_Unknown) {
    fprintf(stderr, "ERROR: Unknown log level (%s)\n", s);
    return RETURNVALUE_PARAM;
  }
  s=GWEN_DB_GetCharValue(db, "logtype", 0, "console");
  logType=GWEN_Logger_Name2Logtype(s);
  if (logType==GWEN_LoggerType_Unknown) {
    fprintf(stderr, "ERROR: Unknown log type (%s)\n", s);
    return RETURNVALUE_PARAM;
  }
  rv=GWEN_Logger_Open(LC_LOGDOMAIN,
                      "memcard3",
		      GWEN_DB_GetCharValue(db, "logfile", 0, "memcard3.log"),
		      logType,
		      GWEN_LoggerFacility_User);
  if (rv) {
    fprintf(stderr, "ERROR: Could not setup logging (%d).\n", rv);
    return RETURNVALUE_SETUP;
  }
  GWEN_Logger_SetLevel(LC_LOGDOMAIN, logLevel);

  /* get command */
  s=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!s) {
    fprintf(stderr, "No command given.\n");
    GWEN_DB_Group_free(db);
    return RETURNVALUE_PARAM;
  }

  cl=LC_Client_new("memcard", PROGRAM_VERSION);
  res=LC_Client_Init(cl);
  if (res!=LC_Client_ResultOk) {
    showError(0, res, "Init");
    return RETURNVALUE_SETUP;
  }

  /* handle command */
  if (strcasecmp(s, "read")==0) {
    rv=memRead(cl, db);
  }
  else if (strcasecmp(s, "write")==0) {
    rv=memWrite(cl, db);
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"", s);
    rv=RETURNVALUE_PARAM;
  }

  res=LC_Client_Fini(cl);
  if (res!=LC_Client_ResultOk) {
    showError(0, res, "Init");
  }

  LC_Client_free(cl);
  GWEN_DB_Group_free(db);
  return 0;
}








