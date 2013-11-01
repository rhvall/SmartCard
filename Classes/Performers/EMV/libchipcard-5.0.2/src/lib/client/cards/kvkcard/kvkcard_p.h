/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CARD_KVKCARD_P_H
#define CHIPCARD_CARD_KVKCARD_P_H

#include <chipcard/card_imp.h>
#include <chipcard/cards/kvkcard.h>


typedef struct LC_KVKCARD LC_KVKCARD;

struct LC_KVKCARD {
  GWEN_DB_NODE *dbData;
  LC_CARD_OPEN_FN openFn;
  LC_CARD_CLOSE_FN closeFn;

};


void GWENHYWFAR_CB LC_KVKCard_freeData(void *bp, void *p);


LC_CLIENT_RESULT CHIPCARD_CB LC_KVKCard_Open(LC_CARD *card);
LC_CLIENT_RESULT CHIPCARD_CB LC_KVKCard_Close(LC_CARD *card);


LC_CLIENT_RESULT LC_KVKCard_ReadCardData(LC_CARD *card);



#endif /* CHIPCARD_CARD_KVKCARD_P_H */




