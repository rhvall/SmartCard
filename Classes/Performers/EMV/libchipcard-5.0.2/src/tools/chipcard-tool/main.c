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

#include "global.h"
#include <gwenhywfar/args.h>
#include <gwenhywfar/cgui.h>

#define PROGRAM_VERSION "3.0"


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
  case LC_Client_ResultNotSupported:
    s="Function not supported.";
    break;
  default:
    s="Unknown error.";
    break;
  }

  fprintf(stderr, "Error in \"%s\": %s (%d)\n", x, s, res);
  if (res==LC_Client_ResultCmdError && card) {
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

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf,
			     I18N("Usage: "));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf,
    			     I18N(" [GLOBAL OPTIONS] COMMAND"
			          " [LOCAL OPTIONS]\n"));
    if (GWEN_Args_Usage(prg_args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "Could not generate help string.\n");
      GWEN_Buffer_free(ubuf);
      return RETURNVALUE_PARAM;
    }
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n\n"));
    GWEN_Buffer_AppendString(ubuf,
    			     I18N("  atr:\n"
			          "    Read the ATR data from a chipcard\n\n"));
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
		      "chipcard-tool",
		      GWEN_DB_GetCharValue(db, "logfile", 0, "chipcard-tool.log"),
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

  cl=LC_Client_new("chipcard-tool", PROGRAM_VERSION);
  res=LC_Client_Init(cl);
  if (res!=LC_Client_ResultOk) {
    fprintf(stderr, "ERROR: Could not initialize libchipcard3.\n");
    LC_Client_free(cl);
    GWEN_DB_Group_free(db);
    return RETURNVALUE_SETUP;
  }

  /* handle command */
  if (strcasecmp(s, "list")==0) {
    fprintf(stderr, "Command \"list\" no longer supported\n");
    rv=RETURNVALUE_PARAM;
  }
  else if (strcasecmp(s, "check")==0) {
    fprintf(stderr, "Command \"check\" no longer supported\n");
    rv=RETURNVALUE_PARAM;
  }
  else if (strcasecmp(s, "atr")==0) {
    rv=getAtr(cl, db);
  }
  else if (strcasecmp(s, "monitor")==0) {
    fprintf(stderr, "Command \"monitor\" no longer supported\n");
    rv=RETURNVALUE_PARAM;
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", s);
    rv=RETURNVALUE_PARAM;
  }

  LC_Client_free(cl);
  GWEN_DB_Group_free(db);
  return 0;
}








