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

#include "ddvcard_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/ctplugin_be.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>

#include <chipcard/cards/ddvcard.h>
#include <chipcard/cards/processorcard.h>
#include <chipcard/ct/ct_card.h>

#define I18N(message) GWEN_I18N_Translate("libchipcard", message)



GWEN_INHERIT(GWEN_CRYPT_TOKEN, LC_CT_DDV)
GWEN_INHERIT(GWEN_PLUGIN, LC_CT_PLUGIN_DDV)



GWEN_PLUGIN *ct_ddvcard_factory(GWEN_PLUGIN_MANAGER *pm,
				const char *modName,
				const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=LC_Crypt_TokenDDV_Plugin_new(pm, modName, fileName);
  if (pl==NULL) {
    DBG_ERROR(LC_LOGDOMAIN, "No plugin created");
    return NULL;
  }

  return pl;
}



GWEN_PLUGIN *LC_Crypt_TokenDDV_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					  const char *modName,
					  const char *fileName) {
  GWEN_PLUGIN *pl;
  LC_CT_PLUGIN_DDV *cpl;
  LC_CLIENT_RESULT res;

  pl=GWEN_Crypt_Token_Plugin_new(pm,
				GWEN_Crypt_Token_Device_Card,
				modName,
				fileName);

  GWEN_NEW_OBJECT(LC_CT_PLUGIN_DDV, cpl);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, LC_CT_PLUGIN_DDV, pl, cpl,
		       LC_Crypt_TokenDDV_Plugin_FreeData);
  cpl->client=LC_Client_new("LC_Crypt_TokenDDV", VERSION);
  res=LC_Client_Init(cpl->client);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN,
	      "Error initialising libchipcard (%d), chipcards will not be available",
	      res);
    GWEN_Plugin_free(pl);
    return NULL;
  }

  /* set virtual functions */
  GWEN_Crypt_Token_Plugin_SetCreateTokenFn(pl, LC_Crypt_TokenDDV_Plugin_CreateToken);
  GWEN_Crypt_Token_Plugin_SetCheckTokenFn(pl, LC_Crypt_TokenDDV_Plugin_CheckToken);
  return pl;
}



void GWENHYWFAR_CB LC_Crypt_TokenDDV_Plugin_FreeData(void *bp, void *p) {
  LC_CT_PLUGIN_DDV *cpl;

  cpl=(LC_CT_PLUGIN_DDV*)p;
  LC_Client_free(cpl->client);
  GWEN_FREE_OBJECT(cpl);
}



GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
LC_Crypt_TokenDDV_Plugin_CreateToken(GWEN_PLUGIN *pl,
				     const char *name) {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_CRYPT_TOKEN *ct;
  LC_CT_PLUGIN_DDV *cpl;

  assert(pl);
  cpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, LC_CT_PLUGIN_DDV, pl);
  assert(cpl);

  pm=GWEN_Plugin_GetManager(pl);
  assert(pm);

  ct=LC_Crypt_TokenDDV_new(pm, cpl->client, name);
  assert(ct);

  return ct;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_Plugin_CheckToken(GWEN_PLUGIN *pl,
				    GWEN_BUFFER *name) {
  GWEN_PLUGIN_MANAGER *pm;
  LC_CT_PLUGIN_DDV *cpl;
  LC_CLIENT_RESULT res;
  LC_CARD *hcard=0;
  const char *currCardNumber;
  int i;

  assert(pl);
  cpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, LC_CT_PLUGIN_DDV, pl);
  assert(cpl);

  pm=GWEN_Plugin_GetManager(pl);
  assert(pm);

  res=LC_Client_Start(cpl->client);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not send StartWait request");
    return GWEN_ERROR_IO;
  }

  for (i=0;i<10;i++) {
    res=LC_Client_GetNextCard(cpl->client, &hcard, i==0?5:10);
    if (res==LC_Client_ResultOk)
      break;
    else {
      if (res==LC_Client_ResultWait) {
	int mres;

	mres=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
				 GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
				 GWEN_GUI_MSG_FLAGS_TYPE_INFO,
				 I18N("Insert card"),
				 I18N("Please insert a chipcard into the reader "
				      "and click a button."
				      "<html>"
				      "Please insert a chipcard into the reader "
				      "and click a button."
				      "</html>"),
				 I18N("Ok"),
				 I18N("Abort"),
				 NULL,
				 0);
	if (mres!=1) {
	  DBG_ERROR(LC_LOGDOMAIN, "Error in user interaction (%d)", mres);
	  LC_Client_Stop(cpl->client);
	  return GWEN_ERROR_USER_ABORTED;
	}
      }
      else {
	GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			     I18N("Error while waiting for card"));
	LC_Client_Stop(cpl->client);
	return GWEN_ERROR_IO;
      }
    }
  }

  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN,
	      "No card within specified timeout (%d)", res);
    LC_Client_Stop(cpl->client);
    return GWEN_ERROR_IO;
  }
  else {
    int rv;

    assert(hcard);
    /* ok, we have a card, don't wait for more */
    LC_Client_Stop(cpl->client);
    /* check card */
    rv=LC_DDVCard_ExtendCard(hcard);
    if (rv) {
      DBG_ERROR(LC_LOGDOMAIN,
		"DDV card not available, please check your setup (%d)", rv);
      LC_Client_ReleaseCard(cpl->client, hcard);
      LC_Card_free(hcard);
      return GWEN_ERROR_NOT_AVAILABLE;
    }

    res=LC_Card_Open(hcard);
    if (res!=LC_Client_ResultOk) {
      LC_Client_ReleaseCard(cpl->client, hcard);
      LC_Card_free(hcard);
      DBG_NOTICE(LC_LOGDOMAIN,
		 "Could not open card (%d), maybe not a DDV card?",
		 res);
      return GWEN_ERROR_NOT_SUPPORTED;
    } /* if card not open */
    else {
      GWEN_DB_NODE *dbCardData;

        dbCardData=LC_DDVCard_GetCardDataAsDb(hcard);
	assert(dbCardData);

        currCardNumber=GWEN_DB_GetCharValue(dbCardData,
                                            "cardNumber",
                                            0,
                                            0);
	if (!currCardNumber) {
          DBG_ERROR(LC_LOGDOMAIN, "INTERNAL: No card number in card data.");
          abort();
        }

        DBG_NOTICE(LC_LOGDOMAIN, "Card number: %s", currCardNumber);

	if (GWEN_Buffer_GetUsedBytes(name)==0) {
	  DBG_NOTICE(LC_LOGDOMAIN, "No or empty token name");
	  GWEN_Buffer_AppendString(name, currCardNumber);
	}
	else {
	  if (strcasecmp(GWEN_Buffer_GetStart(name), currCardNumber)!=0) {
	    DBG_ERROR(LC_LOGDOMAIN, "Card supported, but bad name");
	    LC_Card_Close(hcard);
            LC_Client_ReleaseCard(cpl->client, hcard);
            LC_Card_free(hcard);
	    return GWEN_ERROR_BAD_NAME;
	  }
	}

        LC_Card_Close(hcard);
        LC_Client_ReleaseCard(cpl->client, hcard);
        LC_Card_free(hcard);
        hcard=0;
    } /* if card is open */
    return 0;
  } /* if there is a card */
}



GWEN_CRYPT_TOKEN *LC_Crypt_TokenDDV_new(GWEN_PLUGIN_MANAGER *pm,
					LC_CLIENT *lc,
					const char *name) {
  LC_CT_DDV *lct;
  GWEN_CRYPT_TOKEN *ct;

  DBG_INFO(LC_LOGDOMAIN, "Creating crypttoken (DDV)");

  /* create crypt token */
  ct=GWEN_Crypt_Token_new(GWEN_Crypt_Token_Device_Card,
			  "ddvcard", name);

  /* inherit CryptToken: Set our own data */
  GWEN_NEW_OBJECT(LC_CT_DDV, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct, lct,
                       LC_Crypt_TokenDDV_FreeData);
  lct->pluginManager=pm;
  lct->client=lc;

  /* set virtual functions */
  GWEN_Crypt_Token_SetOpenFn(ct, LC_Crypt_TokenDDV_Open);
  GWEN_Crypt_Token_SetCloseFn(ct, LC_Crypt_TokenDDV_Close);

  GWEN_Crypt_Token_SetGetKeyIdListFn(ct, LC_Crypt_TokenDDV_GetKeyIdList);
  GWEN_Crypt_Token_SetGetKeyInfoFn(ct, LC_Crypt_TokenDDV_GetKeyInfo);
  GWEN_Crypt_Token_SetSetKeyInfoFn(ct, LC_Crypt_TokenDDV_SetKeyInfo);
  GWEN_Crypt_Token_SetGetContextIdListFn(ct, LC_Crypt_TokenDDV_GetContextIdList);
  GWEN_Crypt_Token_SetGetContextFn(ct, LC_Crypt_TokenDDV_GetContext);
  GWEN_Crypt_Token_SetSetContextFn(ct, LC_Crypt_TokenDDV_SetContext);

  GWEN_Crypt_Token_SetSignFn(ct, LC_Crypt_TokenDDV_Sign);
  GWEN_Crypt_Token_SetVerifyFn(ct, LC_Crypt_TokenDDV_Verify);
  GWEN_Crypt_Token_SetEncipherFn(ct, LC_Crypt_TokenDDV_Encipher);
  GWEN_Crypt_Token_SetDecipherFn(ct, LC_Crypt_TokenDDV_Decipher);

  return ct;
}



