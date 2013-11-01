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

#include "starcos_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>
#include <chipcard/chipcard.h>
#include <chipcard/cards/processorcard.h>


GWEN_INHERIT(LC_CARD, LC_STARCOS)


static const unsigned char lc_starcos_key_log_order[]={
  0x86, 0x81, 0x91, 0x96, /* account 1 */
  0x87, 0x82, 0x92, 0x97, /* account 2 */
  0x88, 0x83, 0x93, 0x98, /* account 3 */
  0x89, 0x84, 0x94, 0x99, /* account 4 */
  0x8a, 0x85, 0x95, 0x9a, /* account 5 */
  0x8e,                   /* optional: temporary encryption key */
  0x8f,                   /* optional: temporary sign key */
  0                       /* end mark */
};



int LC_Starcos_ExtendCard(LC_CARD *card){
  LC_STARCOS *scos;
  int rv;

  rv=LC_ProcessorCard_ExtendCard(card);
  if (rv) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not extend card as processor card");
    return rv;
  }

  GWEN_NEW_OBJECT(LC_STARCOS, scos);
  GWEN_INHERIT_SETDATA(LC_CARD, LC_STARCOS, card, scos,
                       LC_Starcos_freeData);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  scos->openFn=LC_Card_GetOpenFn(card);
  scos->closeFn=LC_Card_GetCloseFn(card);
  scos->keyDescriptors=LC_Starcos_KeyDescr_List_new();
  LC_Card_SetOpenFn(card, LC_Starcos_Open);
  LC_Card_SetCloseFn(card, LC_Starcos_Close);

  LC_Card_SetGetInitialPinFn(card, LC_Starcos_GetInitialPin);
  LC_Card_SetGetPinStatusFn(card, LC_Starcos_GetPinStatus);

  LC_Card_SetIsoSignFn(card, LC_Starcos__Sign);
  LC_Card_SetIsoVerifyFn(card, LC_Starcos__Verify);

  return 0;
}



int LC_Starcos_UnextendCard(LC_CARD *card){
  LC_STARCOS *scos;
  int rv;

  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);
  LC_Card_SetOpenFn(card, scos->openFn);
  LC_Card_SetCloseFn(card, scos->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_STARCOS, card);

  rv=LC_ProcessorCard_UnextendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here");
  }
  return rv;
}



void GWENHYWFAR_CB LC_Starcos_freeData(void *bp, void *p){
  LC_STARCOS *scos;

  assert(bp);
  assert(p);
  scos=(LC_STARCOS*)p;
  free(scos->appName);
  GWEN_Buffer_free(scos->bin_ef_gd_0);
  GWEN_DB_Group_free(scos->db_ef_gd_0);
  LC_Starcos_KeyDescr_List_free(scos->keyDescriptors);
  GWEN_FREE_OBJECT(scos);
}



LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_STARCOS *scos;

  DBG_INFO(LC_LOGDOMAIN, "Opening card as STARCOS card");

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  GWEN_DB_Group_free(scos->db_ef_gd_0);
  scos->db_ef_gd_0=0;
  GWEN_Buffer_free(scos->bin_ef_gd_0);
  scos->bin_ef_gd_0=0;

  if (strcasecmp(LC_Card_GetCardType(card), "PROCESSOR")!=0) {
    DBG_INFO(LC_LOGDOMAIN, "Not a processor card");
    return LC_Client_ResultNotSupported;
  }

  res=scos->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_Starcos_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    scos->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}




