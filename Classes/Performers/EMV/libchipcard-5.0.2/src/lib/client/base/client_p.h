/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CLIENT_CLIENT_P_H
#define CHIPCARD_CLIENT_CLIENT_P_H

#include "client_l.h"

#include <gwenhywfar/msgengine.h>

#include <winscard.h>
#include <wintypes.h>


#define LCC_PM_LIBNAME    "libchipcard"
#define LCC_PM_SYSCONFDIR "sysconfdir"
#define LCC_PM_DATADIR    "datadir"


#define MAX_READERS 32



struct LC_CLIENT {
  GWEN_INHERIT_ELEMENT(LC_CLIENT)
  char *programName;
  char *programVersion;

  GWEN_DB_NODE *dbConfig;

  GWEN_MSGENGINE *msgEngine;
  GWEN_XMLNODE *cardNodes;
  GWEN_XMLNODE *appNodes;

  SCARDCONTEXT scardContext;

  int pnpAvailable;
  SCARD_READERSTATE readerStates[MAX_READERS];
  int readerCount;
  int lastUsedReader;
  LPSTR readerList;
};


static int LC_Client_GetReaderAndDriverType(const LC_CLIENT *cl,
					    const char *readerName,
					    GWEN_BUFFER *driverType,
					    GWEN_BUFFER *readerType,
					    uint32_t *pReaderFlags);


static void LC_Client__SampleXmlFiles(const char *where,
                                      GWEN_STRINGLIST *sl);

static int LC_Client_MergeXMLDefs(GWEN_XMLNODE *destNode,
                                  GWEN_XMLNODE *node);

static int LC_Client_ReadXmlFiles(GWEN_XMLNODE *root,
                                  const char *basedir,
                                  const char *tPlural,
                                  const char *tSingular);

static GWEN_XMLNODE *LC_Client__FindCommandInCardNode(GWEN_XMLNODE *node,
                                                      const char *commandName,
                                                      const char *driverType,
                                                      const char *readerType);
static GWEN_XMLNODE*
  LC_Client_FindCommandInCardNode(GWEN_XMLNODE *node,
                                  const char *commandName,
                                  const char *driverType,
                                  const char *readerType);
static GWEN_XMLNODE*
  LC_Client_FindCommandInCardFamily(GWEN_XMLNODE *cardNodes,
                                    GWEN_STRINGLIST *handled,
                                    const char *cardType,
                                    const char *commandName,
                                    const char *driverType,
                                    const char *readerType);
static GWEN_XMLNODE*
  LC_Client_FindCommandInCardTypes(GWEN_XMLNODE *cardNodes,
                                   const GWEN_STRINGLIST *cardTypes,
                                   const char *commandName,
                                   const char *driverType,
                                   const char *readerType);


static GWEN_XMLNODE *LC_Client_FindResultInNode(GWEN_XMLNODE *node,
                                                int sw1, int sw2);

static GWEN_XMLNODE *LC_Client_FindResult(LC_CLIENT *cl,
                                          GWEN_XMLNODE *cmdNode,
                                          int sw1, int sw2);


static GWEN_XMLNODE *LC_Client_FindResponseInNode(GWEN_XMLNODE *cmd,
                                                  const char *typ);

static GWEN_XMLNODE *LC_Client_FindResponse(LC_CLIENT *cl,
                                            GWEN_XMLNODE *cmdNode,
                                            const char *typ);



static LC_CLIENT_RESULT LC_Client__BuildApdu(LC_CLIENT *cl,
					     GWEN_XMLNODE *node,
					     GWEN_DB_NODE *cmdData,
					     GWEN_BUFFER *gbuf);

static int LC_Client_ParseResult(LC_CLIENT *cl,
                                 GWEN_XMLNODE *node,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_DB_NODE *rspData);

static int LC_Client_ParseResponse(LC_CLIENT *cl,
                                   GWEN_XMLNODE *node,
                                   GWEN_BUFFER *gbuf,
                                   GWEN_DB_NODE *rspData);

static int LC_Client_ParseAnswer(LC_CLIENT *cl,
                                 GWEN_XMLNODE *node,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_DB_NODE *rspData);



static int LC_Client_FindReaderState(LC_CLIENT *cl, const char *readerName);


static LC_CLIENT_RESULT LC_Client_ConnectCard(LC_CLIENT *cl,
					      const char *readerName,
					      LC_CARD **pCard);


#endif /* CHIPCARD_CLIENT_CLIENT_P_H */



