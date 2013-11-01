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

#include "starcoscard_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/ctplugin_be.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>

#include <chipcard/cards/starcos.h>
#include <chipcard/cards/processorcard.h>
#include <chipcard/ct/ct_card.h>


#define I18N(message) GWEN_I18N_Translate("libchipcard", message)



GWEN_INHERIT(GWEN_CRYPT_TOKEN, LC_CT_STARCOS)
GWEN_INHERIT(GWEN_PLUGIN, LC_CT_PLUGIN_STARCOS)



GWEN_PLUGIN *ct_starcoscard_factory(GWEN_PLUGIN_MANAGER *pm,
				    const char *modName,
				    const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=LC_Crypt_TokenStarcos_Plugin_new(pm, modName, fileName);
  if (pl==NULL) {
    DBG_ERROR(LC_LOGDOMAIN, "No plugin created");
    return NULL;
  }
  return pl;
}



GWEN_PLUGIN *LC_Crypt_TokenStarcos_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					      const char *modName,
					      const char *fileName) {
  GWEN_PLUGIN *pl;
  LC_CT_PLUGIN_STARCOS *cpl;
  LC_CLIENT_RESULT res;

  pl=GWEN_Crypt_Token_Plugin_new(pm,
				 GWEN_Crypt_Token_Device_Card,
				 modName,
				 fileName);

  GWEN_NEW_OBJECT(LC_CT_PLUGIN_STARCOS, cpl);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, LC_CT_PLUGIN_STARCOS, pl, cpl,
		       LC_Crypt_TokenStarcos_Plugin_FreeData);
  cpl->client=LC_Client_new("LC_Crypt_TokenStarcos", VERSION);
  res=LC_Client_Init(cpl->client);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN,
	      "Error initialising libchipcard (%d), chipcards will not be available",
	      res);
    GWEN_Plugin_free(pl);
    return NULL;
  }

  /* set virtual functions */
  GWEN_Crypt_Token_Plugin_SetCreateTokenFn(pl, LC_Crypt_TokenStarcos_Plugin_CreateToken);
  GWEN_Crypt_Token_Plugin_SetCheckTokenFn(pl, LC_Crypt_TokenStarcos_Plugin_CheckToken);
  return pl;
}



void GWENHYWFAR_CB LC_Crypt_TokenStarcos_Plugin_FreeData(void *bp, void *p) {
  LC_CT_PLUGIN_STARCOS *cpl;

  cpl=(LC_CT_PLUGIN_STARCOS*)p;
  LC_Client_free(cpl->client);
  GWEN_FREE_OBJECT(cpl);
}



GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Plugin_CreateToken(GWEN_PLUGIN *pl,
					 const char *name) {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_CRYPT_TOKEN *ct;
  LC_CT_PLUGIN_STARCOS *cpl;

  assert(pl);
  cpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, LC_CT_PLUGIN_STARCOS, pl);
  assert(cpl);

  pm=GWEN_Plugin_GetManager(pl);
  assert(pm);

  ct=LC_Crypt_TokenStarcos_new(pm, cpl->client, name);
  assert(ct);

  return ct;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Plugin_CheckToken(GWEN_PLUGIN *pl,
					GWEN_BUFFER *name) {
  GWEN_PLUGIN_MANAGER *pm;
  LC_CT_PLUGIN_STARCOS *cpl;
  LC_CLIENT_RESULT res;
  LC_CARD *hcard=0;
  const char *currCardNumber;
  int i;

  assert(pl);
  cpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, LC_CT_PLUGIN_STARCOS, pl);
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
    rv=LC_Starcos_ExtendCard(hcard);
    if (rv) {
      DBG_ERROR(LC_LOGDOMAIN,
		"Starcos card not available, please check your setup (%d)", rv);
      LC_Client_ReleaseCard(cpl->client, hcard);
      LC_Card_free(hcard);
      return GWEN_ERROR_NOT_AVAILABLE;
    }

    res=LC_Card_Open(hcard);
    if (res!=LC_Client_ResultOk) {
      LC_Client_ReleaseCard(cpl->client, hcard);
      LC_Card_free(hcard);
      DBG_NOTICE(LC_LOGDOMAIN,
		 "Could not open card (%d), maybe not a Starcos card?",
		 res);
      return GWEN_ERROR_NOT_SUPPORTED;
    } /* if card not open */
    else {
      GWEN_DB_NODE *dbCardData;

        dbCardData=LC_Starcos_GetCardDataAsDb(hcard);
	assert(dbCardData);

        currCardNumber=GWEN_DB_GetCharValue(dbCardData,
					    "ICCSN/cardNumber",
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



GWEN_CRYPT_TOKEN *LC_Crypt_TokenStarcos_new(GWEN_PLUGIN_MANAGER *pm,
					    LC_CLIENT *lc,
					    const char *name) {
  LC_CT_STARCOS *lct;
  GWEN_CRYPT_TOKEN *ct;

  DBG_INFO(LC_LOGDOMAIN, "Creating crypttoken (Starcos)");

  /* create crypt token */
  ct=GWEN_Crypt_Token_new(GWEN_Crypt_Token_Device_Card,
			  "starcoscard", name);

  /* inherit CryptToken: Set our own data */
  GWEN_NEW_OBJECT(LC_CT_STARCOS, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct, lct,
                       LC_Crypt_TokenStarcos_FreeData);
  lct->pluginManager=pm;
  lct->client=lc;

  /* set virtual functions */
  GWEN_Crypt_Token_SetOpenFn(ct, LC_Crypt_TokenStarcos_Open);
  GWEN_Crypt_Token_SetCloseFn(ct, LC_Crypt_TokenStarcos_Close);

  GWEN_Crypt_Token_SetGetKeyIdListFn(ct, LC_Crypt_TokenStarcos_GetKeyIdList);
  GWEN_Crypt_Token_SetGetKeyInfoFn(ct, LC_Crypt_TokenStarcos_GetKeyInfo);
  GWEN_Crypt_Token_SetSetKeyInfoFn(ct, LC_Crypt_TokenStarcos_SetKeyInfo);
  GWEN_Crypt_Token_SetGetContextIdListFn(ct, LC_Crypt_TokenStarcos_GetContextIdList);
  GWEN_Crypt_Token_SetGetContextFn(ct, LC_Crypt_TokenStarcos_GetContext);
  GWEN_Crypt_Token_SetSetContextFn(ct, LC_Crypt_TokenStarcos_SetContext);

  GWEN_Crypt_Token_SetSignFn(ct, LC_Crypt_TokenStarcos_Sign);
  GWEN_Crypt_Token_SetVerifyFn(ct, LC_Crypt_TokenStarcos_Verify);
  GWEN_Crypt_Token_SetEncipherFn(ct, LC_Crypt_TokenStarcos_Encipher);
  GWEN_Crypt_Token_SetDecipherFn(ct, LC_Crypt_TokenStarcos_Decipher);

  GWEN_Crypt_Token_SetGenerateKeyFn(ct, LC_Crypt_TokenStarcos_GenerateKey);

  return ct;
}



void GWENHYWFAR_CB LC_Crypt_TokenStarcos_FreeData(void *bp, void *p) {
  LC_CT_STARCOS *lct;

  lct=(LC_CT_STARCOS*)p;
  if (lct->card) {
    LC_Client_ReleaseCard(lct->client, lct->card);
    LC_Card_free(lct->card);
  }
  GWEN_FREE_OBJECT(lct);
}



int LC_Crypt_TokenStarcos__GetCard(GWEN_CRYPT_TOKEN *ct, uint32_t guiid) {
  LC_CT_STARCOS *lct;
  LC_CLIENT_RESULT res;
  LC_CARD *hcard=0;
  int first;
  const char *currCardNumber;
  const char *name;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
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
      rv=LC_Starcos_ExtendCard(hcard);
      if (rv) {
        DBG_ERROR(LC_LOGDOMAIN,
                  "Starcos card not available, please check your setup (%d)", rv);
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
                   "Could not open card (%d), maybe not a Starcos card?",
                   res);
      } /* if card not open */
      else {
        GWEN_DB_NODE *dbCardData;

        dbCardData=LC_Starcos_GetCardDataAsDb(hcard);
	assert(dbCardData);

        currCardNumber=GWEN_DB_GetCharValue(dbCardData,
                                            "ICCSN/cardNumber",
                                            0,
                                            0);
	if (!currCardNumber) {
	  DBG_ERROR(LC_LOGDOMAIN, "INTERNAL: No card number in card data.");
          GWEN_DB_Dump(dbCardData, 2);
          abort();
        }

        DBG_NOTICE(LC_LOGDOMAIN, "Card number: %s", currCardNumber);

        if (!name || !*name) {
          GWEN_Crypt_Token_SetTokenName(ct, currCardNumber);
          name=GWEN_Crypt_Token_GetTokenName(ct);
	  DBG_NOTICE(LC_LOGDOMAIN, "No or empty token name, using [%s]",
		     name?name:"<empty>");
          break;
        }

        if (strcasecmp(name, currCardNumber)==0) {
          DBG_NOTICE(LC_LOGDOMAIN, "Card number equals");
          break;
	}
	else {
          DBG_INFO(LC_LOGDOMAIN, "Card number not equal, looking for next card");
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



int LC_Crypt_TokenStarcos__EnsureAccessPin(GWEN_CRYPT_TOKEN *ct, uint32_t guiid) {
  LC_CT_STARCOS *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  while(!lct->haveAccessPin) {
    int rv;

    /* enter pin */
    rv=LC_Crypt_Token_VerifyPin(ct, lct->card,
				GWEN_Crypt_PinType_Access,
				guiid);
    if (rv) {
      DBG_ERROR(LC_LOGDOMAIN, "Error in PIN input");
      return rv;
    }
    else
      lct->haveAccessPin=1;
  } /* while !havepin */

  return 0;
}



int LC_Crypt_TokenStarcos__EnsureAdminPin(GWEN_CRYPT_TOKEN *ct, uint32_t guiid) {
  LC_CT_STARCOS *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  while(!lct->haveAdminPin) {
    int rv;

    /* enter pin */
    rv=LC_Crypt_Token_VerifyPin(ct, lct->card,
				GWEN_Crypt_PinType_Manage,
				guiid);
    if (rv) {
      DBG_ERROR(LC_LOGDOMAIN, "Error in PIN input");
      return rv;
    }
    else
      lct->haveAdminPin=1;
  } /* while !havepin */

  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Open(GWEN_CRYPT_TOKEN *ct, int manage, uint32_t guiid) {
  LC_CT_STARCOS *lct;
  int rv;
  int i;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  /* reset pin status */
  lct->haveAccessPin=0;
  lct->haveAdminPin=0;

  /* reset key info */
  for (i=0; i<LC_CT_STARCOS_NUM_KEY; i++) {
    GWEN_Crypt_Token_KeyInfo_free(lct->keyInfos[i]);
    lct->keyInfos[i]=NULL;
  }

  /* reset context info */
  for (i=0; i<LC_CT_STARCOS_NUM_CONTEXT; i++) {
    GWEN_Crypt_Token_Context_free(lct->contexts[i]);
    lct->contexts[i]=NULL;
  }

  /* get card */
  rv=LC_Crypt_TokenStarcos__GetCard(ct, guiid);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Close(GWEN_CRYPT_TOKEN *ct,
			    int abandon,
			    uint32_t guiid) {
  LC_CT_STARCOS *lct;
  LC_CLIENT_RESULT res;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
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



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_GetKeyIdList(GWEN_CRYPT_TOKEN *ct,
				   uint32_t *pIdList,
				   uint32_t *pCount,
				   uint32_t gid) {
  assert(pCount);

  if (pIdList) {
    int i;

    if (*pCount<20)
      return GWEN_ERROR_BUFFER_OVERFLOW;
    for (i=0; i<10; i++)
      pIdList[i]=i+0x81;
    for (i=0; i<10; i++)
      pIdList[i+10]=i+0x91;
  }
  *pCount=20;

  return 0;
}



int LC_Crypt_TokenStarcos__ReadKeyInfo(GWEN_CRYPT_TOKEN *ct,
				       uint32_t kid,
				       uint32_t gid) {
  LC_CT_STARCOS *lct;
  GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  int idx;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  idx=0;
  if (kid>0x90)
    idx+=10;
  idx+=(kid & 0xf)-1;

  if (idx<0 || idx>=LC_CT_STARCOS_NUM_KEY) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02x (idx=%d)", kid, idx);
    return GWEN_ERROR_INVALID;
  }

  ki=lct->keyInfos[idx];
  if (ki==NULL) {
    int i;
    int rv;
    LC_STARCOS_KEYDESCR *kdescr;
    LC_CLIENT_RESULT res;
    GWEN_BUFFER *bModulus;
    GWEN_BUFFER *bExponent;

    /* ensure the access pin is verified */
    rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
    if (rv<0) {
      DBG_ERROR(LC_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* read key descriptor */
    res=LC_Starcos_GetKeyDescr(lct->card, kid, &kdescr);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "here (%d)", res);
      return GWEN_ERROR_IO;
    }

    /* read public key */
    bModulus=GWEN_Buffer_new(0, 256, 0, 1);
    bExponent=GWEN_Buffer_new(0, 256, 0, 1);
    res=LC_Starcos_ReadPublicKey(lct->card, kid, bModulus, bExponent);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "No public key (%d)", res);
      GWEN_Buffer_free(bModulus);
      bModulus=NULL;
      GWEN_Buffer_free(bExponent);
      bExponent=NULL;
    }

    /* read key info from card */
    ki=GWEN_Crypt_Token_KeyInfo_new(kid, GWEN_Crypt_CryptAlgoId_Rsa, 96);
    if (kid>=0x81 && kid<=0x85) {
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("User Sign Key"));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
    }
    else if (kid>=0x86 && kid<=0x8a) {
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("User Crypt Key"));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANDECIPHER);
    }
    else if (kid>=0x91 && kid<=0x95) {
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Peer Sign Key"));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
    }
    else if (kid>=0x96 && kid<=0x9a) {
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Peer Crypt Key"));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER);
    }

    /* set key version and number */
    i=LC_Starcos_KeyDescr_GetKeyNum(kdescr);
    if (i>=0)
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, i);
    i=LC_Starcos_KeyDescr_GetKeyVer(kdescr);
    if (i>=0)
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, i);
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION);

    /* set public key */
    if (bModulus && bExponent) {
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki,
					  (const uint8_t*)GWEN_Buffer_GetStart(bModulus),
					  GWEN_Buffer_GetUsedBytes(bModulus));
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki,
					   (const uint8_t*)GWEN_Buffer_GetStart(bExponent),
					   GWEN_Buffer_GetUsedBytes(bExponent));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT);
      GWEN_Buffer_free(bModulus);
      GWEN_Buffer_free(bExponent);
    }

    /* done */
    lct->keyInfos[idx]=ki;
  }

  if (kid>=0x81 && kid<=0x85) {
    uint32_t seq;
    LC_CLIENT_RESULT res;

    /* read signature counter for user sign keys in any case */
    res=LC_Starcos_ReadSigCounter(lct->card, kid, &seq);
    if (res!=LC_Client_ResultOk) {
      DBG_WARN(LC_LOGDOMAIN, "No signature counter for key 0x%02x (%d)", kid, res);
    }
    else {
      GWEN_Crypt_Token_KeyInfo_SetSignCounter(ki, seq);
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER);
    }
  }

  return 0;
}



