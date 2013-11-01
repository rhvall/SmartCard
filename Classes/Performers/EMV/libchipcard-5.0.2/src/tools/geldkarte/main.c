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
#undef BUILDING_LIBCHIPCARD2_DLL

#include "global.h"
#include <gwenhywfar/args.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>

#define I18N(msg) msg


#define PROGRAM_VERSION "1.9"


const GWEN_ARGS prg_args[]={
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "configfile",                 /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  "C",                          /* short option */
  "configfile",                 /* long option */
  "Configuration file to load", /* short description */
  "Configuration file to load." /* long description */
},
{
  GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
  GWEN_ArgsType_Char,            /* type */
  "file",                       /* name */
  0,                            /* minnum */
  1,                            /* maxnum */
  "f",                          /* short option */
  "file",                       /* long option */
  "File name",                  /* short description */
  "File name. \n"
  "This filename is used when reading or writing data such as public keys,\n"
  "bank information etc."
},
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
  if (res==LC_Client_ResultCmdError) {
    fprintf(stderr, "  Last card command result:\n");
    fprintf(stderr, "   SW1=%02x, SW2=%02x\n",
            LC_Card_GetLastSW1(card),
            LC_Card_GetLastSW2(card));
    s=LC_Card_GetLastResult(card);
    if (s)
      fprintf(stderr, "   Result: %s\n", s);
    s=LC_Card_GetLastText(card);
    if (s)
      fprintf(stderr, "   Text  : %s\n", s);
  }
}



int loaded(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  LC_GELDKARTE_VALUES *values;
  int rv;
  int v;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  if (v>1)
    fprintf(stderr, "Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Start");
    return RETURNVALUE_WORK;
  }
  if (v>1)
    fprintf(stderr, "Connected.\n");

  if (v>0)
    fprintf(stderr, "Waiting for card...\n");
  res=LC_Client_GetNextCard(cl, &card, 20);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "GetNextCard");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Found a card.\n");

  if (LC_GeldKarte_ExtendCard(card)) {
    fprintf(stderr, "ERROR: Could not extend card as GELDKARTE card\n");
    return RETURNVALUE_WORK;
  }

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
    fprintf(stderr, "Card is a GELDKARTE card as expected.\n");

  if (v>1)
    fprintf(stderr, "Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    return RETURNVALUE_WORK;
  }

  values=LC_GeldKarte_Values_new();
  res=LC_GeldKarte_ReadValues(card, values);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReadValues");
    rv=RETURNVALUE_WORK;
  }
  else
    rv=0;

  if (v>0)
    fprintf(stderr, "Closing card.\n");
  res=LC_Card_Close(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardClose");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Card closed.\n");

  if (v>0)
    fprintf(stderr, "Releasing card.\n");
  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReleaseCard");
    rv=RETURNVALUE_WORK;
  }
  else
    rv=0;
  LC_Card_free(card);

  if (v>0)
    fprintf(stderr, "Card released.\n");

  if (rv==0) {
    fprintf(stdout,
            I18N("Card is loaded with %6.2f %s\n"),
            LC_GeldKarte_Values_GetLoaded(values)/100.0,
            "EUR");
  }
  return rv;
}



int maxload(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  LC_GELDKARTE_VALUES *values;
  int rv;
  int v;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  if (v>1)
    fprintf(stderr, "Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Start");
    return RETURNVALUE_WORK;
  }
  if (v>1)
    fprintf(stderr, "Connected.\n");

  if (v>0)
    fprintf(stderr, "Waiting for card...\n");
  res=LC_Client_GetNextCard(cl, &card, 20);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "GetNextCard");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Found a card.\n");

  if (LC_GeldKarte_ExtendCard(card)) {
    fprintf(stderr, "ERROR: Could not extend card as GELDKARTE card\n");
    return RETURNVALUE_WORK;
  }

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
    fprintf(stderr, "Card is a GELDKARTE card as expected.\n");

  if (v>1)
    fprintf(stderr, "Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    return RETURNVALUE_WORK;
  }

  values=LC_GeldKarte_Values_new();
  res=LC_GeldKarte_ReadValues(card, values);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReadValues");
    rv=RETURNVALUE_WORK;
  }
  else
    rv=0;

  if (v>0)
    fprintf(stderr, "Closing card.\n");
  res=LC_Card_Close(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardClose");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Card closed.\n");

  if (v>0)
    fprintf(stderr, "Releasing card.\n");
  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReleaseCard");
    rv=RETURNVALUE_WORK;
  }
  else
    rv=0;
  LC_Card_free(card);

  if (rv==0) {
    fprintf(stdout,
            I18N("Card can be loaded with %6.2f %s\n"),
            LC_GeldKarte_Values_GetMaxLoad(values)/100.0,
            "EUR");
  }
  return rv;
}



int maxxfer(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  LC_GELDKARTE_VALUES *values;
  int rv;
  int v;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  if (v>1)
    fprintf(stderr, "Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Start");
    return RETURNVALUE_WORK;
  }
  if (v>1)
    fprintf(stderr, "Connected.\n");

  if (v>0)
    fprintf(stderr, "Waiting for card...\n");
  res=LC_Client_GetNextCard(cl, &card, 20);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "GetNextCard");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Found a card.\n");

  if (LC_GeldKarte_ExtendCard(card)) {
    fprintf(stderr, "ERROR: Could not extend card as GELDKARTE card\n");
    return RETURNVALUE_WORK;
  }

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
    fprintf(stderr, "Card is a GELDKARTE card as expected.\n");

  if (v>1)
    fprintf(stderr, "Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    return RETURNVALUE_WORK;
  }

  values=LC_GeldKarte_Values_new();
  res=LC_GeldKarte_ReadValues(card, values);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReadValues");
    rv=RETURNVALUE_WORK;
  }
  else
    rv=0;

  if (v>0)
    fprintf(stderr, "Closing card.\n");
  res=LC_Card_Close(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardClose");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Card closed.\n");

  if (v>0)
    fprintf(stderr, "Releasing card.\n");
  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReleaseCard");
    rv=RETURNVALUE_WORK;
  }
  else
    rv=0;
  LC_Card_free(card);

  if (rv==0) {
    fprintf(stdout,
            I18N("Card can transfer up to %6.2f %s\n"),
            LC_GeldKarte_Values_GetMaxXfer(values)/100.0,
            "EUR");
  }
  return rv;
}



