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
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>
#include <chipcard/chipcard.h>
#include <chipcard/cards/processorcard.h>


GWEN_INHERIT(LC_CARD, LC_DDVCARD)



int LC_DDVCard_ExtendCard(LC_CARD *card){
  LC_DDVCARD *ddv;
  int rv;

  rv=LC_ProcessorCard_ExtendCard(card);
  if (rv) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not extend card as processor card");
    return rv;
  }

  GWEN_NEW_OBJECT(LC_DDVCARD, ddv);

  ddv->ddvType=-1;
  ddv->openFn=LC_Card_GetOpenFn(card);
  ddv->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, LC_DDVCard_Open);
  LC_Card_SetCloseFn(card, LC_DDVCard_Close);

  GWEN_INHERIT_SETDATA(LC_CARD, LC_DDVCARD, card, ddv,
                       LC_DDVCard_freeData);
  return 0;
}



int LC_DDVCard_UnextendCard(LC_CARD *card){
  LC_DDVCARD *ddv;
  int rv;

  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);
  LC_Card_SetOpenFn(card, ddv->openFn);
  LC_Card_SetCloseFn(card, ddv->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_DDVCARD, card);

  rv=LC_ProcessorCard_UnextendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here");
  }
  return rv;
}



void GWENHYWFAR_CB LC_DDVCard_freeData(void *bp, void *p){
  LC_DDVCARD *ddv;

  assert(bp);
  assert(p);
  ddv=(LC_DDVCARD*)p;
  GWEN_Buffer_free(ddv->bin_ef_id_1);
  GWEN_DB_Group_free(ddv->db_ef_id_1);
  GWEN_FREE_OBJECT(ddv);
}