LC_CLIENT_RESULT LC_Starcos__ReadEfToDb(LC_CARD *card,
                                        const char *efName,
                                        const char *formatName,
                                        GWEN_DB_NODE *db) {
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;

  DBG_INFO(LC_LOGDOMAIN, "Selecting EF...");
  res=LC_Card_SelectEf(card, efName);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Reading data...");
  mbuf=GWEN_Buffer_new(0, 16, 0, 1);
  res=LC_Card_IsoReadBinary(card, 0, 0, 12, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  DBG_DEBUG(LC_LOGDOMAIN, "Parsing data...");
  GWEN_Buffer_Rewind(mbuf);
  if (LC_Card_ParseData(card, formatName, mbuf, db)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error in STARCOS data for EF \"%s\"", efName);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }
  GWEN_Buffer_free(mbuf);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_STARCOS *scos;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbVersion;
  int x;
  const char *s;

  DBG_INFO(LC_LOGDOMAIN, "Opening STARCOS card");

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  GWEN_DB_Group_free(scos->db_ef_gd_0);
  scos->db_ef_gd_0=0;
  GWEN_Buffer_free(scos->bin_ef_gd_0);
  scos->bin_ef_gd_0=0;

  res=LC_Card_SelectCard(card, "starcos");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  /* first select generic app for all STARCOS HBCI cards */
  res=LC_Card_SelectApp(card, "starcos");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Selecting MF...");
  res=LC_Card_SelectMf(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  /* try to figure out the correct application */
  dbVersion=GWEN_DB_Group_new("cardVersion");
  res=LC_Starcos__ReadEfToDb(card,
                             "EF_CARD_VERSION", "EF_CARD_VERSION",
                             dbVersion);
  if (res!=LC_Client_ResultOk) {
    if (res==LC_Client_ResultCmdError &&
        LC_Card_GetLastSW1(card)==0x6a &&
        LC_Card_GetLastSW2(card)==0x82) {
      DBG_INFO(LC_LOGDOMAIN,
               "EF_CARD_VERSION not found, assuming normal HBCI card");
    }
    else {
      DBG_INFO(LC_LOGDOMAIN, "here");
      GWEN_DB_Group_free(dbVersion);
      return res;
    }
  }
  else {
    DBG_ERROR(0, "Got this version data:");
    GWEN_DB_Dump(dbVersion, 2);
  
    x=GWEN_DB_GetIntValue(dbVersion, "publisherId", 0, 0x44);
    GWEN_DB_Group_free(dbVersion);
  
    switch(x) {
    case 0x44: s="starcos"; break;
    case 0x47: s="starcos-vr"; break;
    default:   s="starcos"; break;
    }
  
    DBG_INFO(LC_LOGDOMAIN, "Selecting application \"%s\"", s);
    res=LC_Card_SelectApp(card, s);
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here");
      return res;
    }
  }

  /* now read card data, initial pin etc */
  DBG_INFO(LC_LOGDOMAIN, "Selecting EF...");
  res=LC_Card_SelectEf(card, "EF_GD0");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Reading data...");
  mbuf=GWEN_Buffer_new(0, 16, 0, 1);
  res=LC_Card_IsoReadBinary(card, 0, 0, 12, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }
  if (GWEN_Buffer_GetUsedBytes(mbuf)<12) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  DBG_DEBUG(LC_LOGDOMAIN, "Parsing data...");
  GWEN_Buffer_Rewind(mbuf);
  memmove(scos->initialPin,
          GWEN_Buffer_GetStart(mbuf)+6,
          sizeof(scos->initialPin));
  dbData=GWEN_DB_Group_new("cardData");
  if (LC_Card_ParseData(card, "EF_GD0", mbuf, dbData)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error in STARCOS card data");
    GWEN_DB_Group_free(dbData);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  /* select banking DF */
  DBG_INFO(LC_LOGDOMAIN, "Selecting DF_BANKING...");
  res=LC_Card_SelectDf(card, "DF_BANKING");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbData);
    GWEN_Buffer_free(mbuf);
    return res;
  }

  scos->db_ef_gd_0=dbData;
  scos->bin_ef_gd_0=mbuf;
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_STARCOS *scos;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=scos->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}



GWEN_DB_NODE *LC_Starcos_GetCardDataAsDb(const LC_CARD *card){
  LC_STARCOS *scos;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  return scos->db_ef_gd_0;
}



GWEN_BUFFER *LC_Starcos_GetCardDataAsBuffer(const LC_CARD *card){
  LC_STARCOS *scos;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  return scos->bin_ef_gd_0;
}



