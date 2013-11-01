/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD2_MSGENGINE_P_H
#define CHIPCARD2_MSGENGINE_P_H


#include "msgengine_l.h"

#define LC_KVK_UMLAUT_AE 0x5b
#define LC_KVK_UMLAUT_OE 0x5c
#define LC_KVK_UMLAUT_UE 0x5d
#define LC_KVK_UMLAUT_ae 0x7b
#define LC_KVK_UMLAUT_oe 0x7c
#define LC_KVK_UMLAUT_ue 0x7d
#define LC_KVK_UMLAUT_ss 0x7e


typedef struct LC_MSGENGINE LC_MSGENGINE;

struct LC_MSGENGINE {
  int dummy;
};



static
void GWENHYWFAR_CB LC_MsgEngine_FreeData(void *bp, void *p);

static
int LC_MsgEngine_TypeRead(GWEN_MSGENGINE *e,
                          GWEN_BUFFER *msgbuf,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFER *vbuf,
                          char escapeChar,
                          const char *delimiters);

static
int LC_MsgEngine_TypeWrite(GWEN_MSGENGINE *e,
                           GWEN_BUFFER *gbuf,
                           GWEN_BUFFER *data,
                           GWEN_XMLNODE *node);

static
GWEN_DB_NODE_TYPE LC_MsgEngine_TypeCheck(GWEN_MSGENGINE *e,
					 const char *tname);


static
const char *LC_MsgEngine_GetCharValue(GWEN_MSGENGINE *e,
                                      const char *name,
                                      const char *defValue);

static
int LC_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                             const char *name,
                             int defValue);


static
int LC_MsgEngine_BinTypeRead(GWEN_MSGENGINE *e,
                             GWEN_XMLNODE *node,
                             GWEN_DB_NODE *gr,
                             GWEN_BUFFER *vbuf);

static
int LC_MsgEngine_BinTypeWrite(GWEN_MSGENGINE *e,
                              GWEN_XMLNODE *node,
                              GWEN_DB_NODE *gr,
                              GWEN_BUFFER *dbuf);


static
uint32_t LC_MsgEngine__FromBCD(uint32_t value);

static
uint32_t LC_MsgEngine__ToBCD(uint32_t value);



#endif /* CHIPCARD2_MSGENGINE_P_H */