LC_CLIENT_RESULT CHIPCARD_CB LC_DDVCard_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_DDVCARD *ddv;

  DBG_INFO(LC_LOGDOMAIN, "Opening card as DDV card");

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  ddv->ddvType=-1;
  GWEN_DB_Group_free(ddv->db_ef_id_1);
  ddv->db_ef_id_1=0;
  GWEN_Buffer_free(ddv->bin_ef_id_1);
  ddv->bin_ef_id_1=0;

  if (strcasecmp(LC_Card_GetCardType(card), "PROCESSOR")!=0) {
    DBG_ERROR(LC_LOGDOMAIN, "Not a processor card (%s)",
              LC_Card_GetCardType(card));
    return LC_Client_ResultNotSupported;
  }

  res=ddv->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_DDVCard_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    ddv->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_DDVCard_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_DDVCARD *ddv;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *dbRecord;

  DBG_INFO(LC_LOGDOMAIN, "Opening DDV card");

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  ddv->ddvType=-1;
  GWEN_DB_Group_free(ddv->db_ef_id_1);
  ddv->db_ef_id_1=0;
  GWEN_Buffer_free(ddv->bin_ef_id_1);
  ddv->bin_ef_id_1=0;

  res=LC_Card_SelectCard(card, "ProcessorCard");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_Card_SelectApp(card, "ddv");
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

  DBG_INFO(LC_LOGDOMAIN, "Selecting EF...");
  res=LC_Card_SelectEf(card, "EF_ID");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Reading record...");
  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Parsing record...");
  GWEN_Buffer_Rewind(mbuf);
  dbRecord=GWEN_DB_Group_new("record");
  if (LC_Card_ParseRecord(card, 1, mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error in EF_ID");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  if (GWEN_Buffer_GetUsedBytes(mbuf)>22) {
    /* assume DDV1, perform some checks */

    /* check for currency (DDV1 allows EUR only) */
    if (strcasecmp(GWEN_DB_GetCharValue(dbRecord,
                                        "currency",
                                        0, ""),
                   "EUR")!=0) {
      DBG_ERROR(LC_LOGDOMAIN, "Bad currency, this does not seem to be DDV1 card");
      GWEN_DB_Group_free(dbRecord);
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }

    /* check for OS version on the chip card (DDV1 needs 1 or higher) */
    if (GWEN_DB_GetIntValue(dbRecord, "OSVersion",0, 0)<1) {
      DBG_ERROR(LC_LOGDOMAIN, "Bad CardOS version, this does not seem to be DDV1 card");
      GWEN_DB_Group_free(dbRecord);
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }

    /* check for filler (DDV1 needs 0) */
    if (GWEN_DB_GetIntValue(dbRecord, "filler",0, 0)!=0) {
      DBG_ERROR(LC_LOGDOMAIN, "Bad byte at pos 23, this does not seem to be DDV1 card");
      GWEN_DB_Group_free(dbRecord);
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }

    /* check for factor (DDV1 needs 1) */
    if (GWEN_DB_GetIntValue(dbRecord, "factor", 0, 1)!=1) {
      DBG_ERROR(LC_LOGDOMAIN, "Bad byte at pos 24, this does not seem to be DDV1 card");
      GWEN_DB_Group_free(dbRecord);
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }

    /* select DDV1 card and app */
    res=LC_Card_SelectCard(card, "ddv1");
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here");
      return res;
    }
    res=LC_Card_SelectApp(card, "ddv1");
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here");
      return res;
    }

    /* select correct banking DF */
    res=LC_Card_SelectDf(card, "DF_BANKING_20");
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here");
      GWEN_DB_Group_free(dbRecord);
      GWEN_Buffer_free(mbuf);
      return res;
    }

    DBG_INFO(LC_LOGDOMAIN, "Card type is DDV 1");
    ddv->ddvType=1;
  }
  else {
    /* assume DDV0, perform some checks */

    /* check for currency (DDV0 allows DEM only) */
    if (strcasecmp(GWEN_DB_GetCharValue(dbRecord,
                                        "currency",
                                        0, ""),
                   "DEM")!=0) {
      DBG_ERROR(LC_LOGDOMAIN, "Bad currency, this does not seem to be DDV0 card");
      GWEN_DB_Group_free(dbRecord);
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }

    /* select DDV0 card and app */
    res=LC_Card_SelectCard(card, "ddv0");
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here");
      return res;
    }
    res=LC_Card_SelectApp(card, "ddv0");
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here");
      return res;
    }

    /* select correct banking DF */
    res=LC_Card_SelectDf(card, "DF_BANKING");
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here");
      GWEN_DB_Group_free(dbRecord);
      GWEN_Buffer_free(mbuf);
      return res;
    }

    DBG_INFO(LC_LOGDOMAIN, "Card type is DDV 0");
    ddv->ddvType=0;
  }

  ddv->db_ef_id_1=dbRecord;
  ddv->bin_ef_id_1=mbuf;
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_DDVCard_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  res=ddv->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}



LC_CLIENT_RESULT LC_DDVCard_VerifyPin(LC_CARD *card, const char *pin){
  LC_DDVCARD *ddv;
  LC_CLIENT_RESULT res;
  LC_PININFO *pi;
  int triesLeft=-1;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  pi=LC_Card_GetPinInfoByName(card, "ch_pin");
  assert(pi);
  res=LC_Card_IsoVerifyPin(card, 0, pi,
			   (const unsigned char*)pin, strlen(pin),
			   &triesLeft);
  LC_PinInfo_free(pi);
  return res;
}



LC_CLIENT_RESULT LC_DDVCard_SecureVerifyPin(LC_CARD *card){
  LC_DDVCARD *ddv;
  LC_CLIENT_RESULT res;
  LC_PININFO *pi;
  int triesLeft=-1;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  pi=LC_Card_GetPinInfoByName(card, "ch_pin");
  assert(pi);
  res=LC_Card_IsoPerformVerification(card, 0, pi, &triesLeft);
  LC_PinInfo_free(pi);
  return res;
}



int LC_DDVCard_GetCryptKeyVersion0(LC_CARD *card){
  LC_DDVCARD *ddv;
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbRecord;
  GWEN_BUFFER *mbuf;
  int keyVer;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  res=LC_Card_SelectEf(card, "EF_AUTD");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return -1;
  }

  mbuf=GWEN_Buffer_new(0, 4, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return -1;
  }
  GWEN_Buffer_Rewind(mbuf);
  dbRecord=GWEN_DB_Group_new("autd");
  if (LC_Card_ParseRecord(card, 1, mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing record");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return -1;
  }
  GWEN_Buffer_free(mbuf);

  keyVer=GWEN_DB_GetIntValue(dbRecord, "keyVersion", 0, -1);
  GWEN_DB_Group_free(dbRecord);

  if (keyVer==-1) {
    DBG_ERROR(LC_LOGDOMAIN, "No keyVersion in record 2 of EF_AUTD");
  }
  return keyVer;
}



