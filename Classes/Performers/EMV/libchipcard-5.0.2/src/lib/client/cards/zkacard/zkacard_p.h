/***************************************************************************
    begin       : Sat Nov 13 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CARD_ZKACARD_P_H
#define CHIPCARD_CARD_ZKACARD_P_H

#include "zkacard.h"

#include <chipcard/card_imp.h>


typedef struct LC_ZKACARD LC_ZKACARD;
struct LC_ZKACARD {
  LC_CARD_OPEN_FN openFn;
  LC_CARD_CLOSE_FN closeFn;

  GWEN_BUFFER *bin_ef_gd_0;
  GWEN_BUFFER *bin_ef_id;
  GWEN_BUFFER *bin_ef_ssd;

  int len_modus_sk_ch_ds;
  int len_modus_sk_ch_aut;
  int len_modus_sk_ch_ke;
  int min_len_csa_password;

};


void GWENHYWFAR_CB LC_ZkaCard_freeData(void *bp, void *p);
LC_CLIENT_RESULT CHIPCARD_CB LC_ZkaCard_Open(LC_CARD *card);
LC_CLIENT_RESULT CHIPCARD_CB LC_ZkaCard_Close(LC_CARD *card);




#endif