LC_CLIENT_RESULT CHIPCARD_CB 
LC_Starcos_GetPinStatus(LC_CARD *card,
                        unsigned int pid,
                        int *maxErrors,
                        int *currentErrors) {
  LC_STARCOS *scos;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  unsigned int v;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  dbReq=GWEN_DB_Group_new("PinStatus");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "pid", pid);
  res=LC_Card_ExecCommand(card, "PinStatus", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error while executing PinStatus");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  v=(unsigned int)GWEN_DB_GetIntValue(dbResp, "response/status", 0, 0);
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  if (v==0) {
    DBG_INFO(LC_LOGDOMAIN, "No pin status received");
    return LC_Client_ResultDataError;
  }
  if (maxErrors)
    *maxErrors=(((unsigned char)(v))>>4)&0xf;
  if (currentErrors)
    *currentErrors=((unsigned char)(v))&0xf;

  return res;
}



int LC_Starcos__GetKeyDescrOffset(int kid) {
  const unsigned char *p;
  int i;

  i=0;
  p=lc_starcos_key_log_order;
  while(*p && *p!=(unsigned char)kid) {
    p++;
    i++;
  }
  if (!*p)
    return -1;
  return 1+(i*8);
}



LC_CLIENT_RESULT LC_Starcos__GetKeyLogInfo(LC_CARD *card,
					   unsigned int *pResult) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  if (scos->keyLogInfo==0) {
    DBG_INFO(LC_LOGDOMAIN, "Reading keylog info");

    res=LC_Card_SelectEf(card, "EF_KEY_LOG");
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "File EF_KEY_LOG not available");
      return res;
    }

    mbuf=GWEN_Buffer_new(0, 16, 0, 1);
    res=LC_Card_IsoReadBinary(card, 0, 0, 1, mbuf);
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "Error reading info byte of EF_KEYLOG");
      GWEN_Buffer_free(mbuf);
      return res;
    }

    GWEN_Buffer_Rewind(mbuf);
    scos->keyLogInfo=(unsigned char)(*GWEN_Buffer_GetStart(mbuf));

    GWEN_Buffer_free(mbuf);
  }
  *pResult=scos->keyLogInfo;
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos__SaveKeyLogInfo(LC_CARD *card) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  DBG_INFO(LC_LOGDOMAIN, "Writing keylog info");

  if (!scos->keyLogInfo)
    return LC_Client_ResultOk;

  res=LC_Card_SelectEf(card, "EF_KEY_LOG");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "File EF_KEY_LOG not available");
    return res;
  }

  mbuf=GWEN_Buffer_new(0, 8, 0, 1);
  GWEN_Buffer_AppendByte(mbuf, (unsigned char)(scos->keyLogInfo));
  GWEN_Buffer_Rewind(mbuf);
  res=LC_Card_IsoUpdateBinary(card, 0, 0,
                              GWEN_Buffer_GetStart(mbuf),
                              GWEN_Buffer_GetUsedBytes(mbuf));
  GWEN_Buffer_free(mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error reading info byte of EF_KEYLOG");
    return res;
  }
  return LC_Client_ResultOk;
}





LC_STARCOS_KEYDESCR *LC_Starcos__FindKeyDescr(LC_CARD *card, int kid) {
  LC_STARCOS *scos;
  LC_STARCOS_KEYDESCR *d;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  d=LC_Starcos_KeyDescr_List_First(scos->keyDescriptors);
  while(d) {
    if (LC_Starcos_KeyDescr_GetKeyId(d)==kid)
      break;
    d=LC_Starcos_KeyDescr_List_Next(d);
  } /* while */

  return d;
}



LC_CLIENT_RESULT LC_Starcos__LoadKeyDescr(LC_CARD *card, int kid,
					  LC_STARCOS_KEYDESCR **pDescr) {
  LC_STARCOS *scos;
  LC_STARCOS_KEYDESCR *d;
  LC_CLIENT_RESULT res;
  int offset;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *dbData;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  offset=LC_Starcos__GetKeyDescrOffset(kid);
  if (offset==-1) {
    DBG_INFO(LC_LOGDOMAIN, "Key %02x not available", kid);
    return LC_Client_ResultNotFound;
  }

  res=LC_Card_SelectEf(card, "EF_KEY_LOG");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "File EF_KEY_LOG not available");
    return res;
  }

  mbuf=GWEN_Buffer_new(0, 16, 0, 1);
  res=LC_Card_IsoReadBinary(card, 0, offset, 8, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error reading descriptor");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  dbData=GWEN_DB_Group_new("keyDescr");
  GWEN_Buffer_Rewind(mbuf);
  res=LC_Card_ParseData(card, "KeyLogDescr", mbuf, dbData);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error reading descriptor");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  GWEN_Buffer_free(mbuf);
  d=LC_Starcos_KeyDescr_fromDb(dbData);
  if (!d) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing descriptor data");
    GWEN_DB_Group_free(dbData);
    return LC_Client_ResultDataError;
  }
  LC_Starcos_KeyDescr_SetKeyId(d, kid);
  GWEN_DB_Group_free(dbData);

  *pDescr=d;
  return 0;
}