const GWEN_CRYPT_TOKEN_KEYINFO* GWENHYWFAR_CB
LC_Crypt_TokenStarcos_GetKeyInfo(GWEN_CRYPT_TOKEN *ct,
				 uint32_t kid,
				 uint32_t flags,
				 uint32_t gid) {
  LC_CT_STARCOS *lct;
  GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  int idx;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return NULL;
  }

  idx=0;
  if (kid>0x90)
    idx+=10;
  idx+=(kid & 0xf)-1;

  if (idx<0 || idx>=LC_CT_STARCOS_NUM_KEY) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02x (idx=%d)", kid, idx);
    return NULL;
  }

  ki=lct->keyInfos[idx];
  if (ki==NULL) {
    int rv;

    rv=LC_Crypt_TokenStarcos__ReadKeyInfo(ct, kid, gid);
    if (rv<0) {
      DBG_ERROR(LC_LOGDOMAIN, "here (%d)", rv);
      return NULL;
    }
  }
  ki=lct->keyInfos[idx];

  if (kid>=0x81 && kid<=0x85) {
    uint32_t seq;
    LC_CLIENT_RESULT res;

    /* read signature counter for user sign keys in any case */
    res=LC_Starcos_ReadSigCounter(lct->card, kid, &seq);
    if (res!=LC_Client_ResultOk) {
      DBG_WARN(LC_LOGDOMAIN, "No signature counter for key 0x%02x (%d)", kid, res);
    }
    else {
      GWEN_Crypt_Token_KeyInfo_SetSignCounter(ki, seq);
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER);
    }
  }

  return ki;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_SetKeyInfo(GWEN_CRYPT_TOKEN *ct,
				 uint32_t kid,
				 const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				 uint32_t gid) {
  LC_CT_STARCOS *lct;
  uint32_t fl;
  GWEN_CRYPT_TOKEN_KEYINFO *cardki;
  int idx;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_NOT_OPEN;
  }

  /* get stored key info */
  idx=0;
  if (kid>0x90)
    idx+=10;
  idx+=(kid & 0xf)-1;

  if (idx<0 || idx>=LC_CT_STARCOS_NUM_KEY) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02x (idx=%d)", kid, idx);
    return GWEN_ERROR_INVALID;
  }

  cardki=lct->keyInfos[idx];
  if (cardki==NULL) {
    rv=LC_Crypt_TokenStarcos__ReadKeyInfo(ct, kid, gid);
    if (rv<0) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  cardki=lct->keyInfos[idx];
  assert(cardki);

  /* ensure the access pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* ensure the admin pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAdminPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  fl=GWEN_Crypt_Token_KeyInfo_GetFlags(ki);

  if (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER) {
    /* do not write sig counter for now */
  }

  /* write descriptor */
  if ((fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION) ||
      (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER) ||
      (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASSTATUS)) {
    LC_CLIENT_RESULT res;
    LC_STARCOS_KEYDESCR *descr;

    DBG_INFO(LC_LOGDOMAIN, "Loading key descriptor");
    res=LC_Starcos_GetKeyDescr(lct->card, kid, &descr);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "here (%d)", res);
      return GWEN_ERROR_IO;
    }

    if (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER) {
      uint32_t i;

      i=GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki);
      LC_Starcos_KeyDescr_SetKeyNum(descr, i);
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(cardki, i);
    }

    if (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION) {
      uint32_t i;

      i=GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki);
      LC_Starcos_KeyDescr_SetKeyVer(descr, i);
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(cardki, i);
    }

