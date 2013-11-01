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

#define LC_CARD_EXTEND_CLIENT

#include "client_p.h"
#include "card_l.h"
#include <chipcard/sharedstuff/msgengine.h>
#include <chipcard/sharedstuff/driverinfo.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include <winscard.h>


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)


#ifndef MAX_ATR_SIZE
# define MAX_ATR_SIZE 33
#endif


#ifndef OS_WIN32
# ifndef SCARD_E_NO_READERS_AVAILABLE
#  define SCARD_E_NO_READERS_AVAILABLE 0x8010002e
# endif
#endif



static int lc_client__initcounter=0;
static GWEN_XMLNODE *lc_client__card_nodes=NULL;
static GWEN_XMLNODE *lc_client__app_nodes=NULL;
static GWEN_DB_NODE *lc_client__driver_db=NULL;
static GWEN_DB_NODE *lc_client__config=NULL;


GWEN_INHERIT_FUNCTIONS(LC_CLIENT)



GWEN_DB_NODE *LC_Client_GetCommonConfig() {
  return lc_client__config;
}



int LC_Client_InitCommon() {
  if (lc_client__initcounter==0) {
    int rv;
    GWEN_STRINGLIST *paths;

    rv=GWEN_Init();
    if (rv) {
      DBG_ERROR_ERR(LC_LOGDOMAIN, rv);
      return rv;
    }

    if (!GWEN_Logger_IsOpen(LC_LOGDOMAIN)) {
      const char *s;

      /* only set our logger if it not already has been */
      GWEN_Logger_Open(LC_LOGDOMAIN, "chipcard3-client", 0,
		       GWEN_LoggerType_Console,
                       GWEN_LoggerFacility_User);
      GWEN_Logger_SetLevel(LC_LOGDOMAIN, GWEN_LoggerLevel_Warning);

      s=getenv("LC_LOGLEVEL");
      if (s) {
        GWEN_LOGGER_LEVEL ll;

        ll=GWEN_Logger_Name2Level(s);
        if (ll!=GWEN_LoggerLevel_Unknown) {
          GWEN_Logger_SetLevel(LC_LOGDOMAIN, ll);
	  DBG_WARN(LC_LOGDOMAIN,
                   "Overriding loglevel for Libchipcard-Client with \"%s\"",
                   s);
        }
        else {
	  DBG_ERROR(0, "Unknown loglevel \"%s\"", s);
        }
      }
      else {
        GWEN_Logger_SetLevel(LC_LOGDOMAIN, GWEN_LoggerLevel_Warning);
      }
    }

    /* define sysconf path */
    GWEN_PathManager_DefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(LCC_PM_LIBNAME,
				LCC_PM_LIBNAME,
				LCC_PM_SYSCONFDIR,
				LC_CLIENT_CONFIG_DIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(LCC_PM_LIBNAME,
			     LCC_PM_LIBNAME,
			     LCC_PM_SYSCONFDIR,
			     LC_CLIENT_CONFIG_DIR);
#endif

    /* define data path */
    GWEN_PathManager_DefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
    /* add folder relative to EXE */
    GWEN_PathManager_AddRelPath(LCC_PM_LIBNAME,
				LCC_PM_LIBNAME,
				LCC_PM_DATADIR,
				LC_CLIENT_XML_DIR,
				GWEN_PathManager_RelModeExe);
#else
    /* add absolute folder */
    GWEN_PathManager_AddPath(LCC_PM_LIBNAME,
			     LCC_PM_LIBNAME,
			     LCC_PM_DATADIR,
			     LC_CLIENT_XML_DIR);
#endif

    /* load configuration file */
#if 0
    paths=GWEN_PathManager_GetPaths(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
    if (paths) {
      GWEN_DB_NODE *db;
      GWEN_BUFFER *fbuf;

      db=GWEN_DB_Group_new("config");
      fbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Directory_FindFileInPaths(paths,
					LC_CLIENT_CONFIG_FILE,
					fbuf);
      if (rv) {
	DBG_INFO(LC_LOGDOMAIN,
		 "Trying config file with suffix \".default\"");
	rv=GWEN_Directory_FindFileInPaths(paths,
					  LC_CLIENT_CONFIG_FILE".default",
					  fbuf);
      }
      GWEN_StringList_free(paths);
      if (rv) {
	DBG_WARN(LC_LOGDOMAIN,
		 "No configuration file found, using defaults");
      }
      else {
	DBG_INFO(LC_LOGDOMAIN,
		 "Reading configuration file \"%s\"",
		 GWEN_Buffer_GetStart(fbuf));
	rv=GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(fbuf),
			    GWEN_DB_FLAGS_DEFAULT |
			    GWEN_PATH_FLAGS_CREATE_GROUP);
	if (rv<0) {
	  DBG_ERROR(LC_LOGDOMAIN,
		    "Error in configuration file \"%s\" (%d)",
		    GWEN_Buffer_GetStart(fbuf), rv);
	  GWEN_Buffer_free(fbuf);
	  /* undo all init stuff so far */
	  GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
	  GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
	  return rv;
	}
      }
      GWEN_Buffer_free(fbuf);
      lc_client__config=db;
    }
    else {
      DBG_ERROR(LC_LOGDOMAIN, "Internal error: Paths not found");
      return GWEN_ERROR_INTERNAL;
    }
#else
    lc_client__config=GWEN_DB_Group_new("config");
#endif

    /* load XML files */
    paths=GWEN_PathManager_GetPaths(LCC_PM_LIBNAME, LCC_PM_DATADIR);
    if (paths) {
      GWEN_XMLNODE *n;
      GWEN_DB_NODE *db;
      GWEN_BUFFER *fbuf;
      uint32_t bpos;

      fbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_Directory_FindPathForFile(paths,
					"cards/README",
					fbuf);
      GWEN_StringList_free(paths);
      if (rv) {
	DBG_ERROR(LC_LOGDOMAIN, "Data files not found (%d)", rv);
        /* undo all init stuff so far */
	GWEN_Buffer_free(fbuf);
	GWEN_DB_Group_free(lc_client__config);
        lc_client__config=NULL;
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
        return rv;
      }

      /* load card files */
      n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "cards");
      if (LC_Client_ReadXmlFiles(n,
				 GWEN_Buffer_GetStart(fbuf),
				 "cards", "card")) {
	DBG_ERROR(LC_LOGDOMAIN, "Could not read card files");
	GWEN_XMLNode_free(n);
	/* undo all init stuff so far */
	GWEN_Buffer_free(fbuf);
	GWEN_DB_Group_free(lc_client__config);
        lc_client__config=NULL;
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
	return GWEN_ERROR_GENERIC;
      }
      lc_client__card_nodes=n;

      /* load app files */
      n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "apps");
      if (LC_Client_ReadXmlFiles(n,
				 GWEN_Buffer_GetStart(fbuf),
				 "apps", "app")) {
	DBG_ERROR(LC_LOGDOMAIN, "Could not read app files");
	GWEN_XMLNode_free(n);
	/* undo all init stuff so far */
	GWEN_XMLNode_free(lc_client__card_nodes);
	lc_client__card_nodes=NULL;
	GWEN_Buffer_free(fbuf);
	GWEN_DB_Group_free(lc_client__config);
        lc_client__config=NULL;
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
	return GWEN_ERROR_GENERIC;
      }
      lc_client__app_nodes=n;
      /*GWEN_XMLNode_WriteFile(n, "/tmp/apps", GWEN_XML_FLAGS_DEFAULT);*/

      /* load driver files (if any) */
      bpos=GWEN_Buffer_GetPos(fbuf);
      GWEN_Buffer_AppendString(fbuf, DIRSEP "drivers");
      db=GWEN_DB_Group_new("drivers");
      rv=LC_DriverInfo_ReadDrivers(GWEN_Buffer_GetStart(fbuf), db, 0, 1);
      if (rv) {
        DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
        GWEN_DB_Group_free(db);
	/* undo all init stuff so far */
	GWEN_XMLNode_free(lc_client__app_nodes);
	lc_client__app_nodes=NULL;
	GWEN_XMLNode_free(lc_client__card_nodes);
	lc_client__card_nodes=NULL;
	GWEN_Buffer_free(fbuf);
	GWEN_DB_Group_free(lc_client__config);
	lc_client__config=NULL;
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
	GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
	return rv;
      }
      lc_client__driver_db=db;
      GWEN_Buffer_Crop(fbuf, 0, bpos);

      /* insert more loading here */
      GWEN_Buffer_free(fbuf);
    }
    else {
      DBG_ERROR(LC_LOGDOMAIN, "No data files found.");
      /* undo all init stuff so far */
      GWEN_DB_Group_free(lc_client__config);
      lc_client__config=NULL;
      GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
      GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);
      return GWEN_ERROR_GENERIC;
    }
  }

  lc_client__initcounter++;
  return 0;
}