LC_CLIENT_RESULT LC_Starcos_SaveKeyDescr(LC_CARD *card,
					 const LC_STARCOS_KEYDESCR *d) {
  LC_STARCOS *scos;
  GWEN_DB_NODE *dbDescr;
  GWEN_BUFFER *mbuf;
  int offset;
  int kid;
  LC_CLIENT_RESULT res;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  kid=LC_Starcos_KeyDescr_GetKeyId(d);
  offset=LC_Starcos__GetKeyDescrOffset(kid);
  if (offset==-1) {
    DBG_INFO(LC_LOGDOMAIN, "Key %02x not available", kid);
    return LC_Client_ResultInvalid;
  }

  res=LC_Card_SelectEf(card, "EF_KEY_LOG");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "File EF_KEY_LOG not available");
    return LC_Client_ResultGeneric;
  }

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  dbDescr=GWEN_DB_Group_new("descriptor");
  assert(dbDescr);
  if (LC_Starcos_KeyDescr_toDb(d, dbDescr)) {
    DBG_ERROR(LC_LOGDOMAIN, "Internal error");
    GWEN_DB_Group_free(dbDescr);
    abort();
  }

  mbuf=GWEN_Buffer_new(0, 16, 0, 1);
  if (LC_Card_CreateData(card, "KeyLogDescr", mbuf, dbDescr)) {
    DBG_ERROR(LC_LOGDOMAIN, "Bad data in key descriptor");
    GWEN_Buffer_free(mbuf);
    GWEN_DB_Group_free(dbDescr);
    return LC_Client_ResultDataError;
  }
  GWEN_DB_Group_free(dbDescr);

  GWEN_Buffer_Rewind(mbuf);
  res=LC_Card_IsoUpdateBinary(card, 0, offset,
                              GWEN_Buffer_GetStart(mbuf),
                              GWEN_Buffer_GetUsedBytes(mbuf));
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error writing descriptor");
    GWEN_Buffer_free(mbuf);
    return res;
  }
  GWEN_Buffer_free(mbuf);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos_GetKeyDescr(LC_CARD *card, int kid,
					LC_STARCOS_KEYDESCR **pDescr) {
  LC_STARCOS *scos;
  LC_STARCOS_KEYDESCR *d;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  d=LC_Starcos__FindKeyDescr(card, kid);
  if (d==NULL) {
    LC_CLIENT_RESULT res;

    res=LC_Starcos__LoadKeyDescr(card, kid, &d);
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
      return res;
    }
    LC_Starcos_KeyDescr_List_Add(d, scos->keyDescriptors);
  }

  *pDescr=d;
  return 0;
}



LC_CLIENT_RESULT LC_Starcos_GenerateKeyPair(LC_CARD *card,
                                            int kid,
                                            int bits) {
  LC_STARCOS *scos;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  unsigned int kli;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  DBG_INFO(LC_LOGDOMAIN, "Reading keylog info");
  res=LC_Starcos__GetKeyLogInfo(card, &kli);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }
  if (kid==0x8e) {
    if (kli & 0x08) {
      kli&=~0x08;
      scos->keyLogInfo=kli;
      DBG_INFO(LC_LOGDOMAIN, "Saving keylog info");
      res=LC_Starcos__SaveKeyLogInfo(card);
      if (res!=LC_Client_ResultOk) {
        DBG_INFO(LC_LOGDOMAIN, "here");
        return res;
      }
    }
  }
  else if (kid==0x8f) {
    if (kli & 0x80) {
      kli&=~0x80;
      scos->keyLogInfo=kli;
      res=LC_Starcos__SaveKeyLogInfo(card);
      if (res!=LC_Client_ResultOk) {
        DBG_INFO(LC_LOGDOMAIN, "here");
        return res;
      }
    }
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN,
              "Will only generate keys for KIDs 0x8e and 0x8f (%02x)",
              kid);
    return LC_Client_ResultInvalid;
  }

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  DBG_INFO(LC_LOGDOMAIN, "Generating key pair");
  dbReq=GWEN_DB_Group_new("GenerateKeyPair");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "kid", kid);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "bits", bits);
  res=LC_Card_ExecCommand(card, "GenerateKeyPair", dbReq, dbResp);
  scos->keyLogInfo=0;
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return res;
}



