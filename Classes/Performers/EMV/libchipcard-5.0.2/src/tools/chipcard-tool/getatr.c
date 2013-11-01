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
#undef BUILDING_LIBCHIPCARD2_DLL


#include "global.h"
#include <time.h>
#include <assert.h>
#include <chipcard/client.h>
#include <gwenhywfar/debug.h>



int getAtr(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CLIENT_RESULT res;
  int timeOut;
  LC_CARD *card=NULL;

  timeOut=GWEN_DB_GetIntValue(dbArgs, "timeout", 0, CARD_TIMEOUT);

  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(0, res, "StartWait");
    return 2;
  }

  res=LC_Client_GetNextCard(cl, &card, timeOut);
  if (res!=LC_Client_ResultOk) {
    showError(0, res, "GetNextCard");
    return 2;
  }
  if (!card) {
    fprintf(stderr, "ERROR: No card found.\n");
    return 2;
  }
  fprintf(stderr, "INFO: We got this card:\n");
  LC_Card_Dump(card, 2);

  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(0, res, "ReleaseCard");
    return 2;
  }
  LC_Card_free(card);

  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(0, res, "StopWait");
    return 2;
  }

  return 0;
}