void GWENHYWFAR_CB LC_Crypt_TokenDDV_FreeData(void *bp, void *p) {
  LC_CT_DDV *lct;

  lct=(LC_CT_DDV*)p;
  if (lct->card) {
    LC_Client_ReleaseCard(lct->client, lct->card);
    LC_Card_free(lct->card);
  }
  GWEN_FREE_OBJECT(lct);
}



int LC_Crypt_TokenDDV__GetCard(GWEN_CRYPT_TOKEN *ct, uint32_t guiid) {
  LC_CT_DDV *lct;
  LC_CLIENT_RESULT res;
  LC_CARD *hcard=0;
  int first;
  const char *currCardNumber;
  const char *name;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  name=GWEN_Crypt_Token_GetTokenName(ct);

  res=LC_Client_Start(lct->client);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not send Start request");
    return GWEN_ERROR_IO;
  }

  first=1;
  hcard=0;
  for (;;) {
    int timeout;

    /* determine timeout value */
    if (first)
      timeout=3;
    else
      timeout=5;

    if (hcard==0) {
      res=LC_Client_GetNextCard(lct->client, &hcard, timeout);
      if (res!=LC_Client_ResultOk &&
	  res!=LC_Client_ResultWait) {
	DBG_ERROR(LC_LOGDOMAIN, "Error while waiting for card (%d)", res);
	return GWEN_ERROR_IO;
      }
    }
    if (!hcard) {
      int mres;

      mres=GWEN_Crypt_Token_InsertToken(ct, guiid);
      if (mres) {
        DBG_ERROR(LC_LOGDOMAIN, "Error in user interaction (%d)", mres);
        LC_Client_Stop(lct->client);
        return GWEN_ERROR_USER_ABORTED;
      }
    }
    else {
      int rv;

      /* ok, we have a card, now check it */
      rv=LC_DDVCard_ExtendCard(hcard);
      if (rv) {
        DBG_ERROR(LC_LOGDOMAIN,
                  "DDV card not available, please check your setup (%d)", rv);
        LC_Client_ReleaseCard(lct->client, hcard);
        LC_Card_free(hcard);
	LC_Client_Stop(lct->client);
        return GWEN_ERROR_NOT_AVAILABLE;
      }

      res=LC_Card_Open(hcard);
      if (res!=LC_Client_ResultOk) {
        LC_Client_ReleaseCard(lct->client, hcard);
        LC_Card_free(hcard);
        hcard=0;
        DBG_NOTICE(LC_LOGDOMAIN,
                   "Could not open card (%d), maybe not a DDV card?",
                   res);
      } /* if card not open */
      else {
        GWEN_DB_NODE *dbCardData;

        dbCardData=LC_DDVCard_GetCardDataAsDb(hcard);
	assert(dbCardData);

        currCardNumber=GWEN_DB_GetCharValue(dbCardData,
                                            "cardNumber",
                                            0,
                                            0);
	if (!currCardNumber) {
          DBG_ERROR(LC_LOGDOMAIN, "INTERNAL: No card number in card data.");
          abort();
        }

        DBG_NOTICE(LC_LOGDOMAIN, "Card number: %s", currCardNumber);

        if (!name || !*name) {
          DBG_NOTICE(LC_LOGDOMAIN, "No or empty token name");
          GWEN_Crypt_Token_SetTokenName(ct, currCardNumber);
          name=GWEN_Crypt_Token_GetTokenName(ct);
          break;
        }

        if (strcasecmp(name, currCardNumber)==0) {
          DBG_NOTICE(LC_LOGDOMAIN, "Card number equals");
          break;
        }

        LC_Card_Close(hcard);
        LC_Client_ReleaseCard(lct->client, hcard);
        LC_Card_free(hcard);
        hcard=0;

	res=LC_Client_GetNextCard(lct->client, &hcard, GWEN_TIMEOUT_NONE);
	if (res!=LC_Client_ResultOk) {
	  int mres;

	  if (res!=LC_Client_ResultWait) {
	    DBG_ERROR(LC_LOGDOMAIN,
		      "Communication error (%d)", res);
	    LC_Client_Stop(lct->client);
	    return GWEN_ERROR_IO;
	  }

	  mres=GWEN_Crypt_Token_InsertCorrectToken(ct, guiid);
	  if (mres) {
	    DBG_ERROR(LC_LOGDOMAIN, "Error in user interaction (%d)", mres);
	    LC_Client_Stop(lct->client);
	    return GWEN_ERROR_USER_ABORTED;
	  }
	} /* if there is no other card waiting */
	else {
          /* otherwise there already is another card in another reader,
           * so no need to bother the user. This allows to insert all
           * cards in all readers and let me choose the card ;-) */
        } /* if there is another card waiting */
      } /* if card open */
    } /* if there is a card */

    first=0;
  } /* for */

  /* ok, now we have the card we wanted to have, now ask for the pin */
  LC_Client_Stop(lct->client);

  lct->card=hcard;
  return 0;
}



