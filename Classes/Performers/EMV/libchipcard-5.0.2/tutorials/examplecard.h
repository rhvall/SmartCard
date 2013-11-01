/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/** @file examplecard.h
 *  @short Pubic header file
 *
 * This file may be included by whoever wants to. It defines the public
 * functions of this type.
 */


#ifndef CHIPCARD_CARD_EXAMPLECARD_H
#define CHIPCARD_CARD_EXAMPLECARD_H

#include <chipcard/card.h>


int ExampleCard_ExtendCard(LC_CARD *card);
int ExampleCard_UnextendCard(LC_CARD *card);

LC_CLIENT_RESULT ExampleCard_Reopen(LC_CARD *card);


int ExampleCard_GetExampleData(const LC_CARD *card);


typedef struct TYPE_REAL TYPE_VISIBLE;
int test_type(TYPE_VISIBLE *v);

#endif /* CHIPCARD_CARD_EXAMPLECARD_P_H */