void LC_Client_FiniCommon() {
  if (lc_client__initcounter==1) {
    GWEN_DB_Group_free(lc_client__driver_db);
    lc_client__driver_db=NULL;
    GWEN_DB_Group_free(lc_client__config);
    lc_client__config=0;
    GWEN_XMLNode_free(lc_client__app_nodes);
    lc_client__app_nodes=0;
    GWEN_XMLNode_free(lc_client__card_nodes);
    lc_client__card_nodes=0;

    GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_DATADIR);
    GWEN_PathManager_UndefinePath(LCC_PM_LIBNAME, LCC_PM_SYSCONFDIR);

    GWEN_Logger_Close(LC_LOGDOMAIN);
    GWEN_Fini();
  }
  if (lc_client__initcounter>0)
    lc_client__initcounter--;
}




LC_CLIENT *LC_Client_new(const char *programName, const char *programVersion) {
  LC_CLIENT *cl;

  assert(programName);
  assert(programVersion);

  if (LC_Client_InitCommon()) {
    DBG_ERROR(0, "Unable to initialize, aborting");
    return NULL;
  }

  GWEN_NEW_OBJECT(LC_CLIENT, cl);
  GWEN_INHERIT_INIT(LC_CLIENT, cl);
  cl->programName=strdup(programName);
  cl->programVersion=strdup(programVersion);

  cl->cardNodes=lc_client__card_nodes;
  cl->appNodes=lc_client__app_nodes;
  cl->msgEngine=LC_MsgEngine_new();

  cl->dbConfig=lc_client__config;

  return cl;
}



