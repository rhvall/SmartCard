/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CARD_STARCOS_P_H
#define CHIPCARD_CARD_STARCOS_P_H

#include <chipcard/card_imp.h>
#include "starcos.h"
#include "starcos_keydescr_l.h"

typedef struct LC_STARCOS LC_STARCOS;



typedef enum {
  LC_Card_StarcosSecState_None=0,
  LC_Card_StarcosSecState_Sign,
  LC_Card_StarcosSecState_Crypt
} LC_CARD_STARCOS_SECSTATE;


struct LC_STARCOS {
  LC_CARD_OPEN_FN openFn;
  LC_CARD_CLOSE_FN closeFn;

  GWEN_BUFFER *bin_ef_gd_0;
  GWEN_DB_NODE *db_ef_gd_0;
  unsigned char initialPin[5];

  char *appName;

  /*1st byte of EF_KEY_LOG */
  unsigned int keyLogInfo;
  LC_STARCOS_KEYDESCR_LIST *keyDescriptors;

  /* for "manage security env" */
  LC_CARD_STARCOS_SECSTATE securityState;
  unsigned int currentPubKeyId;
  unsigned int currentPrivateKeyId;
  unsigned int currentAlgo;

};



void GWENHYWFAR_CB LC_Starcos_freeData(void *bp, void *p);
LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos_Open(LC_CARD *card);
LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos_Close(LC_CARD *card);

LC_CLIENT_RESULT LC_Starcos__ReadEfToDb(LC_CARD *card,
                                        const char *efName,
                                        const char *formatName,
                                        GWEN_DB_NODE *db);


int LC_Starcos__FindKeyDescrOffset(int kid);
LC_CLIENT_RESULT LC_Starcos__LoadKeyDescr(LC_CARD *card, int kid,
					  LC_STARCOS_KEYDESCR **pDescr);

LC_CLIENT_RESULT LC_Starcos__GetKeyLogInfo(LC_CARD *card,
					   unsigned int *pResult);
LC_CLIENT_RESULT LC_Starcos__SaveKeyLogInfo(LC_CARD *card);

int LC_Starcos__IsSignKey(int kid);
int LC_Starcos__IsCryptKey(int kid);

int LC_Starcos__GetIpfKeyOffset(LC_CARD *card, int kid);


LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos__Sign(LC_CARD *card,
                                              const char *ptr,
                                              unsigned int size,
                                              GWEN_BUFFER *sigBuf);
LC_CLIENT_RESULT CHIPCARD_CB LC_Starcos__Verify(LC_CARD *card,
                                                const char *ptr,
                                                unsigned int size,
                                                const char *sigptr,
                                                unsigned int sigsize);

LC_CLIENT_RESULT CHIPCARD_CB
  LC_Starcos_GetInitialPin(LC_CARD *card,
                           int pid,
                           unsigned char *buffer,
                           unsigned int maxSize,
                           unsigned int *pinLength);

LC_CLIENT_RESULT CHIPCARD_CB
  LC_Starcos_GetPinStatus(LC_CARD *card,
                          unsigned int pid,
                          int *maxErrors,
                          int *currentErrors);



#endif /* CHIPCARD_CARD_STARCOS_P_H */

