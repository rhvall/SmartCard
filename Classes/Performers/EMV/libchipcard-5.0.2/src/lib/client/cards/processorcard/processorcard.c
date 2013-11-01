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


#include "processorcard_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <chipcard/chipcard.h>


GWEN_INHERIT(LC_CARD, LC_PROCESSORCARD)



int LC_ProcessorCard_ExtendCard(LC_CARD *card){
  LC_PROCESSORCARD *mc;

  GWEN_NEW_OBJECT(LC_PROCESSORCARD, mc);

  mc->openFn=LC_Card_GetOpenFn(card);
  mc->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, LC_ProcessorCard_Open);
  LC_Card_SetCloseFn(card, LC_ProcessorCard_Close);

  GWEN_INHERIT_SETDATA(LC_CARD, LC_PROCESSORCARD, card, mc,
                       LC_ProcessorCard_freeData);
  return 0;
}



int LC_ProcessorCard_UnextendCard(LC_CARD *card){
  LC_PROCESSORCARD *mc;

  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_PROCESSORCARD, card);
  assert(mc);
  LC_Card_SetOpenFn(card, mc->openFn);
  LC_Card_SetCloseFn(card, mc->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_PROCESSORCARD, card);
  return 0;
}



void GWENHYWFAR_CB LC_ProcessorCard_freeData(void *bp, void *p){
  LC_PROCESSORCARD *mc;

  assert(bp);
  assert(p);
  mc=(LC_PROCESSORCARD*)p;
  GWEN_FREE_OBJECT(mc);
}



LC_CLIENT_RESULT CHIPCARD_CB LC_ProcessorCard_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_PROCESSORCARD *mc;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening card as processor card");

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_PROCESSORCARD, card);
  assert(mc);

  res=mc->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_ProcessorCard_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    mc->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_ProcessorCard_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_PROCESSORCARD *mc;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening processor card");

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_PROCESSORCARD, card);
  assert(mc);

  DBG_DEBUG(LC_LOGDOMAIN, "Selecting processor card and app");
  res=LC_Card_SelectCard(card, "ProcessorCard");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_Card_SelectApp(card, "ProcessorCard");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_ProcessorCard_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_PROCESSORCARD *mc;

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_PROCESSORCARD, card);
  assert(mc);

  res=mc->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}