void LC_Client_free(LC_CLIENT *cl) {
  if (cl) {
    GWEN_INHERIT_FINI(LC_CLIENT, cl);
    free(cl->programVersion);
    free(cl->programName);
    GWEN_MsgEngine_free(cl->msgEngine);

    GWEN_FREE_OBJECT(cl);

    LC_Client_FiniCommon();
  }
}





/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 * I                         Virtual functions                             I
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


LC_CLIENT_RESULT LC_Client_Init(LC_CLIENT *cl) {
  LONG rv;

  assert(cl);
  if (LC_Client_InitCommon()) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on init");
    return LC_Client_ResultInternal;
  }

  /* establish context */
  rv=SCardEstablishContext(SCARD_SCOPE_SYSTEM,    /* scope */
			   NULL,                  /* reserved1 */
			   NULL,                  /* reserved2 */
			   &(cl->scardContext));  /* ptr to context */
  if (rv!=SCARD_S_SUCCESS) {
    if (rv == SCARD_E_NO_SERVICE) {
      DBG_ERROR(LC_LOGDOMAIN,
		"SCardEstablishContext: "
		"Error SCARD_E_NO_SERVICE: "
		"The Smartcard resource manager is not running. "
		"Maybe you have to start the Smartcard service manually?");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			   I18N("The PC/SC service is not running.\n"
				"Please make sure that the package \"pcscd\" is\n"
				"installed along with the appropriate driver.\n"
				"For cyberJack devices you will need to install\n"
				"the package \"ifd-cyberjack\" (Debian) or\n"
				"\"cyberjack-ifd\" (SuSE).\n"
				"For most other readers the package \"libccid\"\n"
				"needs to be installed."
				"<html>"
				"<p>The PC/SC service is not running.</p>"
				"<p>Please make sure that the package <b>pcscd</b> is "
				"installed along with the appropriate driver.</p>"
				"<p>For cyberJack devices you will need to install "
				"the package <b>ifd-cyberjack</b> (Debian) or "
				"<b>cyberjack-ifd</b> (SuSE).</p>"
				"<p>For most other readers the package <b>libccid</b> "
				"needs to be installed.</p>"
                                "</html>"));
    }
    else {
      DBG_ERROR(LC_LOGDOMAIN,
		"SCardEstablishContext: %ld (%04lx)", (long int) rv,
		rv);
    }
    LC_Client_FiniCommon();
    return LC_Client_ResultIoError;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Client_Fini(LC_CLIENT *cl) {
  LONG rv;

  rv=SCardReleaseContext(cl->scardContext);
  if (rv!=SCARD_S_SUCCESS) {
    DBG_ERROR(LC_LOGDOMAIN,
	      "SCardReleaseContext: %04lx", (long unsigned int) rv);
    LC_Client_FiniCommon();
    return LC_Client_ResultIoError;
  }

  LC_Client_FiniCommon();
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Client_ConnectCard(LC_CLIENT *cl,
				       const char *rname,
				       LC_CARD **pCard) {
  LC_CLIENT_RESULT res;
  LONG rv;
  SCARDHANDLE scardHandle;
  DWORD dwActiveProtocol;
  LC_CARD *card;
  char readerName[256];
  DWORD pcchReaderLen;
  BYTE pbAtr[MAX_ATR_SIZE];
  DWORD dwAtrLen;
  DWORD dwState;
  GWEN_BUFFER *bDriverType;
  GWEN_BUFFER *bReaderType;
  uint32_t rflags=0;

  assert(cl);

  DBG_INFO(LC_LOGDOMAIN, "Trying protocol T1");
  rv=SCardConnect(cl->scardContext,
		  rname,
                  SCARD_SHARE_EXCLUSIVE,
                  SCARD_PROTOCOL_T1,
                  &scardHandle,
                  &dwActiveProtocol);
  if (rv!=SCARD_S_SUCCESS) {
    DBG_INFO(LC_LOGDOMAIN, "Trying protocol T0");
    rv=SCardConnect(cl->scardContext,
		    rname,
		    SCARD_SHARE_EXCLUSIVE,
		    SCARD_PROTOCOL_T0,
                    &scardHandle,
                    &dwActiveProtocol);
  }
#ifdef SCARD_PROTOCOL_RAW
  if (rv!=SCARD_S_SUCCESS) {
    DBG_INFO(LC_LOGDOMAIN, "Trying protocol RAW");
    rv=SCardConnect(cl->scardContext,
		    rname,
		    SCARD_SHARE_EXCLUSIVE,
		    SCARD_PROTOCOL_RAW,
		    &scardHandle,
		    &dwActiveProtocol);
  }
#endif

  if (rv!=SCARD_S_SUCCESS) {
    DBG_INFO(LC_LOGDOMAIN,
	     "SCardConnect: %04lx", (long unsigned int) rv);
    return LC_Client_ResultIoError;
  }

  /* get protocol and ATR */
  DBG_INFO(LC_LOGDOMAIN, "Reading protocol and ATR");
  pcchReaderLen=sizeof(readerName);
  dwAtrLen=sizeof(pbAtr);
  rv=SCardStatus(scardHandle,
                 readerName,
                 &pcchReaderLen,
                 &dwState,
                 &dwActiveProtocol,
                 pbAtr,
                 &dwAtrLen);

  if (rv!=SCARD_S_SUCCESS) {
    DBG_ERROR(LC_LOGDOMAIN,
	      "SCardStatus: %04lx", (long unsigned int) rv);
    SCardDisconnect(scardHandle, SCARD_UNPOWER_CARD);
    return LC_Client_ResultIoError;
  }

  /* derive reader and driver type from name */
  DBG_INFO(LC_LOGDOMAIN, "Getting reader- and driver type");
  bDriverType=GWEN_Buffer_new(0, 32, 0, 1);
  bReaderType=GWEN_Buffer_new(0, 32, 0, 1);
  res=LC_Client_GetReaderAndDriverType(cl,
				       readerName,
				       bDriverType,
				       bReaderType,
				       &rflags);
  if (res) {
    DBG_INFO(LC_LOGDOMAIN,
	     "Unable to determine type of reader [%s] (%d), assuming generic pcsc",
	     readerName,
	     res);
    GWEN_Buffer_AppendString(bDriverType, "generic_pcsc");
    GWEN_Buffer_AppendString(bReaderType, "generic_pcsc");
  }

  /* create new card */
  card=LC_Card_new(cl,
		   scardHandle,
		   readerName,
		   dwActiveProtocol,
		   "processor",      /* cardType */
		   rflags,
		   dwAtrLen?pbAtr:0, /* atrBuf */
		   dwAtrLen);        /* atrLen */

  /* complete card data */
  LC_Card_SetDriverType(card, GWEN_Buffer_GetStart(bDriverType));
  LC_Card_SetReaderType(card, GWEN_Buffer_GetStart(bReaderType));

  GWEN_Buffer_free(bReaderType);
  GWEN_Buffer_free(bDriverType);

  *pCard=card;

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Client_ExecApdu(LC_CLIENT *cl,
                                    LC_CARD *card,
                                    const char *apdu,
				    unsigned int apdulen,
                                    GWEN_BUFFER *rbuf,
                                    LC_CLIENT_CMDTARGET t) {
  LONG rv;
  unsigned char rbuffer[300];
  DWORD rblen;

  assert(cl);
  assert(card);
  assert(apdu);
  assert(apdulen>3);

  if (t==LC_Client_CmdTargetReader) {
    int feature;
    uint32_t controlCode;

    feature=apdu[0];
    controlCode=
        (apdu[1]<<24)+
        (apdu[2]<<16)+
        (apdu[3]<<8)+
      apdu[4];
    if (feature && controlCode==0)
      controlCode=LC_Card_GetFeatureCode(card, feature);

    if (controlCode==0) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Bad control code for feature %d of reader \"%s\"",
                feature,
		LC_Card_GetReaderName(card));
      return LC_Client_ResultInvalid;
    }

    DBG_DEBUG(LC_LOGDOMAIN, "Sending command to reader (control: %08x):",
              controlCode);
    GWEN_Text_LogString((const char*)apdu+5, apdulen-5,
                        LC_LOGDOMAIN,
			GWEN_LoggerLevel_Debug);

    rblen=sizeof(rbuffer);
    rv=SCardControl(LC_Card_GetSCardHandle(card),
		    controlCode,
		    apdu+5,
		    apdulen-5,
		    rbuffer,
		    sizeof(rbuffer),
                    &rblen);
    if (rv!=SCARD_S_SUCCESS) {
      DBG_ERROR(LC_LOGDOMAIN,
                "SCardControl: %04lx", (long unsigned int) rv);
      return LC_Client_ResultIoError;
    }
    if (rblen) {
      GWEN_Buffer_AppendBytes(rbuf, (const char*)rbuffer, rblen);
      if (rblen>1) {
	LC_Card_SetLastResult(card, "ok",
			      "SCardControl succeeded",
			      rbuffer[rblen-2],
			      rbuffer[rblen-1]);
      }
    }
    return LC_Client_ResultOk;
  }
  else {
    SCARD_IO_REQUEST txHeader;
    SCARD_IO_REQUEST rxHeader;

    DBG_DEBUG(LC_LOGDOMAIN, "Sending command to card:");
    GWEN_Text_LogString((const char*)apdu, apdulen,
                        LC_LOGDOMAIN,
                        GWEN_LoggerLevel_Debug);
    txHeader.dwProtocol=LC_Card_GetProtocol(card);
    //txHeader.dwProtocol=1;
    txHeader.cbPciLength=sizeof(txHeader);
    rxHeader.cbPciLength=sizeof(rxHeader);
    rblen=sizeof(rbuffer);
    rv=SCardTransmit(LC_Card_GetSCardHandle(card),
                     &txHeader,
                     (LPCBYTE) apdu,
                     apdulen,
                     &rxHeader,
                     rbuffer,
                     &rblen);
    if (rv!=SCARD_S_SUCCESS) {
      DBG_ERROR(LC_LOGDOMAIN,
                "SCardControl: %04lx", (long unsigned int) rv);
      return LC_Client_ResultIoError;
    }
    DBG_DEBUG(LC_LOGDOMAIN, "Received response:");
    GWEN_Text_LogString((const char*)rbuffer, rblen,
			LC_LOGDOMAIN,
			GWEN_LoggerLevel_Debug);
    if (rblen) {
      GWEN_Buffer_AppendBytes(rbuf, (const char*)rbuffer, rblen);
      if (rblen>1) {
	LC_Card_SetLastResult(card, "ok",
			      "SCardTransmit succeeded",
			      rbuffer[rblen-2],
			      rbuffer[rblen-1]);
      }
    }
    else {
      DBG_DEBUG(LC_LOGDOMAIN, "Empty response");
    }
    return LC_Client_ResultOk;
  }
}