int LC_Crypt_TokenDDV__EnsurePin(GWEN_CRYPT_TOKEN *ct, uint32_t guiid) {
  LC_CT_DDV *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  while(!lct->havePin) {
    int rv;

    /* enter pin */
    rv=LC_Crypt_Token_VerifyPin(ct, lct->card, GWEN_Crypt_PinType_Access, guiid);
    if (rv) {
      DBG_ERROR(LC_LOGDOMAIN, "Error in PIN input");
      return rv;
    }
    else
      lct->havePin=1;
  } /* while !havepin */

  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_Open(GWEN_CRYPT_TOKEN *ct, int manage, uint32_t guiid) {
  LC_CT_DDV *lct;
  int rv;
  int i;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  /* reset pin status */
  lct->havePin=0;

  /* reset key info */
  for (i=0; i<LC_CT_DDV_NUM_KEY; i++) {
    GWEN_Crypt_Token_KeyInfo_free(lct->keyInfos[i]);
    lct->keyInfos[i]=NULL;
  }

  /* reset context info */
  for (i=0; i<LC_CT_DDV_NUM_CONTEXT; i++) {
    GWEN_Crypt_Token_Context_free(lct->contexts[i]);
    lct->contexts[i]=NULL;
  }

  /* get card */
  rv=LC_Crypt_TokenDDV__GetCard(ct, guiid);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_Close(GWEN_CRYPT_TOKEN *ct,
			int abandon,
			uint32_t guiid) {
  LC_CT_DDV *lct;
  LC_CLIENT_RESULT res;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  res=LC_Card_Close(lct->card);
  if (res!=LC_Client_ResultOk) {
    LC_Client_ReleaseCard(lct->client, lct->card);
    LC_Card_free(lct->card);
    lct->card=0;
    return GWEN_ERROR_IO;
  }

  res=LC_Client_ReleaseCard(lct->client, lct->card);
  LC_Card_free(lct->card);
  lct->card=0;
  if (res!=LC_Client_ResultOk)
    return GWEN_ERROR_IO;

  return 0;
}



int LC_Crypt_TokenDDV__ReadSignSeq(GWEN_CRYPT_TOKEN *ct,
				   uint32_t kid,
				   uint32_t *pSigCounter) {
  LC_CT_DDV *lct;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *dbRecord;
  int seq;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  /* get keyinfo and perform some checks */
  if (kid!=1) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }

  /* read signature sequence counter from card */
  res=LC_Card_SelectEf(lct->card, "EF_SEQ");
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "here");
    return GWEN_ERROR_IO;
  }

  mbuf=GWEN_Buffer_new(0, 4, 0, 1);
  res=LC_Card_IsoReadRecord(lct->card,
                            LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_IO;
  }
  GWEN_Buffer_Rewind(mbuf);
  dbRecord=GWEN_DB_Group_new("seq");
  if (LC_Card_ParseRecord(lct->card, 1, mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing record");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_IO;
  }

  seq=GWEN_DB_GetIntValue(dbRecord, "seq", 0, -1);
  if (seq==-1) {
    DBG_ERROR(LC_LOGDOMAIN, "Bad record data in EF_SEQ");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_IO;
  }

  *pSigCounter=seq;

  GWEN_DB_Group_free(dbRecord);
  GWEN_Buffer_free(mbuf);

  return 0;
}