#if 0 /* status not yet defined in gwen */
    if (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASSTATUS) {
      GWEN_CRYPT_TOKEN_KEYSTATUS i;
      int nstatus;

      i=GWEN_Crypt_Token_KeyInfo_GetStatus(ki);
      switch(i) {
      case GWEN_Crypt_Token_KeyStatusFree:
	nstatus=LC_STARCOS_KEY_STATUS_INACTIVE_FREE;
	break;
      case GWEN_Crypt_Token_KeyStatusNew:
	nstatus=LC_STARCOS_KEY_STATUS_ACTIVE_NEW;
        break;
      case GWEN_Crypt_Token_KeyStatusActive:
	nstatus=LC_STARCOS_KEY_STATUS_ACTIVE;
        break;
      case GWEN_Crypt_Token_KeyStatusUnknown:
      default:
	nstatus=LC_STARCOS_KEY_STATUS_INTERNAL_UNUSED;
        break;
      }

      if (nstatus!=LC_STARCOS_KEY_STATUS_INTERNAL_UNUSED) {
	LC_Starcos_KeyDescr_SetStatus(descr, nstatus);
	GWEN_Crypt_Token_KeyInfo_SetStatus(cardki, i);
      }
    }
#else
    LC_Starcos_KeyDescr_SetStatus(descr, LC_STARCOS_KEY_STATUS_ACTIVE);