/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 * I                     Informational functions                           I
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


const char *LC_Client_GetProgramName(const LC_CLIENT *cl) {
  assert(cl);
  return cl->programName;
}



const char *LC_Client_GetProgramVersion(const LC_CLIENT *cl) {
  assert(cl);
  return cl->programVersion;
}



GWEN_XMLNODE *LC_Client_GetAppNodes(const LC_CLIENT *cl) {
  assert(cl);
  return cl->appNodes;
}



GWEN_XMLNODE *LC_Client_GetCardNodes(const LC_CLIENT *cl) {
  assert(cl);
  return cl->cardNodes;
}



GWEN_MSGENGINE *LC_Client_GetMsgEngine(const LC_CLIENT *cl) {
  assert(cl);
  return cl->msgEngine;
}



int LC_Client_GetReaderAndDriverType(const LC_CLIENT *cl,
				     const char *readerName,
				     GWEN_BUFFER *driverType,
				     GWEN_BUFFER *readerType,
				     uint32_t *pReaderFlags) {
  GWEN_DB_NODE *dbDriver;

  dbDriver=GWEN_DB_FindFirstGroup(lc_client__driver_db, "driver");
  while(dbDriver) {
    const char *sDriverName;

    sDriverName=GWEN_DB_GetCharValue(dbDriver, "driverName", 0, NULL);
    if (sDriverName) {
      GWEN_DB_NODE *dbReader;

      dbReader=GWEN_DB_FindFirstGroup(dbDriver, "reader");
      while(dbReader) {
	const char *sReaderName;
	const char *sTmpl;

	sReaderName=GWEN_DB_GetCharValue(dbReader, "readerType", 0, NULL);
	sTmpl=GWEN_DB_GetCharValue(dbReader, "devicePathTmpl", 0, NULL);
	if (sReaderName && sTmpl) {
	  if (-1!=GWEN_Text_ComparePattern(readerName, sTmpl, 1)) {
	    /* reader found */
	    GWEN_Buffer_AppendString(driverType, sDriverName);
	    GWEN_Buffer_AppendString(readerType, sReaderName);
	    *pReaderFlags=LC_ReaderFlags_fromDb(dbReader, "flags");
	    DBG_INFO(LC_LOGDOMAIN,
		     "Reader [%s] is [%s]/[%s], %08x",
		     readerName,
		     sDriverName, sReaderName, *pReaderFlags);
	    return 0;
	  }
	}
	else {
	  DBG_INFO(LC_LOGDOMAIN,
                   "Either reader name or template missing");
	}
	dbReader=GWEN_DB_FindNextGroup(dbReader, "reader");
      }
    }
    else {
      DBG_INFO(LC_LOGDOMAIN,
	       "Driver name is missing");
    }
    dbDriver=GWEN_DB_FindNextGroup(dbDriver, "driver");
  }

  return GWEN_ERROR_NOT_FOUND;
}



