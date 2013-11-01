/***************************************************************************
    begin       : Sat Nov 13 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "zkacard_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>
#include <chipcard/chipcard.h>
#include <chipcard/cards/processorcard.h>


GWEN_INHERIT(LC_CARD, LC_ZKACARD)



int LC_ZkaCard_ExtendCard(LC_CARD *card) {
  LC_ZKACARD *xc;
  int rv;

  rv=LC_ProcessorCard_ExtendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  GWEN_NEW_OBJECT(LC_ZKACARD, xc);
  GWEN_INHERIT_SETDATA(LC_CARD, LC_ZKACARD, card, xc,
		       LC_ZkaCard_freeData);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  xc->openFn=LC_Card_GetOpenFn(card);
  xc->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, LC_ZkaCard_Open);
  LC_Card_SetCloseFn(card, LC_ZkaCard_Close);

  return 0;
}



int LC_ZkaCard_UnextendCard(LC_CARD *card) {
  LC_ZKACARD *xc;
  int rv;

  xc=GWEN_INHERIT_GETDATA(LC_CARD, LC_ZKACARD, card);
  assert(xc);
  LC_Card_SetOpenFn(card, xc->openFn);
  LC_Card_SetCloseFn(card, xc->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_ZKACARD, card);

  rv=LC_ProcessorCard_UnextendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here");
  }
  return rv;
}



void GWENHYWFAR_CB LC_ZkaCard_freeData(void *bp, void *p){
  LC_ZKACARD *xc;

  assert(bp);
  assert(p);
  xc=(LC_ZKACARD*)p;

  GWEN_Buffer_free(xc->bin_ef_gd_0);
  GWEN_Buffer_free(xc->bin_ef_id);
  GWEN_Buffer_free(xc->bin_ef_ssd);

  GWEN_FREE_OBJECT(xc);
}



LC_CLIENT_RESULT LC_ZkaCard_Reopen(LC_CARD *card) {
  LC_CLIENT_RESULT res;
  LC_ZKACARD *xc;
  GWEN_BUFFER *mbuf;

  DBG_INFO(LC_LOGDOMAIN, "Opening ZkaCard card");

  assert(card);
  xc=GWEN_INHERIT_GETDATA(LC_CARD, LC_ZKACARD, card);
  assert(xc);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  GWEN_Buffer_free(xc->bin_ef_gd_0);
  xc->bin_ef_gd_0=NULL;

  GWEN_Buffer_free(xc->bin_ef_id);
  xc->bin_ef_id=NULL;

  GWEN_Buffer_free(xc->bin_ef_ssd);
  xc->bin_ef_ssd=NULL;

  res=LC_Card_SelectCard(card, "zkacard");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  /* first select generic app for all ZKACARD HBCI cards */
  res=LC_Card_SelectApp(card, "zkacard");
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

  /* read EF_ID */
  DBG_INFO(LC_LOGDOMAIN, "Selecting EF_ID...");
  res=LC_Card_SelectEf(card, "EF_ID");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Reading record...");
  mbuf=GWEN_Buffer_new(0, 32, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }
  xc->bin_ef_id=mbuf;


  /* read EG_GD0 */
  DBG_INFO(LC_LOGDOMAIN, "Selecting EF_GD0...");
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
  xc->bin_ef_gd_0=mbuf;


  /* select DF_SIG */
  DBG_INFO(LC_LOGDOMAIN, "Selecting DF_SIG...");
  res=LC_Card_SelectDf(card, "DF_SIG");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  /* read EG_SSD */
  DBG_INFO(LC_LOGDOMAIN, "Selecting EF_SSD...");
  res=LC_Card_SelectEf(card, "EF_SSD");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Reading data...");
  mbuf=GWEN_Buffer_new(0, 16, 0, 1);
  res=LC_Card_ReadBinary(card, 0, 65535, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    GWEN_Buffer_free(mbuf);
    return res;
  }
  xc->bin_ef_ssd=mbuf;



  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_ZkaCard_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_ZKACARD *xc;

  DBG_INFO(LC_LOGDOMAIN, "Opening card as ZkaCard card");

  assert(card);
  xc=GWEN_INHERIT_GETDATA(LC_CARD, LC_ZKACARD, card);
  assert(xc);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  GWEN_Buffer_free(xc->bin_ef_gd_0);
  xc->bin_ef_gd_0=NULL;

  GWEN_Buffer_free(xc->bin_ef_id);
  xc->bin_ef_id=NULL;

  GWEN_Buffer_free(xc->bin_ef_ssd);
  xc->bin_ef_ssd=NULL;

  res=xc->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_ZkaCard_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    xc->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_ZkaCard_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_ZKACARD *xc;

  assert(card);
  xc=GWEN_INHERIT_GETDATA(LC_CARD, LC_ZKACARD, card);
  assert(xc);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=xc->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}