#endif

    if ((kid>=0x86 && kid<=0x8a) ||
	(kid>=0x96 && kid<=0x9a))
      LC_Starcos_KeyDescr_SetKeyType(descr, 'V');
    else
      LC_Starcos_KeyDescr_SetKeyType(descr, 'S');

    /* save descriptor after changing */
    DBG_INFO(LC_LOGDOMAIN, "Saving key descriptor");
    res=LC_Starcos_SaveKeyDescr(lct->card, descr);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "here (%d)", res);
      return GWEN_ERROR_IO;
    }
  }

  /* write public key */
  if ((fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) &&
      (fl & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
    if (kid<0x91 || kid>0x9a) {
      DBG_ERROR(LC_LOGDOMAIN,
		"Cannot change public part on private key");
      return GWEN_ERROR_INVALID;
    }
    else {
      LC_CLIENT_RESULT res;
      const uint8_t *pModulus;
      uint32_t lModulus;
      const uint8_t *pExponent;
      uint32_t lExponent;
  
      pModulus=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
      lModulus=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
      pExponent=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
      lExponent=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
      assert(pModulus && lModulus && pExponent && lExponent);
      DBG_INFO(LC_LOGDOMAIN, "Writing public key");
      res=LC_Starcos_WritePublicKey(lct->card, kid,
				    pModulus, lModulus,
				    pExponent, lExponent);
      if (res!=LC_Client_ResultOk) {
	DBG_ERROR(LC_LOGDOMAIN, "here (%d)", res);
	return GWEN_ERROR_IO;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(cardki, pModulus, lModulus);
      GWEN_Crypt_Token_KeyInfo_SetExponent(cardki, pExponent, lExponent);
      GWEN_Crypt_Token_KeyInfo_AddFlags(cardki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT);
    }
  }

  /* done */
  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_GetContextIdList(GWEN_CRYPT_TOKEN *ct,
				       uint32_t *pIdList,
				       uint32_t *pCount,
				       uint32_t gid) {
  assert(pCount);

  if (pIdList) {
    int i;

    if (*pCount<LC_CT_STARCOS_NUM_CONTEXT)
      return GWEN_ERROR_BUFFER_OVERFLOW;
    for (i=0; i<LC_CT_STARCOS_NUM_CONTEXT; i++)
      pIdList[i]=i+1;
  }
  *pCount=LC_CT_STARCOS_NUM_CONTEXT;

  return 0;
}



const GWEN_CRYPT_TOKEN_CONTEXT* GWENHYWFAR_CB
LC_Crypt_TokenStarcos_GetContext(GWEN_CRYPT_TOKEN *ct,
				 uint32_t id,
				 uint32_t gid) {
  LC_CT_STARCOS *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (id<1 || id>LC_CT_STARCOS_NUM_CONTEXT) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid id (%d)", id);
    return NULL;
  }

  if (lct->contexts[id-1]==NULL) {
    LC_CLIENT_RESULT res;
    GWEN_DB_NODE *dbData;
    GWEN_CRYPT_TOKEN_CONTEXT *ctx;
    int rv;
    const char *bankCode;
    const char *address;

    /* ensure the access pin is verified */
    rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
    if (rv<0) {
      DBG_ERROR(LC_LOGDOMAIN, "here (%d)", rv);
      return NULL;
    }

    dbData=GWEN_DB_Group_new("institute");
    res=LC_Starcos_ReadInstituteData(lct->card, id, dbData);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "No context available");
      GWEN_DB_Group_free(dbData);
      return NULL;
    }

    /* read context info from card */
    ctx=GWEN_Crypt_Token_Context_new();
    GWEN_Crypt_Token_Context_SetId(ctx, id);

    bankCode=GWEN_DB_GetCharValue(dbData, "bankCode", 0, NULL);
    address=GWEN_DB_GetCharValue(dbData, "comAddress", 0, 0);
    if (bankCode && *bankCode && address && *address) {
      const char *s;
      int j;

      GWEN_Crypt_Token_Context_SetServiceId(ctx, bankCode);
      GWEN_Crypt_Token_Context_SetAddress(ctx, address);

      s=GWEN_DB_GetCharValue(dbData, "userId", 0, 0);
      if (s)
	GWEN_Crypt_Token_Context_SetUserId(ctx, s);
      s=GWEN_DB_GetCharValue(dbData, "bankId", 0, 0);
      if (s)
	GWEN_Crypt_Token_Context_SetPeerId(ctx, s);
      s=GWEN_DB_GetCharValue(dbData, "systemId", 0, 0);
      if (s)
	GWEN_Crypt_Token_Context_SetSystemId(ctx, s);
      j=GWEN_DB_GetIntValue(dbData, "comService", 0, 2);
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
    }
    else {
      DBG_INFO(LC_LOGDOMAIN, "Empty entry (%d)", id);
    }
    GWEN_DB_Group_free(dbData);

    /* set key ids (same for every context) */
    GWEN_Crypt_Token_Context_SetSignKeyId(ctx, 0x80+id);     /* user sign key */
    GWEN_Crypt_Token_Context_SetVerifyKeyId(ctx, 0x90+id);   /* peer sign key */
    GWEN_Crypt_Token_Context_SetEncipherKeyId(ctx, 0x95+id); /* peer crypt key */
    GWEN_Crypt_Token_Context_SetDecipherKeyId(ctx, 0x85+id); /* user crypt key */

    /* store new context */
    lct->contexts[id-1]=ctx;
  }

  return lct->contexts[id-1];
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_SetContext(GWEN_CRYPT_TOKEN *ct,
			     uint32_t id,
			     const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
			     uint32_t gid) {
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_GenerateKey(GWEN_CRYPT_TOKEN *ct,
				  uint32_t kid,
				  const GWEN_CRYPT_CRYPTALGO *a,
				  uint32_t gid) {
  LC_CT_STARCOS *lct;
  LC_CLIENT_RESULT res;
  uint32_t nkeyId;
  LC_STARCOS_KEYDESCR *kdescr;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (lct->card==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No card.");
    return GWEN_ERROR_IO;
  }

  if (kid>=0x81 && kid<=0x85)
    nkeyId=0x8f;
  else if (kid>=0x85 && kid<=0x8a)
    nkeyId=0x8e;
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02x", kid);
    return GWEN_ERROR_INVALID;
  }

  /* ensure the access pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* read key descriptor */
  res=LC_Starcos_GetKeyDescr(lct->card, kid, &kdescr);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "here (%d)", res);
    return GWEN_ERROR_IO;
  }

  /* ensure the admin pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAdminPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* first generate temporary key */
  res=LC_Starcos_GenerateKeyPair(lct->card, nkeyId, 768);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    LC_Starcos_KeyDescr_free(kdescr);
    return GWEN_ERROR_IO;
  }

  LC_Starcos_KeyDescr_SetStatus(kdescr, LC_STARCOS_KEY_STATUS_INACTIVE_FREE);
  res=LC_Starcos_SaveKeyDescr(lct->card, kdescr);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    LC_Starcos_KeyDescr_free(kdescr);
    return GWEN_ERROR_IO;
  }

  if (nkeyId==0x8f)
    LC_Starcos_KeyDescr_SetKeyType(kdescr, 'S');
  else
    LC_Starcos_KeyDescr_SetKeyType(kdescr, 'V');
  LC_Starcos_KeyDescr_SetStatus(kdescr, LC_STARCOS_KEY_STATUS_ACTIVE);

  res=LC_Starcos_ActivateKeyPair(lct->card, nkeyId, kid, kdescr);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    LC_Starcos_KeyDescr_free(kdescr);
    return GWEN_ERROR_IO;
  }

  LC_Starcos_KeyDescr_free(kdescr);
  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Sign(GWEN_CRYPT_TOKEN *ct,
			   uint32_t kid,
			   GWEN_CRYPT_PADDALGO *a,
			   const uint8_t *pInData,
			   uint32_t inLen,
			   uint8_t *pSignatureData,
			   uint32_t *pSignatureLen,
			   uint32_t *pSeqCounter,
			   uint32_t gid) {
  LC_CT_STARCOS *lct;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *dbuf;
  uint32_t seq;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (GWEN_Crypt_PaddAlgo_GetId(a)!=GWEN_Crypt_PaddAlgoId_Iso9796_1A4) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid padd algo");
    return GWEN_ERROR_INVALID;
  }

  if (kid<0x81 || kid>0x85) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02d", kid);
    return GWEN_ERROR_INVALID;
  }

  if (inLen!=20) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid hash size");
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    return rv;
  }

  /* get current signature counter */
  res=LC_Starcos_ReadSigCounter(lct->card, kid, &seq);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error reading signature counter (%d)", res);
    return GWEN_ERROR_IO;
  }

  /* set security environment */
  res=LC_Card_IsoManageSe(lct->card, 0xb6,
                          kid & 0xff,
                          kid & 0xff,
			  0x25); /* assume RMD160 and 9796-2 */
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error preparing signing (%d)", res);
    return GWEN_ERROR_IO;
  }

  /* sign */
  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoSign(lct->card,
		      (const char *)pInData,
		      inLen,
		      dbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error signing hash (%d)", res);
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_IO;
  }

  /* copy signature into given buffer */
  if (GWEN_Buffer_GetUsedBytes(dbuf)>*pSignatureLen) {
    DBG_ERROR(LC_LOGDOMAIN, "Buffer overrun (%d>=%d)",
	      GWEN_Buffer_GetUsedBytes(dbuf), *pSignatureLen);
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  memmove(pSignatureData, GWEN_Buffer_GetStart(dbuf), GWEN_Buffer_GetUsedBytes(dbuf));
  *pSignatureLen=GWEN_Buffer_GetUsedBytes(dbuf);
  GWEN_Buffer_free(dbuf);

  /* report used signature counter */
  if (pSeqCounter)
    *pSeqCounter=seq;

  /* done */
  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Verify(GWEN_CRYPT_TOKEN *ct,
			     uint32_t kid,
			     GWEN_CRYPT_PADDALGO *a,
			     const uint8_t *pInData,
			     uint32_t inLen,
			     const uint8_t *pSignatureData,
			     uint32_t signatureLen,
			     uint32_t seqCounter,
			     uint32_t gid) {
  LC_CT_STARCOS *lct;
  LC_CLIENT_RESULT res;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (GWEN_Crypt_PaddAlgo_GetId(a)!=GWEN_Crypt_PaddAlgoId_Iso9796_1A4) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid padd algo");
    return GWEN_ERROR_INVALID;
  }

  if (!(
	(kid>=0x81 && kid<=0x85) ||
	(kid>=0x91 && kid<=0x95)
       )
     ) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02d", kid);
    return GWEN_ERROR_INVALID;
  }

  if (inLen!=20) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid hash size");
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    return rv;
  }

  /* set security environment */
  res=LC_Card_IsoManageSe(lct->card, 0xb6,
			  0,
                          kid & 0xff,
			  0x25); /* assume RMD160 and 9796-2 */
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error preparing verification (%d)", res);
    return GWEN_ERROR_IO;
  }

  /* verify */
  res=LC_Card_IsoVerify(lct->card,
			(const char *)pInData,
			inLen,
			(const char*)pSignatureData,
			signatureLen);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error signing hash (%d)", res);
    return GWEN_ERROR_IO;
  }

  /* done */
  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Encipher(GWEN_CRYPT_TOKEN *ct,
			       uint32_t kid,
			       GWEN_CRYPT_PADDALGO *a,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       uint8_t *pOutData,
			       uint32_t *pOutLen,
			       uint32_t gid) {
  LC_CT_STARCOS *lct;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *dbuf;
  int rv;
  const uint8_t *p;
  uint32_t l;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (GWEN_Crypt_PaddAlgo_GetId(a)!=GWEN_Crypt_PaddAlgoId_LeftZero) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid padd algo");
    return GWEN_ERROR_INVALID;
  }

  if (!(
	(kid>=0x86 && kid<=0x8a) ||
	(kid>=0x96 && kid<=0x9a)
       )
     ){
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02d", kid);
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    return rv;
  }

  /* set security environment */
  res=LC_Card_IsoManageSe(lct->card, 0xb8,
			  0,
			  kid & 0xff,
			  0x03); /* leftzero */
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error preparing encrypting (%d)", res);
    return GWEN_ERROR_IO;
  }

  /* encrypt */
  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoEncipher(lct->card,
			  (const char *)pInData,
			  inLen,
			  dbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error encrypting (%d)", res);
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_IO;
  }

  /* check number of response bytes */
  p=(const uint8_t*)GWEN_Buffer_GetStart(dbuf);
  l=GWEN_Buffer_GetUsedBytes(dbuf);
  if (l>=*pOutLen){
    DBG_ERROR(LC_LOGDOMAIN, "Buffer overrun (%d>=%d)",
	      GWEN_Buffer_GetUsedBytes(dbuf), *pOutLen);
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  else if (l<2){
    DBG_ERROR(LC_LOGDOMAIN, "Too few bytes in response (%d)", l);
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_INTERNAL;
  }

  /* copy result into given buffer */
  if (*p==0) {
    /* skip padd marker */
    p++;
    l--;
  }
  memmove(pOutData, p, l);
  *pOutLen=l;

  GWEN_Buffer_free(dbuf);

  /* done */
  return 0;
}