GWEN_DB_NODE *LC_Client_GetConfig(const LC_CLIENT *cl) {
  assert(cl);
  return cl->dbConfig;
}



int LC_Client_FindReaderState(LC_CLIENT *cl, const char *readerName) {
  int i;

  assert(cl);
  for (i=0; i<cl->readerCount; i++) {
    if (strcasecmp(cl->readerStates[i].szReader, readerName)==0)
      return i;
  }

  return -1;
}



int LC_Client_UpdateReaderStates(LC_CLIENT *cl) {
  LONG rv;
  LPSTR mszGroups=0;
  LPSTR mszReaders=0;
  DWORD dwReaders=0;
  const char *p;
  int i, j;

  assert(cl);

  /* allocate buffer for reader list */
  rv=SCardListReaders(cl->scardContext,   /* context */
		      NULL,               /* mszGroups */
		      NULL,               /* mszReaders */
		      &dwReaders);
  if (rv!=SCARD_S_SUCCESS) {
    if (rv==SCARD_E_NO_READERS_AVAILABLE) {
      DBG_ERROR(LC_LOGDOMAIN,
		"No readers available");
    }
    else {
      DBG_ERROR(LC_LOGDOMAIN,
		"SCardListReaders(1): %08lx", (long unsigned int) rv);
    }
    return LC_Client_ResultIoError;
  }
  mszReaders=(LPSTR)malloc(sizeof(char)*dwReaders);
  if (mszReaders==0) {
    return LC_Client_ResultInternal;
  }

  /* list readers */
  rv=SCardListReaders(cl->scardContext,   /* context */
                      mszGroups,          /* mszGroups */
                      mszReaders,         /* mszReaders */
                      &dwReaders);
  if (rv!=SCARD_S_SUCCESS) {
    DBG_ERROR(LC_LOGDOMAIN,
              "SCardListReaders(2): %04lx", (long unsigned int) rv);
    return LC_Client_ResultIoError;
  }

  /* delete removed readers */
  for (i=0; i<cl->readerCount; i++) {
    int found=0;

    /* find reader */
    p=(const char*)mszReaders;
    while(*p) {
      if (strcasecmp(cl->readerStates[i].szReader, p)==0) {
	/* re-assign reader name, because we are about to exchange the readerList string */
        cl->readerStates[i].szReader=p;
	found=1;
        break;
      }
      while(*p)
	p++;
      p++;
    } /* while */

    if (!found) {
      /* not in the reader list, remove */
      for (j=i; j<(cl->readerCount-1); j++)
	cl->readerStates[j]=cl->readerStates[j+1];
      cl->readerCount--;
    }
  }

  /* add new readers  */
  p=(const char*)mszReaders;
  while(*p) {

    i=LC_Client_FindReaderState(cl, p);
    if (i!=-1) {
      DBG_INFO(LC_LOGDOMAIN, "Reader \"%s\" already listed", p);
    }
    else {
      if (cl->readerCount<MAX_READERS) {
	DBG_INFO(LC_LOGDOMAIN, "Creating reader \"%s\"", p);
	i=cl->readerCount;
        /* preset */
	memset((void*) &(cl->readerStates[i]), 0, sizeof(SCARD_READERSTATE));
	cl->readerStates[i].szReader=p;
	cl->readerStates[i].dwCurrentState=SCARD_STATE_UNAWARE;
	/* reader added */
	cl->readerCount++;
      }
      else {
	DBG_ERROR(LC_LOGDOMAIN, "Too many readers (%d)",
		  cl->readerCount);
      }
    }
    /* next reader */
    while(*p)
      p++;
    p++;
  } /* while */

  if (cl->pnpAvailable) {
    if (-1==LC_Client_FindReaderState(cl, "\\\\?PnP?\\Notification")) {
      /* add pnp reader */
      if (cl->readerCount<MAX_READERS) {
	cl->readerStates[cl->readerCount].szReader = "\\\\?PnP?\\Notification";
	cl->readerStates[cl->readerCount].dwCurrentState = SCARD_STATE_UNAWARE;
	cl->readerCount++;
      }
      else {
	DBG_ERROR(LC_LOGDOMAIN, "Too many readers (%d)",
		  cl->readerCount);
      }
    }
  }

  /* replace reader string */
  free(cl->readerList);
  cl->readerList=mszReaders;

  return 0;
}



