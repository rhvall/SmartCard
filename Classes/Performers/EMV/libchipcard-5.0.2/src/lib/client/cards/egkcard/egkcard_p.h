/***************************************************************************
    begin       : Tue Feb 20 2007
    copyright   : (C) 2007-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CARD_EGKCARD_P_H
#define CHIPCARD_CARD_EGKCARD_P_H


#include <chipcard/card_imp.h>
#include <chipcard/cards/egkcard.h>


typedef struct LC_EGKCARD LC_EGKCARD;


struct LC_EGKCARD {

  LC_CARD_OPEN_FN openFn;
  LC_CARD_CLOSE_FN closeFn;
};


static void GWENHYWFAR_CB LC_EgkCard_freeData(void *bp, void *p);


static LC_CLIENT_RESULT CHIPCARD_CB LC_EgkCard_Open(LC_CARD *card);
static LC_CLIENT_RESULT CHIPCARD_CB LC_EgkCard_Close(LC_CARD *card);

static LC_CLIENT_RESULT LC_EgkCard_ReadPersonalData_old(GWEN_XMLNODE *n,
							LC_HI_PERSONAL_DATA *d);
static LC_CLIENT_RESULT LC_EgkCard_ReadPersonalData_3_0_0(GWEN_XMLNODE *n,
							  LC_HI_PERSONAL_DATA *d);
static LC_CLIENT_RESULT LC_EgkCard_ReadPersonalData_5_1_0(GWEN_XMLNODE *n,
							  LC_HI_PERSONAL_DATA *d);

static LC_CLIENT_RESULT LC_EgkCard_ReadInsuranceData_old(GWEN_XMLNODE *n,
							 LC_HI_INSURANCE_DATA *d);
static LC_CLIENT_RESULT LC_EgkCard_ReadInsuranceData_3_0_0(GWEN_XMLNODE *n,
							   LC_HI_INSURANCE_DATA *d);
static LC_CLIENT_RESULT LC_EgkCard_ReadInsuranceData_5_1_0(GWEN_XMLNODE *n,
							   LC_HI_INSURANCE_DATA *d);


#endif /* CHIPCARD_CARD_EGKCARD_P_H */