int LC_DDVCard_GetSignKeyVersion0(LC_CARD *card){
  LC_DDVCARD *ddv;
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbRecord;
  GWEN_BUFFER *mbuf;
  int keyVer;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  res=LC_Card_SelectEf(card, "EF_KEYD");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return -1;
  }

  mbuf=GWEN_Buffer_new(0, 4, 0, 1);
  res=LC_Card_IsoReadRecord(card,
                            LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                            1 /* should be 2 */,
                            mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return -1;
  }
  GWEN_Buffer_Rewind(mbuf);
  dbRecord=GWEN_DB_Group_new("autd");
  if (LC_Card_ParseRecord(card,
                          1 /* should be 2, but that doesn't work */,
                          mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing record");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return -1;
  }
  GWEN_Buffer_free(mbuf);

  keyVer=GWEN_DB_GetIntValue(dbRecord, "keyVersion", 0, -1);
  GWEN_DB_Group_free(dbRecord);

  if (keyVer==-1) {
    DBG_ERROR(LC_LOGDOMAIN, "No keyVersion in record 2 of EF_KEYD");
  }

  return keyVer;
}



int LC_DDVCard_GetKeyVersion1(LC_CARD *card, int keyNumber){
  LC_DDVCARD *ddv;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  int keyVersion;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  dbReq=GWEN_DB_Group_new("GetKeyInfo");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "keyNumber", keyNumber);
  res=LC_Card_ExecCommand(card, "GetKeyInfo", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return -1;
  }

  keyVersion=GWEN_DB_GetIntValue(dbResp,
                                 "response/keyVersion", 0, -1);
  if (keyVersion==-1) {
    DBG_ERROR(LC_LOGDOMAIN, "No keyversion returned by command");
  }
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return keyVersion;
}



int LC_DDVCard_GetSignKeyVersion1(LC_CARD *card){
  return LC_DDVCard_GetKeyVersion1(card, 2);
}



int LC_DDVCard_GetCryptKeyVersion1(LC_CARD *card){
  return LC_DDVCard_GetKeyVersion1(card, 3);
}



int LC_DDVCard_GetSignKeyVersion(LC_CARD *card){
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (ddv->ddvType==0) {
    return LC_DDVCard_GetSignKeyVersion0(card);
  }
  else if (ddv->ddvType==1) {
    return LC_DDVCard_GetSignKeyVersion1(card);
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Unknown DDV card type (%d)", ddv->ddvType);
    return -1;
  }
}



int LC_DDVCard_GetCryptKeyVersion(LC_CARD *card){
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (ddv->ddvType==0) {
    return LC_DDVCard_GetCryptKeyVersion0(card);
  }
  else if (ddv->ddvType==1) {
    return LC_DDVCard_GetCryptKeyVersion1(card);
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Unknown DDV card type (%d)", ddv->ddvType);
    return -1;
  }
}



int LC_DDVCard_GetSignKeyNumber(LC_CARD *card){
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (ddv->ddvType==0)
    return 0;
  else
    return 2;
}



int LC_DDVCard_GetCryptKeyNumber(LC_CARD *card){
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (ddv->ddvType==0)
    return 0;
  else
    return 3;
}