LC_CLIENT_RESULT LC_Starcos_ActivateKeyPair(LC_CARD *card,
                                            int srcKid,
					    int dstKid,
					    const LC_STARCOS_KEYDESCR *descr){
  LC_STARCOS *scos;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbDescr;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  unsigned int kli;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Starcos__GetKeyLogInfo(card, &kli);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  if (srcKid==0x8e) {
    if (!(kli & 0x08)) {
      DBG_ERROR(LC_LOGDOMAIN, "No key, please create one");
      return LC_Client_ResultInvalid;
    }
    if (dstKid<0x86 || dstKid>0x8a) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Bad combination: "
                "Allowed for source KID 0x8e are 0x86-a (%02x)",
                dstKid);
      return LC_Client_ResultInvalid;
    }
  }
  else if (srcKid==0x8f) {
    if (!(kli & 0x80)) {
      DBG_ERROR(LC_LOGDOMAIN, "No key, please create one");
      return LC_Client_ResultInvalid;
    }
    if (dstKid<0x81 || dstKid>0x85) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Bad combination: "
                "Allowed for source KID 0x8f are 0x81-5 (%02x)",
                dstKid);
      return LC_Client_ResultInvalid;
    }
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN,
              "Only 0x8e and 0x8f are accepted as source KIDs(%02x)",
              srcKid);
    return LC_Client_ResultInvalid;
  }

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  dbReq=GWEN_DB_Group_new("ActivateKeyPair");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "srckid", srcKid);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "destkid", dstKid);
  dbDescr=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_DEFAULT, "descriptor");
  assert(dbDescr);
  if (LC_Starcos_KeyDescr_toDb(descr, dbDescr)) {
    DBG_ERROR(LC_LOGDOMAIN, "Internal error");
    abort();
  }
  res=LC_Card_ExecCommand(card, "ActivateKeyPair", dbReq, dbResp);
  scos->keyLogInfo=0;
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return res;
}



int LC_Starcos__IsSignKey(int kid) {
  if (kid>=0x86 && kid<=0x8a)
    return 0;
  if (kid>=0x96 && kid<=0x9a)
    return 0;
  if (kid==0x8e)
    return 0;
  return 1;
}



int LC_Starcos__IsCryptKey(int kid) {
  return !LC_Starcos__IsSignKey(kid);
}



int LC_Starcos__GetIpfKeyOffset(LC_CARD *card, int kid) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;
  int pos;
  int keyCount;
  int i;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_SelectEf(card, "EF_IPF");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "File EF_IPF not available");
    return 0;
  }
  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  mbuf=GWEN_Buffer_new(0, 16, 0, 1);
  res=LC_Card_IsoReadBinary(card, 0, 0, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error reading keycount from EF_IPF[%d]", 0);
    GWEN_Buffer_free(mbuf);
    return -1;
  }

  keyCount=(unsigned char)(*GWEN_Buffer_GetStart(mbuf));
  DBG_INFO(LC_LOGDOMAIN, "%d keys total", keyCount);
  i=0;
  pos=1;
  for (i=0; i<keyCount; i++) {
    GWEN_Buffer_Reset(mbuf);
    LC_Card_SetLastResult(card, 0, 0, 0, 0);
    res=LC_Card_IsoReadBinary(card, 0, pos, 1, mbuf);
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "Error reading kid from EF_IPF[%d]", pos);
      GWEN_Buffer_free(mbuf);
      return -1;
    }
    if ((unsigned char)(*GWEN_Buffer_GetStart(mbuf))==(unsigned char)kid)
      break;
    pos+=121;
  } /* for */
  GWEN_Buffer_free(mbuf);

  return pos;
}