int LC_Crypt_TokenDDV__WriteSignSeq(GWEN_CRYPT_TOKEN *ct,
				    uint32_t kid,
				    uint32_t sigCounter) {
  LC_CT_DDV *lct;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *dbRecord;
  int seq;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  /* get keyinfo and perform some checks */
  if (kid!=1) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }

  /* read signature sequence counter from card */
  res=LC_Card_SelectEf(lct->card, "EF_SEQ");
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "here");
    return GWEN_ERROR_IO;
  }

  mbuf=GWEN_Buffer_new(0, 4, 0, 1);
  res=LC_Card_IsoReadRecord(lct->card,
                            LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_IO;
  }
  GWEN_Buffer_Rewind(mbuf);
  dbRecord=GWEN_DB_Group_new("seq");
  if (LC_Card_ParseRecord(lct->card, 1, mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing record");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_IO;
  }

  seq=GWEN_DB_GetIntValue(dbRecord, "seq", 0, -1);
  if (seq==-1) {
    DBG_ERROR(LC_LOGDOMAIN, "Bad record data in EF_SEQ");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_IO;
  }

  GWEN_DB_SetIntValue(dbRecord, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "seq", sigCounter);
  GWEN_Buffer_Reset(mbuf);
  if (LC_Card_CreateRecord(lct->card, 1, mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error creating record");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_IO;
  }
  GWEN_Buffer_Rewind(mbuf);
  res=LC_Card_IsoUpdateRecord(lct->card,
                              LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                              1,
                              GWEN_Buffer_GetStart(mbuf),
                              GWEN_Buffer_GetUsedBytes(mbuf));

  GWEN_DB_Group_free(dbRecord);
  GWEN_Buffer_free(mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return GWEN_ERROR_IO;
  }

  return 0;
}



int LC_Crypt_TokenDDV__IncSignSeq(GWEN_CRYPT_TOKEN *ct,
				  uint32_t kid,
				  uint32_t *pSigCounter) {
  int rv;
  uint32_t sc;

  rv=LC_Crypt_TokenDDV__ReadSignSeq(ct, kid, &sc);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  sc++;
  sc&=0xffff;
  *pSigCounter=sc;

  rv=LC_Crypt_TokenDDV__WriteSignSeq(ct, kid, sc);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_GetKeyIdList(GWEN_CRYPT_TOKEN *ct,
			       uint32_t *pIdList,
			       uint32_t *pCount,
			       uint32_t gid) {
  assert(pCount);

  if (pIdList) {
    int i;

    if (*pCount<LC_CT_DDV_NUM_KEY)
      return GWEN_ERROR_BUFFER_OVERFLOW;
    for (i=0; i<LC_CT_DDV_NUM_KEY; i++)
      pIdList[i]=i+1;
  }
  *pCount=LC_CT_DDV_NUM_KEY;

  return 0;
}



const GWEN_CRYPT_TOKEN_KEYINFO* GWENHYWFAR_CB
LC_Crypt_TokenDDV_GetKeyInfo(GWEN_CRYPT_TOKEN *ct,
			     uint32_t id,
			     uint32_t flags,
			     uint32_t gid) {
  LC_CT_DDV *lct;
  uint32_t seq;
  int rv;
  GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return NULL;
  }

  if (id<1 || id>LC_CT_DDV_NUM_KEY)
    return NULL;

  ki=lct->keyInfos[id-1];
  if (ki==NULL) {
    int i;

    /* read key info from card */
    ki=GWEN_Crypt_Token_KeyInfo_new(id, GWEN_Crypt_CryptAlgoId_Des3K, 2);
    if (id==1) {
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, "Sign Key");
      i=LC_DDVCard_GetSignKeyNumber(lct->card);
      if (i>=0)
	GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, i);
      i=LC_DDVCard_GetSignKeyVersion(lct->card);
      if (i>=0)
	GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, i);
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
                                        GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
                                        GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
    }
    else {
      int i;

      i=LC_DDVCard_GetCryptKeyNumber(lct->card);
      if (i>=0)
	GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, i);
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, "Crypt Key");
      i=LC_DDVCard_GetCryptKeyVersion(lct->card);
      if (i>=0)
	GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, i);
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANDECIPHER);
    }

    lct->keyInfos[id-1]=ki;
  }

  if (id==1) {
    /* read signature counter for key 1 in any case */
    rv=LC_Crypt_TokenDDV__ReadSignSeq(ct, id, &seq);
    if (rv) {
      DBG_WARN(LC_LOGDOMAIN, "Could not read sign counter (%d)", rv);
    }
    else {
      GWEN_Crypt_Token_KeyInfo_SetSignCounter(ki, seq);
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki, GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER);
    }
  }

  return lct->keyInfos[id-1];
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_SetKeyInfo(GWEN_CRYPT_TOKEN *ct,
			     uint32_t id,
			     const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			     uint32_t gid) {
  LC_CT_DDV *lct;
  uint32_t fl;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  fl=GWEN_Crypt_Token_KeyInfo_GetFlags(ki);

  if (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER) {
    int rv;
    uint32_t seq;

    /* ensure the pin is verified */
    rv=LC_Crypt_TokenDDV__EnsurePin(ct, gid);
    if (rv<0) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* set sign seq */
    seq=GWEN_Crypt_Token_KeyInfo_GetSignCounter(ki);
    rv=LC_Crypt_TokenDDV__WriteSignSeq(ct, id, seq);
    if (rv<0) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_GetContextIdList(GWEN_CRYPT_TOKEN *ct,
				   uint32_t *pIdList,
				   uint32_t *pCount,
				   uint32_t gid) {
  assert(pCount);

  if (pIdList) {
    int i;

    if (*pCount<LC_CT_DDV_NUM_CONTEXT)
      return GWEN_ERROR_BUFFER_OVERFLOW;
    for (i=0; i<LC_CT_DDV_NUM_CONTEXT; i++)
      pIdList[i]=i+1;
  }
  *pCount=LC_CT_DDV_NUM_CONTEXT;

  return 0;
}



const GWEN_CRYPT_TOKEN_CONTEXT* GWENHYWFAR_CB
LC_Crypt_TokenDDV_GetContext(GWEN_CRYPT_TOKEN *ct,
			     uint32_t id,
			     uint32_t gid) {
  LC_CT_DDV *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (id<1 || id>LC_CT_DDV_NUM_CONTEXT)
    return NULL;

  if (lct->contexts[id-1]==NULL) {
    LC_CLIENT_RESULT res;
    GWEN_DB_NODE *dbData;
    GWEN_DB_NODE *dbCtx;
    GWEN_CRYPT_TOKEN_CONTEXT *ctx;
    GWEN_BUFFER *cbuf;

    dbData=GWEN_DB_Group_new("institute");
    res=LC_DDVCard_ReadInstituteData(lct->card, id, dbData);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "No context available");
      GWEN_DB_Group_free(dbData);
      return NULL;
    }

    /* read context info from card */
    ctx=GWEN_Crypt_Token_Context_new();
    GWEN_Crypt_Token_Context_SetId(ctx, id);
    dbCtx=GWEN_DB_FindFirstGroup(dbData, "context");
    if (dbCtx) {
      const char *s;
      int j;

      s=GWEN_DB_GetCharValue(dbCtx, "userId", 0, 0);
      if (s)
	GWEN_Crypt_Token_Context_SetUserId(ctx, s);
      s=GWEN_DB_GetCharValue(dbCtx, "bankName", 0, 0);
      if (s)
	GWEN_Crypt_Token_Context_SetPeerName(ctx, s);
      s=GWEN_DB_GetCharValue(dbCtx, "bankCode", 0, 0);
      if (s)
	GWEN_Crypt_Token_Context_SetServiceId(ctx, s);
      s=GWEN_DB_GetCharValue(dbCtx, "comAddress", 0, 0);
      if (s)
	GWEN_Crypt_Token_Context_SetAddress(ctx, s);
      j=GWEN_DB_GetIntValue(dbCtx, "comService", 0, 2);
      switch(j) {
      case 0:
      case 1:  /* BTX */
	break;
      case 2:  /* "real" HBCI */
	GWEN_Crypt_Token_Context_SetPort(ctx, 3000);
	break;
      case 3:  /* PIN/TAN (HTTPS) */
	GWEN_Crypt_Token_Context_SetPort(ctx, 443);
	break;
      default:
	break;
      }

      /* set key ids (same for every context) */
      GWEN_Crypt_Token_Context_SetSignKeyId(ctx, 1);
      GWEN_Crypt_Token_Context_SetVerifyKeyId(ctx, 1);
      GWEN_Crypt_Token_Context_SetEncipherKeyId(ctx, 2);
      GWEN_Crypt_Token_Context_SetDecipherKeyId(ctx, 2);
    }
    GWEN_DB_Group_free(dbData);

    /* create system id (same for every context) */
    cbuf=LC_DDVCard_GetCardDataAsBuffer(lct->card);
    if (cbuf==0) {
      DBG_ERROR(LC_LOGDOMAIN, "No card data");
    }
    else {
      GWEN_BUFFER *tbuf;
      int rv;

      tbuf=GWEN_Buffer_new(0, 2*GWEN_Buffer_GetUsedBytes(cbuf), 0, 1);
      rv=GWEN_Text_ToHexBuffer(GWEN_Buffer_GetStart(cbuf),
			       GWEN_Buffer_GetUsedBytes(cbuf),
                               tbuf,
			       0, 0, 0);
      if (rv) {
	DBG_ERROR(LC_LOGDOMAIN, "Error converting card id to hex (%d)", rv);
      }
      else
	GWEN_Crypt_Token_Context_SetSystemId(ctx, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }

    /* store new context */
    lct->contexts[id-1]=ctx;
  }

  return lct->contexts[id-1];
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_SetContext(GWEN_CRYPT_TOKEN *ct,
			     uint32_t id,
			     const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
			     uint32_t gid) {
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_Sign(GWEN_CRYPT_TOKEN *ct,
		       uint32_t keyId,
		       GWEN_CRYPT_PADDALGO *a,
		       const uint8_t *pInData,
		       uint32_t inLen,
		       uint8_t *pSignatureData,
		       uint32_t *pSignatureLen,
		       uint32_t *pSeqCounter,
		       uint32_t gid) {
  LC_CT_DDV *lct;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *srcBuf;
  GWEN_BUFFER *dstBuf;
  uint32_t seq;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (GWEN_Crypt_PaddAlgo_GetId(a)!=GWEN_Crypt_PaddAlgoId_None) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid padd algo");
    return GWEN_ERROR_INVALID;
  }

  if (keyId!=1) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }

  if (inLen!=20) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid hash size");
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenDDV__EnsurePin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    return rv;
  }

  /* sign the data */
  srcBuf=GWEN_Buffer_new(0, 20, 0, 1);
  GWEN_Buffer_AppendBytes(srcBuf, (const char*)pInData, inLen);
  GWEN_Buffer_Rewind(srcBuf);
  dstBuf=GWEN_Buffer_new(0, 8, 0, 1);
  res=LC_DDVCard_SignHash(lct->card, srcBuf, dstBuf);
  GWEN_Buffer_free(srcBuf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error signing hash (%d)", res);
    GWEN_Buffer_free(dstBuf);
    return GWEN_ERROR_IO;
  }

  /* always increment signature counter */
  rv=LC_Crypt_TokenDDV__IncSignSeq(ct, keyId, &seq);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dstBuf);
    return rv;
  }
  if (pSeqCounter)
    *pSeqCounter=seq;

  /* copy signature */
  memmove(pSignatureData, GWEN_Buffer_GetStart(dstBuf), 8);
  GWEN_Buffer_free(dstBuf);
  *pSignatureLen=8;

  /* done */
  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_Verify(GWEN_CRYPT_TOKEN *ct,
			 uint32_t keyId,
			 GWEN_CRYPT_PADDALGO *a,
			 const uint8_t *pInData,
			 uint32_t inLen,
			 const uint8_t *pSignatureData,
			 uint32_t signatureLen,
			 uint32_t seqCounter,
			 uint32_t gid) {
  LC_CT_DDV *lct;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *srcBuf;
  GWEN_BUFFER *dstBuf;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (GWEN_Crypt_PaddAlgo_GetId(a)!=GWEN_Crypt_PaddAlgoId_None) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid padd algo");
    return GWEN_ERROR_INVALID;
  }

  if (keyId!=1) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }

  if (inLen!=20) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid hash size");
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenDDV__EnsurePin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    return rv;
  }

  /* sign the data */
  srcBuf=GWEN_Buffer_new(0, 20, 0, 1);
  GWEN_Buffer_AppendBytes(srcBuf, (const char*)pInData, inLen);
  GWEN_Buffer_Rewind(srcBuf);
  dstBuf=GWEN_Buffer_new(0, 8, 0, 1);
  res=LC_DDVCard_SignHash(lct->card, srcBuf, dstBuf);
  GWEN_Buffer_free(srcBuf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error signing hash (%d)", res);
    GWEN_Buffer_free(dstBuf);
    return GWEN_ERROR_IO;
  }

  /* compare signatures */
  if (memcmp(pSignatureData, GWEN_Buffer_GetStart(dstBuf), 8)) {
    DBG_ERROR(LC_LOGDOMAIN, "Signatures do not match");
    GWEN_Buffer_free(dstBuf);
    return GWEN_ERROR_VERIFY;
  }
  GWEN_Buffer_free(dstBuf);

  /* done */
  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_Encipher(GWEN_CRYPT_TOKEN *ct,
			   uint32_t keyId,
			   GWEN_CRYPT_PADDALGO *a,
			   const uint8_t *pInData,
			   uint32_t inLen,
			   uint8_t *pOutData,
			   uint32_t *pOutLen,
			   uint32_t gid) {
  LC_CT_DDV *lct;
  GWEN_BUFFER *srcBuf;
  GWEN_BUFFER *dstBuf;
  unsigned int i;
  const char *p;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (keyId!=2) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }

  srcBuf=GWEN_Buffer_new(0, inLen+128, 0, 1);
  GWEN_Buffer_AppendBytes(srcBuf, (const char*)pInData, inLen);

  /* apply padding algo */
  rv=GWEN_Padd_ApplyPaddAlgo(a, srcBuf);
  if (rv<0) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(srcBuf);
    return rv;
  }

  if (GWEN_Buffer_GetUsedBytes(srcBuf) % 8) {
    DBG_ERROR(LC_LOGDOMAIN,
	      "Data size (%d) is not multiple of 8 after padding",
	      GWEN_Buffer_GetUsedBytes(srcBuf));
    GWEN_Buffer_free(srcBuf);
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenDDV__EnsurePin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    GWEN_Buffer_free(srcBuf);
    return rv;
  }

  /* encrypt */
  /* TODO: don't create and copy new dstBuf, instead let GWEN_BUFFER use
   * the given buffer in non-dynamic mode */
  dstBuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(srcBuf), 0, 1);
  p=GWEN_Buffer_GetStart(srcBuf);
  i=GWEN_Buffer_GetUsedBytes(srcBuf)/8;
  while(i--) {
    LC_CLIENT_RESULT res;

    res=LC_DDVCard_CryptCharBlock(lct->card, p,
				  8,
				  dstBuf);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "Error encrypting hash (%d)", res);
      GWEN_Buffer_free(dstBuf);
      GWEN_Buffer_free(srcBuf);
      return GWEN_ERROR_IO;
    }
    p+=8;
  }

  if (GWEN_Buffer_GetUsedBytes(dstBuf)>*pOutLen) {
    DBG_ERROR(LC_LOGDOMAIN, "Buffer too small");
    GWEN_Buffer_free(dstBuf);
    GWEN_Buffer_free(srcBuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  /* copy to given buffer */
  memmove(pOutData,
	  GWEN_Buffer_GetStart(dstBuf),
	  GWEN_Buffer_GetUsedBytes(dstBuf));
  *pOutLen=GWEN_Buffer_GetUsedBytes(dstBuf);

  GWEN_Buffer_free(dstBuf);
  GWEN_Buffer_free(srcBuf);

  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenDDV_Decipher(GWEN_CRYPT_TOKEN *ct,
			   uint32_t keyId,
			   GWEN_CRYPT_PADDALGO *a,
			   const uint8_t *pInData,
			   uint32_t inLen,
			   uint8_t *pOutData,
			   uint32_t *pOutLen,
			   uint32_t gid) {
  LC_CT_DDV *lct;
  GWEN_BUFFER *dstBuf;
  unsigned int i;
  const char *p;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_DDV, ct);
  assert(lct);

  if (keyId!=2) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }

  if (inLen % 8) {
    DBG_ERROR(LC_LOGDOMAIN,
	      "Data size (%d) is not multiple of 8 after padding",
	      inLen);
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenDDV__EnsurePin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    return rv;
  }

  /* decrypt */
  /* TODO: don't create and copy new dstBuf, instead let GWEN_BUFFER use
   * the given buffer in non-dynamic mode */
  dstBuf=GWEN_Buffer_new(0, inLen, 0, 1);
  p=(const char*)pInData;
  i=inLen/8;
  while(i--) {
    LC_CLIENT_RESULT res;

    res=LC_DDVCard_CryptCharBlock(lct->card, p, 8, dstBuf);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "Error encrypting hash (%d)", res);
      GWEN_Buffer_free(dstBuf);
      return GWEN_ERROR_IO;
    }
    p+=8;
  }

  if (GWEN_Buffer_GetUsedBytes(dstBuf)>*pOutLen) {
    DBG_ERROR(LC_LOGDOMAIN, "Buffer too small");
    GWEN_Buffer_free(dstBuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  /* unapply padding algo */
  rv=GWEN_Padd_UnapplyPaddAlgo(a, dstBuf);
  if (rv<0) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dstBuf);
    return rv;
  }


  /* copy to given buffer */
  memmove(pOutData,
	  GWEN_Buffer_GetStart(dstBuf),
	  GWEN_Buffer_GetUsedBytes(dstBuf));
  *pOutLen=GWEN_Buffer_GetUsedBytes(dstBuf);

  GWEN_Buffer_free(dstBuf);

  return 0;
}