LC_CLIENT_RESULT LC_DDVCard_GetChallenge(LC_CARD *card, GWEN_BUFFER *mbuf){
  LC_DDVCARD *ddv;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  dbReq=GWEN_DB_Group_new("GetChallenge");
  dbResp=GWEN_DB_Group_new("response");
  res=LC_Card_ExecCommand(card,
                          "GetChallenge",
			  dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  p=GWEN_DB_GetBinValue(dbResp,
                        "response/challenge",
                        0,
                        0, 0,
                        &bs);
  if (p && bs==8) {
    GWEN_Buffer_AppendBytes(mbuf, p, bs);
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Expected 8 bytes response, got %d bytes", bs);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return LC_Client_ResultDataError;
  }

  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return res;
}



LC_CLIENT_RESULT LC_DDVCard_CryptCharBlock(LC_CARD *card,
                                           const char *data,
                                           unsigned int dlen,
                                           GWEN_BUFFER *obuf){
  LC_DDVCARD *ddv;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (dlen!=8) {
    DBG_ERROR(LC_LOGDOMAIN,
              "In-block must exactly be 8 bytes in length (is %d)",
              dlen);
    return LC_Client_ResultDataError;
  }

  dbReq=GWEN_DB_Group_new("CryptBlock");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "in",
                      data, dlen);

  res=LC_Card_ExecCommand(card, "CryptBlock", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  p=GWEN_DB_GetBinValue(dbResp,
                        "response/out",
                        0,
                        0, 0,
                        &bs);
  if ( p && bs==8)
    GWEN_Buffer_AppendBytes(obuf, p, bs);
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Expected 8 bytes response, got %d bytes", bs);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return LC_Client_ResultDataError;
  }

  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_DDVCard_CryptBlock(LC_CARD *card,
                                       GWEN_BUFFER *ibuf,
                                       GWEN_BUFFER *obuf){
  return LC_DDVCard_CryptCharBlock(card,
                                   GWEN_Buffer_GetStart(ibuf),
                                   GWEN_Buffer_GetUsedBytes(ibuf),
                                   obuf);
}



LC_CLIENT_RESULT LC_DDVCard_SignHash0(LC_CARD *card,
                                      GWEN_BUFFER *hbuf,
                                      GWEN_BUFFER *obuf){
  LC_DDVCARD *ddv;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (GWEN_Buffer_GetUsedBytes(hbuf)!=20) {
    DBG_ERROR(LC_LOGDOMAIN, "Hash must exactly be 20 bytes in length (is %d)",
              GWEN_Buffer_GetUsedBytes(hbuf));
    return LC_Client_ResultDataError;
  }

  /* write right part of the hash */
  dbReq=GWEN_DB_Group_new("WriteHashR");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "hashR",
                      GWEN_Buffer_GetStart(hbuf)+8, 12);

  res=LC_Card_ExecCommand(card, "WriteHashR", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error while executing WriteHashR");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);

  /* write left part of the hash */
  dbReq=GWEN_DB_Group_new("WriteHashL");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "hashL",
                      GWEN_Buffer_GetStart(hbuf), 8);

  res=LC_Card_ExecCommand(card, "WriteHashL", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error while executing WriteHashL");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);


  /* retrieve signed hash */
  dbReq=GWEN_DB_Group_new("ReadSignedHash");
  dbResp=GWEN_DB_Group_new("response");
  res=LC_Card_ExecCommand(card, "ReadSignedHash", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error while executing ReadSignedHash");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  p=GWEN_DB_GetBinValue(dbResp,
                        "response/signedHash",
                        0,
                        0, 0,
                        &bs);
  if ( p && bs==8)
    GWEN_Buffer_AppendBytes(obuf, p, bs);
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Expected 8 bytes response, got %d bytes", bs);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return LC_Client_ResultDataError;
  }

  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_DDVCard_SignHash1(LC_CARD *card,
                                      GWEN_BUFFER *hbuf,
                                      GWEN_BUFFER *obuf){
  LC_DDVCARD *ddv;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (GWEN_Buffer_GetUsedBytes(hbuf)!=20) {
    DBG_ERROR(LC_LOGDOMAIN, "Hash must exactly be 20 bytes in length (is %d)",
              GWEN_Buffer_GetUsedBytes(hbuf));
    return LC_Client_ResultDataError;
  }

  /* write right part of the hash */
  dbReq=GWEN_DB_Group_new("WriteHashR");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "hashR",
                      GWEN_Buffer_GetStart(hbuf)+8, 12);

  res=LC_Card_ExecCommand(card, "WriteHashR", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error while executing WriteHashR");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);


  /* write left part of hash and retrieve signed hash */
  dbReq=GWEN_DB_Group_new("SignHash");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "hashL",
                      GWEN_Buffer_GetStart(hbuf), 8);
  res=LC_Card_ExecCommand(card, "SignHash", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "Error while executing SignHash");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }


  p=GWEN_DB_GetBinValue(dbResp,
                        "response/signedHash",
                        0,
                        0, 0,
                        &bs);
  if ( p && bs==8)
    GWEN_Buffer_AppendBytes(obuf, p, bs);
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Expected 8 bytes response, got %d bytes", bs);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return LC_Client_ResultDataError;
  }

  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_DDVCard_SignHash(LC_CARD *card,
                                     GWEN_BUFFER *hbuf,
                                     GWEN_BUFFER *obuf){
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (ddv->ddvType==0) {
    return LC_DDVCard_SignHash0(card, hbuf, obuf);
  }
  else if (ddv->ddvType==1) {
    return LC_DDVCard_SignHash1(card, hbuf, obuf);
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Unknown DDV card type (%d)", ddv->ddvType);
    return LC_Client_ResultCmdError;
  }
}



