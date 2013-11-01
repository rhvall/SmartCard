/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CARD_DDVCARD_P_H
#define CHIPCARD_CARD_DDVCARD_P_H


#include <chipcard/card_imp.h>
#include <chipcard/cards/ddvcard.h>


typedef struct LC_DDVCARD LC_DDVCARD;


struct LC_DDVCARD {
  int ddvType;
  GWEN_BUFFER *bin_ef_id_1;
  GWEN_DB_NODE *db_ef_id_1;

  LC_CARD_OPEN_FN openFn;
  LC_CARD_CLOSE_FN closeFn;
};


void GWENHYWFAR_CB LC_DDVCard_freeData(void *bp, void *p);


LC_CLIENT_RESULT CHIPCARD_CB LC_DDVCard_Open(LC_CARD *card);
LC_CLIENT_RESULT CHIPCARD_CB LC_DDVCard_Close(LC_CARD *card);

LC_CLIENT_RESULT LC_DDVCard_SignHash0(LC_CARD *card,
                                      GWEN_BUFFER *hbuf,
                                      GWEN_BUFFER *obuf);
LC_CLIENT_RESULT LC_DDVCard_SignHash1(LC_CARD *card,
                                      GWEN_BUFFER *hbuf,
                                      GWEN_BUFFER *obuf);

int LC_DDVCard_GetCryptKeyVersion0(LC_CARD *card);
int LC_DDVCard_GetSignKeyVersion0(LC_CARD *card);
int LC_DDVCard_GetKeyVersion1(LC_CARD *card, int keyNumber);
int LC_DDVCard_GetSignKeyVersion1(LC_CARD *card);
int LC_DDVCard_GetCryptKeyVersion1(LC_CARD *card);



#endif /* CHIPCARD_CARD_DDVCARD_P_H */