int b_logs(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  LC_GELDKARTE_BLOG_LIST2 *bll;
  int rv;
  int v;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  if (v>1)
    fprintf(stderr, "Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Start");
    return RETURNVALUE_WORK;
  }
  if (v>1)
    fprintf(stderr, "Connected.\n");

  if (v>0)
    fprintf(stderr, "Waiting for card...\n");
  res=LC_Client_GetNextCard(cl, &card, 20);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "GetNextCard");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Found a card.\n");

  if (LC_GeldKarte_ExtendCard(card)) {
    fprintf(stderr, "ERROR: Could not extend card as GELDKARTE card\n");
    return RETURNVALUE_WORK;
  }

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
    fprintf(stderr, "Card is a GELDKARTE card as expected.\n");

  if (v>1)
    fprintf(stderr, "Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    return RETURNVALUE_WORK;
  }


  bll=LC_GeldKarte_BLog_List2_new();
  res=LC_GeldKarte_ReadBLogs(card, bll);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReadBLogs");
    return RETURNVALUE_WORK;
  }
  else
    rv=0;

  if (v>0)
    fprintf(stderr, "Closing card.\n");
  res=LC_Card_Close(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardClose");
    return RETURNVALUE_WORK;
  }
  if (v>0)
    fprintf(stderr, "Card closed.\n");

  if (v>0)
    fprintf(stderr, "Releasing card.\n");
  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "ReleaseCard");
    rv=RETURNVALUE_WORK;
  }
  else
    rv=0;
  LC_Card_free(card);

  if (rv==0) {
    LC_GELDKARTE_BLOG_LIST2_ITERATOR *blli;

    blli=LC_GeldKarte_BLog_List2_First(bll);
    if (blli) {
      LC_GELDKARTE_BLOG *bl;

      bl=LC_GeldKarte_BLog_List2Iterator_Data(blli);
      assert(bl);
      fprintf(stdout,
	      "Status bSEQ lSEQ hSEQ sSEQ KeyId Merchant     Value   Loaded \n");
      while(bl) {

	fprintf(stdout,
		"%6d %4d %4d %4d %4d %4d %12s %7.2f %7.2f\n",
		LC_GeldKarte_BLog_GetStatus(bl),
		LC_GeldKarte_BLog_GetBSeq(bl),
		LC_GeldKarte_BLog_GetLSeq(bl),
		LC_GeldKarte_BLog_GetHSeq(bl),
		LC_GeldKarte_BLog_GetSSeq(bl),
		LC_GeldKarte_BLog_GetKeyId(bl),
		LC_GeldKarte_BLog_GetMerchantId(bl),
		(float)(LC_GeldKarte_BLog_GetValue(bl)/100.0),
		(float)(LC_GeldKarte_BLog_GetLoaded(bl)/100.0));

	bl=LC_GeldKarte_BLog_List2Iterator_Next(blli);
      }
      LC_GeldKarte_BLog_List2Iterator_free(blli);
    }
    else {
      fprintf(stdout, I18N("No BLogs on the card\n"));
    }
  }
  return rv;
}



int main(int argc, char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  const char *s;
  LC_CLIENT *cl;
  GWEN_LOGGER_LOGTYPE logType;
  GWEN_LOGGER_LEVEL logLevel;
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
    fprintf(stdout,
            I18N("General usage: %s COMMAND [OPTIONS]\n"
                 "Allowed Commands:\n"
                 " This tool accepts the following commands:\n"
                 "  loaded\n"
                 "    show the amount of money stored on the card\n"
                 "  maxload\n"
                 "    show the maximum amount to be stored on the card\n"
                 "  maxxfer\n"
                 "    show the maximum amount which can be transferred "
                 "in one session.\n"
                 "\n"
                 "Allowed Options:\n"),
            argv[0]);
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
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
		      "geldkarte3",
		      GWEN_DB_GetCharValue(db, "logfile", 0, "geldkarte3.log"),
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

  cl=LC_Client_new("geldkarte", PROGRAM_VERSION);
  if (LC_Client_Init(cl)) {
    fprintf(stderr, "ERROR: Could not init libchipcard3.\n");
    LC_Client_free(cl);
    GWEN_DB_Group_free(db);
    return RETURNVALUE_SETUP;
  }

  /* handle command */
  if (strcasecmp(s, "loaded")==0) {
    rv=loaded(cl, db);
  }
  else if (strcasecmp(s, "maxload")==0) {
    rv=maxload(cl, db);
  }
  else if (strcasecmp(s, "maxxfer")==0) {
    rv=maxxfer(cl, db);
  }
  else if (strcasecmp(s, "blogs")==0) {
    rv=b_logs(cl, db);
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"", s);
    rv=RETURNVALUE_PARAM;
  }

  LC_Client_free(cl);
  GWEN_DB_Group_free(db);
  return 0;
}