GWEN_DB_NODE *LC_DDVCard_GetCardDataAsDb(const LC_CARD *card){
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  return ddv->db_ef_id_1;
}



GWEN_BUFFER *LC_DDVCard_GetCardDataAsBuffer(const LC_CARD *card){
  LC_DDVCARD *ddv;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  return ddv->bin_ef_id_1;
}



LC_CLIENT_RESULT LC_DDVCard_ReadInstituteData(LC_CARD *card,
                                              int idx,
                                              GWEN_DB_NODE *dbData){
  LC_DDVCARD *ddv;
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbCurr;
  int i;
  unsigned int ctxCount;
  GWEN_BUFFER *buf;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  res=LC_Card_SelectEf(card, "EF_BNK");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  ctxCount=0;
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  for (i=1; i<6; i++) {
    GWEN_Buffer_Reset(buf);
    res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                              idx?idx:i, buf);
    if (res!=LC_Client_ResultOk)
      break;
    dbCurr=GWEN_DB_Group_new("context");
    GWEN_Buffer_Rewind(buf);
    if (LC_Card_ParseRecord(card, idx?idx:i, buf, dbCurr)) {
      DBG_ERROR(LC_LOGDOMAIN, "Error parsing record %d", idx?idx:i);
      GWEN_DB_Group_free(dbCurr);
    }
    else {
      const char *p1;

      p1=GWEN_DB_GetCharValue(dbCurr, "bankCode", 0, "");
      if (p1) {
	char *p2;
        char *p3;

	p2=strdup(p1);
	while ( (p3=strchr(p2, '=')) ) {
          *p3='2';
	}
	GWEN_DB_SetCharValue(dbCurr, GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "bankCode", p2);
        free(p2);
      }
      GWEN_DB_AddGroup(dbData, dbCurr);
      ctxCount++;
    }
    if (idx)
      break;
  } /* for */
  GWEN_Buffer_free(buf);

  if (!ctxCount) {
    return LC_Client_ResultDataError;
  }
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_DDVCard_WriteInstituteData(LC_CARD *card,
                                               int idx,
                                               GWEN_DB_NODE *dbData){
  LC_DDVCARD *ddv;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *buf;

  assert(card);
  ddv=GWEN_INHERIT_GETDATA(LC_CARD, LC_DDVCARD, card);
  assert(ddv);

  if (idx==0) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid index 0");
    return LC_Client_ResultInvalid;
  }

  /* select EF_BNK */
  res=LC_Card_SelectEf(card, "EF_BNK");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  /* create record data */
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  if (LC_Card_CreateRecord(card, idx, buf, dbData)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error creating record %d", idx);
    GWEN_Buffer_free(buf);
    return LC_Client_ResultDataError;
  }
  GWEN_Buffer_Rewind(buf);

  /* write record */
  res=LC_Card_IsoUpdateRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN, idx,
                              GWEN_Buffer_GetStart(buf),
                              GWEN_Buffer_GetUsedBytes(buf));
  GWEN_Buffer_free(buf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return LC_Client_ResultOk;
}

