int GWENHYWFAR_CB
LC_Crypt_TokenStarcos_Decipher(GWEN_CRYPT_TOKEN *ct,
			       uint32_t kid,
			       GWEN_CRYPT_PADDALGO *a,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       uint8_t *pOutData,
			       uint32_t *pOutLen,
			       uint32_t gid) {
  LC_CT_STARCOS *lct;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *dbuf;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, LC_CT_STARCOS, ct);
  assert(lct);

  if (GWEN_Crypt_PaddAlgo_GetId(a)!=GWEN_Crypt_PaddAlgoId_LeftZero &&
      GWEN_Crypt_PaddAlgo_GetId(a)!=GWEN_Crypt_PaddAlgoId_None) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid padd algo (%02x)",
	      GWEN_Crypt_PaddAlgo_GetId(a));
    return GWEN_ERROR_INVALID;
  }

  if (kid<0x86 || kid>0x8a) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid key id %02d", kid);
    return GWEN_ERROR_INVALID;
  }

  /* ensure that the pin is verified */
  rv=LC_Crypt_TokenStarcos__EnsureAccessPin(ct, gid);
  if (rv<0) {
    DBG_ERROR(LC_LOGDOMAIN, "Error on pin entry (%d)", rv);
    return rv;
  }

  /* set security environment */
  res=LC_Card_IsoManageSe(lct->card, 0xb8,
                          kid & 0xff,
			  kid & 0xff,
			  0x03); /* leftzero */
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error preparing decrypting (%d)", res);
    return GWEN_ERROR_IO;
  }

  /* encrypt */
  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (*pInData==0) {
    /* skip padd marker here, it is added automatically in IsoDecipher */
    pInData++;
    inLen--;
  }
  res=LC_Card_IsoDecipher(lct->card,
			  (const char *)pInData,
			  inLen,
			  dbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error decrypting (%d)", res);
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_IO;
  }

  if (GWEN_Crypt_PaddAlgo_GetId(a)==GWEN_Crypt_PaddAlgoId_None &&
      GWEN_Buffer_GetUsedBytes(dbuf)<inLen) {
    GWEN_Buffer_SetPos(dbuf, 0);
    GWEN_Buffer_FillLeftWithBytes(dbuf, 0,
				  inLen-GWEN_Buffer_GetUsedBytes(dbuf));
  }

  /* copy result into given buffer */
  if (GWEN_Buffer_GetUsedBytes(dbuf)>=*pOutLen) {
    DBG_ERROR(LC_LOGDOMAIN, "Buffer overrun (%d>=%d)",
	      GWEN_Buffer_GetUsedBytes(dbuf), *pOutLen);
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  memmove(pOutData, GWEN_Buffer_GetStart(dbuf), GWEN_Buffer_GetUsedBytes(dbuf));
  *pOutLen=GWEN_Buffer_GetUsedBytes(dbuf);
  GWEN_Buffer_free(dbuf);

  /* done */
  return 0;
}