LC_CLIENT_RESULT LC_Client_Start(LC_CLIENT *cl) {
  LONG rv;

  assert(cl);

#if 0
  /* check whether pnp pseudo reader is available */
  cl->readerStates[0].szReader = "\\\\?PnP?\\Notification";
  cl->readerStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
  rv=SCardGetStatusChange(cl->scardContext, 0, cl->readerStates, 1);

  if (cl->readerStates[0].dwEventState && SCARD_STATE_UNKNOWN) {
    DBG_INFO(LC_LOGDOMAIN, "PnP not supported");
    cl->pnpAvailable=0;
  }
  else
    cl->pnpAvailable=1;
#endif

  rv=LC_Client_UpdateReaderStates(cl);
  if (rv<0) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", (int) rv);
    return LC_Client_ResultGeneric;
  }
  cl->lastUsedReader=-1;

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Client_Stop(LC_CLIENT *cl) {
  assert(cl);

  /* clear reader list and reader status list */
  memset((void*) &cl->readerStates, 0, sizeof(SCARD_READERSTATE)*MAX_READERS);
  cl->readerCount=0;
  free(cl->readerList);
  cl->readerList=NULL;

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Client_GetNextCard(LC_CLIENT *cl, LC_CARD **pCard, int timeout) {
  LONG rv;
  int i;
  uint32_t progressId;
  time_t startt;
  uint64_t to;
  int distance;

  assert(cl);

  startt=time(0);
  if (timeout==GWEN_TIMEOUT_NONE ||
      timeout==GWEN_TIMEOUT_FOREVER)
    to=0;
  else
    to=timeout;

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
				    GWEN_GUI_PROGRESS_ALLOW_EMBED |
				    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    I18N("Waiting for card to be inserted"),
				    NULL,
				    to,
				    0);

  distance=GWEN_GUI_CHECK_PERIOD;
  if (distance>timeout)
    distance=timeout;

  for (;;) {
    double d;
    int err;

    /* continue checking */
    for (i=cl->lastUsedReader+1; i<cl->readerCount; i++) {
      /* we have a change here */
      if (cl->readerStates[i].dwEventState & SCARD_STATE_CHANGED)
	cl->readerStates[i].dwCurrentState=cl->readerStates[i].dwEventState;
      else
        continue;

      DBG_DEBUG(LC_LOGDOMAIN, "Status changed on reader [%s] (%08x, %08x)",
		cl->readerStates[i].szReader,
		(unsigned int)(cl->readerStates[i].dwCurrentState),
		(unsigned int)(cl->readerStates[i].dwEventState));

      if (cl->pnpAvailable && i==cl->readerCount-1) {
	/* pnp pseudo reader: a reader has been added or removed */
	DBG_DEBUG(LC_LOGDOMAIN, "Pseudo reader, updating reader list (%08x, %08x)",
		  (unsigned int)(cl->readerStates[i].dwCurrentState),
		  (unsigned int)(cl->readerStates[i].dwEventState));
	LC_Client_UpdateReaderStates(cl);
	cl->lastUsedReader=-1;
	break;
      }
      else {
	if ((cl->readerStates[i].dwEventState & SCARD_STATE_PRESENT) &&
	    !(cl->readerStates[i].dwEventState & SCARD_STATE_EXCLUSIVE) &&
	    !(cl->readerStates[i].dwEventState & SCARD_STATE_INUSE)) {
	  LC_CLIENT_RESULT res;
	  LC_CARD *card=NULL;

	  /* card inserted and not used by another application */
	  DBG_DEBUG(LC_LOGDOMAIN, "Found usable card in reader [%s]", cl->readerStates[i].szReader);
	  res=LC_Client_ConnectCard(cl, cl->readerStates[i].szReader, &card);
	  if (res==LC_Client_ResultOk) {
	    /* card csuccessfully connected, return */
	    *pCard=card;
	    cl->lastUsedReader=i;
	    GWEN_Gui_ProgressEnd(progressId);
	    return LC_Client_ResultOk;
	  }
	  else {
	    DBG_ERROR(LC_LOGDOMAIN,
		      "Error connecting to card in reader [%s]",
		      cl->readerStates[i].szReader);
	  }
	}
	else {
	  DBG_INFO(LC_LOGDOMAIN, "Either no card in reader or card unavailable in reader [%s]",
		    cl->readerStates[i].szReader);
	}
      }
    }

    if (i>=cl->readerCount) {
      /* there was no relevant change in a reader, wait for status change */
      cl->lastUsedReader=-1;
      rv=SCardGetStatusChange(cl->scardContext, distance, cl->readerStates, cl->readerCount);
      if (rv==SCARD_E_TIMEOUT) {
	/* timeout, just repeat next loop */
	if (timeout==GWEN_TIMEOUT_NONE) {
	  GWEN_Gui_ProgressEnd(progressId);
	  return LC_Client_ResultWait;
	}
      }
      else if (rv!=SCARD_S_SUCCESS) {
	DBG_ERROR(LC_LOGDOMAIN, "SCardGetStatusChange: %d", (int) rv);
	GWEN_Gui_ProgressEnd(progressId);
        return LC_Client_ResultIoError;
      }
    }

    /* check timeout */
    d=difftime(time(0), startt);
    if (timeout!=GWEN_TIMEOUT_FOREVER) {
      if (timeout==GWEN_TIMEOUT_NONE ||
	  d>timeout) {
	DBG_INFO(GWEN_LOGDOMAIN,
		 "Timeout (%d) while waiting, giving up",
		 timeout);
	GWEN_Gui_ProgressEnd(progressId);
	return LC_Client_ResultWait;
      }
    }

    /* check for user abort */
    err=GWEN_Gui_ProgressAdvance(progressId, (uint64_t)(d*1000));
    if (err==GWEN_ERROR_USER_ABORTED) {
      DBG_ERROR(GWEN_LOGDOMAIN, "User aborted");
      GWEN_Gui_ProgressEnd(progressId);
      return LC_Client_ResultAborted;
    }
  }
}



LC_CLIENT_RESULT LC_Client_ReleaseCard(LC_CLIENT *cl, LC_CARD *card) {
  LONG rv;

  assert(cl);
  assert(card);

  rv=SCardDisconnect(LC_Card_GetSCardHandle(card), SCARD_RESET_CARD);
  if (rv!=SCARD_S_SUCCESS) {
    DBG_ERROR(LC_LOGDOMAIN, "SCardDisconnect: %04lx", (long unsigned int) rv);
    return LC_Client_ResultIoError;
  }

  return LC_Client_ResultOk;
}









#include "client_xml.c"
#include "client_cmd.c"