LC_CLIENT_RESULT LC_Starcos_WritePublicKey(LC_CARD *card, int kid,
					   const uint8_t *pModulus,
					   uint32_t lModulus,
					   const uint8_t *pExponent,
					   uint32_t lExponent) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;
  int pos;
  unsigned char algoByte;
  int modLen;
  const void *p;
  unsigned int bs;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  /* plausability check */
  if (kid<0x91 || kid>0x9a) {
    DBG_ERROR(LC_LOGDOMAIN,
              "Bad key id for writing (allowed: 0x91 <= x <= 0x9a, is:%02x)",
              kid);
    return LC_Client_ResultInvalid;
  }

  /* get write pos */
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  pos=LC_Starcos__GetIpfKeyOffset(card, kid);
  if (pos==-1) {
    DBG_ERROR(LC_LOGDOMAIN, "Key %02x not found in EF_IPF", kid);
    return LC_Client_ResultInvalid;
  }
  DBG_ERROR(LC_LOGDOMAIN, "Key %02x is at %04x", kid, pos);

  mbuf=GWEN_Buffer_new(0, 128, 0, 1);

  /* read AlgoByte */
  res=LC_Card_IsoReadBinary(card, 0, pos+6, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }
  algoByte=(unsigned char)(*GWEN_Buffer_GetStart(mbuf));

  /* handle modulus */
  p=pModulus;
  bs=lModulus;
  modLen=bs;

  /* write modulus to buffer */
  GWEN_Buffer_Reset(mbuf);
  if (algoByte & 0x08) {
    const char *s;
    int i;

    DBG_INFO(LC_LOGDOMAIN, "Need to mirror the modulus");

    /* we have to mirror the modulus */
    s=(const char*)p+bs;
    for (i=0; i<(int)bs; i++)
      GWEN_Buffer_AppendByte(mbuf, *(--s));
  }
  else {
    /* simply add modulus */
    GWEN_Buffer_AppendBytes(mbuf, p, bs);
  }

  /* write modulus to card */
  GWEN_Buffer_Rewind(mbuf);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_IsoUpdateBinary(card, 0, pos+20,
                              GWEN_Buffer_GetStart(mbuf),
                              GWEN_Buffer_GetUsedBytes(mbuf));
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  /* write modLen */
  GWEN_Buffer_Reset(mbuf);
  GWEN_Buffer_AppendByte(mbuf, modLen);
  GWEN_Buffer_Rewind(mbuf);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_IsoUpdateBinary(card, 0, pos+14,
                              GWEN_Buffer_GetStart(mbuf),
                              GWEN_Buffer_GetUsedBytes(mbuf));
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  /* write empty space len */
  GWEN_Buffer_Reset(mbuf);
  GWEN_Buffer_AppendByte(mbuf, 0x60-modLen);
  GWEN_Buffer_Rewind(mbuf);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_IsoUpdateBinary(card, 0, pos+18,
                              GWEN_Buffer_GetStart(mbuf),
                              GWEN_Buffer_GetUsedBytes(mbuf));
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  /* handle exponent */
  p=pExponent;
  bs=lExponent;

  /* write exponent to buffer */
  GWEN_Buffer_Reset(mbuf);
  GWEN_Buffer_AppendBytes(mbuf, p, bs);

  /* write exponent to card */
  GWEN_Buffer_Rewind(mbuf);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_IsoUpdateBinary(card, 0, pos+20+modLen,
                              GWEN_Buffer_GetStart(mbuf),
                              GWEN_Buffer_GetUsedBytes(mbuf));
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  GWEN_Buffer_free(mbuf);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos_ReadPublicKey(LC_CARD *card, int kid,
					  GWEN_BUFFER *bModulus,
					  GWEN_BUFFER *bExponent) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *mbuf;
  int pos;
  GWEN_DB_NODE *dbData;
  int modLen;
  const void *p;
  unsigned int bs;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  /* plausability check */
  if (!(
        (kid>=0x81 && kid<=0x8a) ||
        (kid>=0x91 && kid<=0x9a) ||
        kid==0x8e ||
        kid==0x8f
       )
     ) {
    DBG_ERROR(LC_LOGDOMAIN,
              "Bad key id for reading (%02x)",
              kid);
    return LC_Client_ResultInvalid;
  }

  /* get read pos */
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  pos=LC_Starcos__GetIpfKeyOffset(card, kid);
  if (pos==-1) {
    DBG_ERROR(LC_LOGDOMAIN, "Key %02x not found in EF_IPF", kid);
    return LC_Client_ResultNotFound;
  }

  /* read key to buffer */
  mbuf=GWEN_Buffer_new(0, 128, 0, 1);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_IsoReadBinary(card, 0, pos, 121, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  /* parse buffer */
  GWEN_Buffer_Rewind(mbuf);
  dbData=GWEN_DB_Group_new("IpfKey");
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_ParseData(card, "IpfKey", mbuf, dbData);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbData);
    GWEN_Buffer_free(mbuf);
    return res;
  }
  GWEN_Buffer_free(mbuf);

  modLen=GWEN_DB_GetIntValue(dbData, "modlen", 0, 0);
  if (!modLen) {
    DBG_ERROR(LC_LOGDOMAIN, "No modulus");
    GWEN_DB_Group_free(dbData);
    return LC_Client_ResultDataError;

  }
  if (modLen>96) {
    DBG_ERROR(LC_LOGDOMAIN, "Modulus/exponent too long");
    GWEN_DB_Group_free(dbData);
    return LC_Client_ResultDataError;
  }
  p=GWEN_DB_GetBinValue(dbData, "modAndExpo", 0, 0, 0, &bs);
  if (!p || bs<99) {
    DBG_ERROR(LC_LOGDOMAIN, "Modulus/exponent too small");
    GWEN_DB_Group_free(dbData);
    return LC_Client_ResultDataError;
  }

  if (GWEN_DB_GetIntValue(dbData, "algoByte", 0, 0) & 0x08) {
    const char *s;
    int i;

    /* we have to mirror the modulus */
    DBG_INFO(LC_LOGDOMAIN, "Mirroring modulus");
    s=(const char*)p+modLen;
    for (i=0; i<modLen; i++)
      GWEN_Buffer_AppendByte(bModulus, *(--s));
  }
  else
    GWEN_Buffer_AppendBytes(bModulus, p, modLen);

  GWEN_Buffer_AppendBytes(bExponent, p+modLen, 3);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos_ReadInstituteData(LC_CARD *card,
                                              int idx,
                                              GWEN_DB_NODE *dbData) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *buf;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  res=LC_Card_SelectEf(card, "EF_BNK");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
			    idx, buf);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing record %d", idx);
    GWEN_Buffer_free(buf);
    return res;
  }

  GWEN_Buffer_Rewind(buf);
  if (GWEN_Buffer_GetUsedBytes(buf)) {
    if ((unsigned char)(GWEN_Buffer_GetStart(buf)[0])!=0xff) {
      if (LC_Card_ParseRecord(card, idx, buf, dbData)) {
	DBG_ERROR(LC_LOGDOMAIN, "Error parsing record %d", idx);
	GWEN_Buffer_free(buf);
	return LC_Client_ResultDataError;
      }
    } /* if buffer content is valid */
  } /* if buffer not empty */
  GWEN_Buffer_free(buf);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos_WriteInstituteData(LC_CARD *card,
                                               int idx,
                                               GWEN_DB_NODE *dbData) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *buf;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  if (!idx || idx>5) {
    DBG_ERROR(LC_LOGDOMAIN, "Bad index");
    return LC_Client_ResultInvalid;
  }
  res=LC_Card_SelectEf(card, "EF_BNK");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_CreateRecord(card, idx, buf, dbData);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(buf);
    return res;
  }

  GWEN_Buffer_Rewind(buf);
  res=LC_Card_IsoUpdateRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                              idx,
                              GWEN_Buffer_GetStart(buf),
                              GWEN_Buffer_GetUsedBytes(buf));
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(buf);
    return res;
  }
  GWEN_Buffer_free(buf);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos_ReadSigCounter(LC_CARD *card, int kid,
					   uint32_t *pSeq) {
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  unsigned int i;
  GWEN_BUFFER *buf;
  uint32_t seq;
  GWEN_DB_NODE *dbData;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  if (kid<0x81 || kid>0x85) {
    DBG_ERROR(LC_LOGDOMAIN,
              "Bad key id (accepted: 0x81-0x85, is: %02x)",
              kid);
    return LC_Client_ResultInvalid;
  }
  i=kid-0x80;

  res=LC_Card_SelectEf(card, "EF_SEQ");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (res=%d)", res);
    return res;
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                            i, buf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (res=%d)", res);
    GWEN_Buffer_free(buf);
    return res;
  }
  GWEN_Buffer_Rewind(buf);
  dbData=GWEN_DB_Group_new("signcounter");
  res=LC_Card_ParseRecord(card, i, buf, dbData);
  if (res!=LC_Client_ResultOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing record %d (%d)", i, res);
    GWEN_DB_Group_free(dbData);
    GWEN_Buffer_free(buf);
    return res;
  }
  GWEN_Buffer_free(buf);

  seq=(uint32_t)GWEN_DB_GetIntValue(dbData, "seq", 0, 0);
  if (seq==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No signature counter in data");
    GWEN_DB_Dump(dbData, 2);
    GWEN_DB_Group_free(dbData);
    return LC_Client_ResultInternal;
  }
  GWEN_DB_Group_free(dbData);

  *pSeq=seq;
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos__Sign(LC_CARD *card,
                                              const char *ptr,
                                              unsigned int size,
                                              GWEN_BUFFER *sigBuf) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbRsp;
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  /* put hash */
  dbReq=GWEN_DB_Group_new("PutHash");
  dbRsp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "data", ptr, size);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_ExecCommand(card, "PutHash", dbReq, dbRsp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbRsp);
    return res;
  }
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbRsp);

  /* sign hash */
  dbReq=GWEN_DB_Group_new("Sign");
  dbRsp=GWEN_DB_Group_new("response");
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_ExecCommand(card, "Sign", dbReq, dbRsp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }

  /* extract the signature */
  p=GWEN_DB_GetBinValue(dbRsp, "response/signature", 0, 0, 0, &bs);
  if (!p || !bs) {
    DBG_ERROR(LC_LOGDOMAIN, "No signature returned by card");
    GWEN_DB_Dump(dbRsp, 2);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }
  GWEN_Buffer_AppendBytes(sigBuf, p, bs);
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbRsp);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos__Verify(LC_CARD *card,
                                                const char *ptr,
                                                unsigned int size,
                                                const char *sigptr,
                                                unsigned int sigsize) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbRsp;
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  /* put hash */
  dbReq=GWEN_DB_Group_new("PutHash");
  dbRsp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "data", ptr, size);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_ExecCommand(card, "PutHash", dbReq, dbRsp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbRsp);

  /* verify hash */
  dbReq=GWEN_DB_Group_new("Verify");
  dbRsp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "signature",
                      sigptr, sigsize);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_ExecCommand(card, "Verify", dbReq, dbRsp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }

  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbRsp);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Starcos_GetChallenge(LC_CARD *card, GWEN_BUFFER *mbuf) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_STARCOS *scos;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  dbReq=GWEN_DB_Group_new("Challenge");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "lr", 8);

  dbResp=GWEN_DB_Group_new("response");
  res=LC_Card_ExecCommand(card, "Challenge", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  p=GWEN_DB_GetBinValue(dbResp, "response/random", 0, 0, 0, &bs);
  if (!p || !bs) {
    DBG_ERROR(LC_LOGDOMAIN, "No data returned by card");
    GWEN_DB_Dump(dbResp, 2);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }
  GWEN_Buffer_AppendBytes(mbuf, p, bs);

  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB 
LC_Starcos_GetInitialPin(LC_CARD *card,
                         int pid,
                         unsigned char *buffer,
                         unsigned int maxSize,
                         unsigned int *pinLength) {
  LC_STARCOS *scos;

  assert(card);
  scos=GWEN_INHERIT_GETDATA(LC_CARD, LC_STARCOS, card);
  assert(scos);

  if (sizeof(scos->initialPin)>maxSize) {
    DBG_ERROR(LC_LOGDOMAIN, "Buffer too small");
    return LC_Client_ResultInvalid;
  }

  memmove(buffer, scos->initialPin, sizeof(scos->initialPin));
  *pinLength=sizeof(scos->initialPin);
  return 0;
}





