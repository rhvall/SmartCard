/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CLIENT_CLIENT_L_H
#define CHIPCARD_CLIENT_CLIENT_L_H

#include "client.h"
#include "card.h"

#include <gwenhywfar/msgengine.h>


LC_CLIENT_RESULT LC_Client_ExecApdu(LC_CLIENT *cl,
                                    LC_CARD *card,
                                    const char *apdu,
                                    unsigned int len,
                                    GWEN_BUFFER *rbuf,
                                    LC_CLIENT_CMDTARGET t);

LC_CLIENT_RESULT LC_Client_BuildApdu(LC_CLIENT *cl,
                                     LC_CARD *card,
                                     const char *command,
                                     GWEN_DB_NODE *cmdData,
                                     GWEN_BUFFER *gbuf);


LC_CLIENT_RESULT LC_Client_ExecCommand(LC_CLIENT *cl,
                                       LC_CARD *card,
                                       const char *commandName,
                                       GWEN_DB_NODE *cmdData,
                                       GWEN_DB_NODE *rspData);


GWEN_XMLNODE *LC_Client_FindCardCommand(LC_CLIENT *cl,
                                        LC_CARD *card,
                                        const char *commandName);

int LC_Client_AddCardTypesByAtr(LC_CLIENT *cl, LC_CARD *card);

GWEN_XMLNODE *LC_Client_GetAppNode(LC_CLIENT *cl, const char *appName);

GWEN_XMLNODE *LC_Client_GetCardNode(LC_CLIENT *cl, const char *cardName);

GWEN_MSGENGINE *LC_Client_GetMsgEngine(const LC_CLIENT *cl);

int LC_Client_InitCommon();
void LC_Client_FiniCommon();

GWEN_DB_NODE *LC_Client_GetCommonConfig();


#endif /* CHIPCARD_CLIENT_CLIENT_L_H */



