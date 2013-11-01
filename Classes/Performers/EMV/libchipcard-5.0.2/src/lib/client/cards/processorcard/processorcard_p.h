/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CARD_PROCESSORCARD_P_H
#define CHIPCARD_CARD_PROCESSORCARD_P_H

#include <chipcard/card_imp.h>
#include <chipcard/cards/processorcard.h>


typedef struct LC_PROCESSORCARD LC_PROCESSORCARD;

struct LC_PROCESSORCARD {
  LC_CARD_OPEN_FN openFn;
  LC_CARD_CLOSE_FN closeFn;
};


void GWENHYWFAR_CB LC_ProcessorCard_freeData(void *bp, void *p);

LC_CLIENT_RESULT CHIPCARD_CB LC_ProcessorCard_Open(LC_CARD *card);
LC_CLIENT_RESULT LC_ProcessorCard_Reopen(LC_CARD *card);
LC_CLIENT_RESULT CHIPCARD_CB LC_ProcessorCard_Close(LC_CARD *card);





#endif /* CHIPCARD_CARD_PROCESSORCARD_P_H */


