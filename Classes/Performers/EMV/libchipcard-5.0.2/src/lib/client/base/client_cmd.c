

int LC_Client_AddCardTypesByAtr(LC_CLIENT *cl, LC_CARD *card){
  GWEN_XMLNODE *cardNode;
  const unsigned char *atr;
  unsigned int atrLen;
  GWEN_BUFFER *hexAtr;
  int types=0;
  int done;

  DBG_DEBUG(0, "Adding card types...");

  /* get ATR, convert it to hex */
  atrLen=LC_Card_GetAtr(card, &atr);
  if (atr==0 || atrLen==0) {
    DBG_INFO(0, "No ATR");
    return 1;
  }
  hexAtr=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Text_ToHexBuffer((const char*)atr, atrLen, hexAtr, 0, 0, 0)) {
    DBG_ERROR(LC_LOGDOMAIN, "Internal error");
    abort();
  }

  cardNode=GWEN_XMLNode_FindFirstTag(cl->cardNodes, "card", 0, 0);
  if (!cardNode) {
    DBG_ERROR(LC_LOGDOMAIN, "No card nodes.");
    return -1;
  }
  while(cardNode) {
    const char *name;
    const char *tp;
    const char *xtp;
    int sameBaseType=0;

    name=GWEN_XMLNode_GetProperty(cardNode, "name", 0);
    assert(name);
    tp=GWEN_XMLNode_GetProperty(cardNode, "type", 0);

    DBG_VERBOUS(LC_LOGDOMAIN, "Checking card \"%s\"", name);
    xtp=LC_Card_GetCardType(card);
    sameBaseType=(tp && xtp && strcasecmp(tp, xtp)==0);
    if (sameBaseType) {
      GWEN_XMLNODE *nAtrs;

      nAtrs=GWEN_XMLNode_FindFirstTag(cardNode, "cardinfo", 0, 0);
      if (nAtrs)
        nAtrs=GWEN_XMLNode_FindFirstTag(nAtrs, "atrs", 0, 0);
      if (nAtrs) {
        GWEN_XMLNODE *nAtr;
  
        nAtr=GWEN_XMLNode_GetFirstTag(nAtrs);
        while(nAtr) {
          GWEN_XMLNODE *nData;
  
          nData=GWEN_XMLNode_GetFirstData(nAtr);
          if (nData) {
            const char *p;
  
            p=GWEN_XMLNode_GetData(nData);
            if (p) {
              GWEN_BUFFER *dbuf;

              /* compress ATR from XML file */
              dbuf=GWEN_Buffer_new(0, 256, 0, 1);
              while(*p) {
                if (!isspace(*p))
                  GWEN_Buffer_AppendByte(dbuf, *p);
                p++;
              } /* while */
              if (-1!=GWEN_Text_ComparePattern(GWEN_Buffer_GetStart(hexAtr),
                                               GWEN_Buffer_GetStart(dbuf),
                                               0)) {
                DBG_DEBUG(LC_LOGDOMAIN, "Card \"%s\" matches ATR", name);
                if (LC_Card_AddCardType(card, name)) {
                  DBG_INFO(LC_LOGDOMAIN, "Added card type \"%s\"", name);
                  types++;
                }
              }
              GWEN_Buffer_free(dbuf);
            } /* if data */
          } /* if data node */
          nAtr=GWEN_XMLNode_GetNextTag(nAtr);
        } /* while */
      } /* if atrs */
    } /* if sameBaseType */
    cardNode=GWEN_XMLNode_FindNextTag(cardNode, "card", 0, 0);
  } /* while */
  GWEN_Buffer_free(hexAtr);

  /* add all cards whose base types are contained in the list.
   * repeat this as long as we added cards */
  done=0;
  while(!done) {
    done=1;
    cardNode=GWEN_XMLNode_FindFirstTag(cl->cardNodes, "card", 0, 0);
    while(cardNode) {
      const char *name;
      const char *extends;

      name=GWEN_XMLNode_GetProperty(cardNode, "name", 0);
      assert(name);
      extends=GWEN_XMLNode_GetProperty(cardNode, "extends", 0);
      if (extends) {
        if (GWEN_StringList_HasString(LC_Card_GetCardTypes(card), extends)) {
          if (LC_Card_AddCardType(card, name)) {
            DBG_INFO(LC_LOGDOMAIN, "Added card type \"%s\"", name);
            types++;
            done=0;
          }
        }
      }
      cardNode=GWEN_XMLNode_FindNextTag(cardNode, "card", 0, 0);
    }
  } /* while */

  return (types!=0)?0:1;
}



GWEN_XMLNODE *LC_Client__FindCommandInCardNode(GWEN_XMLNODE *node,
                                               const char *commandName,
                                               const char *driverType,
                                               const char *readerType){
  GWEN_XMLNODE *cmds;
  GWEN_XMLNODE *n;

  DBG_INFO(LC_LOGDOMAIN,
           "Searching in \"%s\" (%s/%s)",
           GWEN_XMLNode_GetProperty(node, "name", "(noname)"),
           driverType?driverType:"(none)",
           readerType?readerType:"(none)");

  cmds=GWEN_XMLNode_FindNode(node,
                             GWEN_XMLNodeTypeTag,
                             "commands");
  if (!cmds) {
    DBG_INFO(LC_LOGDOMAIN, "No commands in card data");
    return 0;
  }

  /* first try exact match */
  if (driverType && readerType) {
    DBG_DEBUG(LC_LOGDOMAIN, "Searching for %s/%s/%s",
              driverType, readerType, commandName);
    n=GWEN_XMLNode_FindFirstTag(cmds,
                                "command",
                                "name",
                                commandName);
    while(n) {
      if (strcasecmp(GWEN_XMLNode_GetProperty(n, "driver", ""),
                     driverType)==0 &&
          strcasecmp(GWEN_XMLNode_GetProperty(n, "reader", ""),
                     readerType)==0) {
        DBG_DEBUG(LC_LOGDOMAIN,
                  "Found command in %s/%s", driverType, readerType);
        return n;
      }
      n=GWEN_XMLNode_FindNextTag(n, "command", "name", commandName);
    } /* while */
  }

  if (driverType) {
    /* try match of driver only */
    DBG_DEBUG(LC_LOGDOMAIN, "Searching for %s/%s",
              driverType, commandName);
    n=GWEN_XMLNode_FindFirstTag(cmds,
                                "command",
                                "name",
                                commandName);
    while(n) {
      if (strcasecmp(GWEN_XMLNode_GetProperty(n, "driver", ""),
                     driverType)==0) {
        DBG_DEBUG(LC_LOGDOMAIN, "Found command in %s", driverType);
        return n;
      }
      n=GWEN_XMLNode_FindNextTag(n, "command", "name", commandName);
    } /* while */
  }

  /* try match of command name only */
  DBG_DEBUG(LC_LOGDOMAIN, "Searching for %s", commandName);
  n=GWEN_XMLNode_FindFirstTag(cmds,
                              "command",
                              "name",
                              commandName);
  while(n) {
    if (!GWEN_XMLNode_GetProperty(n, "driver", 0))
      return n;
    n=GWEN_XMLNode_FindNextTag(n, "command", "name", commandName);
  } /* while */

  return n;
}



GWEN_XMLNODE *LC_Client_FindCommandInCardNode(GWEN_XMLNODE *node,
                                              const char *commandName,
                                              const char *driverType,
                                              const char *readerType) {
  GWEN_XMLNODE *n;

  n=LC_Client__FindCommandInCardNode(node, commandName,
                                     driverType, readerType);
  if (n==0)
    n=LC_Client__FindCommandInCardNode(node, commandName,
                                       driverType, 0);
  if (n==0)
    n=LC_Client__FindCommandInCardNode(node, commandName,
                                       0, 0);

  return n;
}



GWEN_XMLNODE *LC_Client_FindCommandInCardFamily(GWEN_XMLNODE *cardNodes,
                                                GWEN_STRINGLIST *handled,
                                                const char *cardType,
                                                const char *commandName,
                                                const char *driverType,
                                                const char *readerType){
  GWEN_XMLNODE *node;

  DBG_DEBUG(LC_LOGDOMAIN, "Searching in family of \"%s\"", cardType);
  node=GWEN_XMLNode_FindFirstTag(cardNodes, "card", "name", cardType);
  if (node) {
    while(node) {
      GWEN_XMLNODE *n;
      const char *parent;

      cardType=GWEN_XMLNode_GetProperty(node, "name", 0);
      assert(cardType);
      DBG_VERBOUS(LC_LOGDOMAIN, "Searching in \"%s\" (%s/%s)",
                  GWEN_XMLNode_GetProperty(node, "name", "(noname)"),
                  driverType?driverType:"(none)",
                  readerType?readerType:"(none)");
      if (!GWEN_StringList_HasString(handled, cardType)) {
        n=LC_Client_FindCommandInCardNode(node, commandName,
                                          driverType, readerType);
        GWEN_StringList_AppendString(handled, cardType, 0, 1);
        if (n) {
          return n;
        }
      }
      else {
        DBG_INFO(LC_LOGDOMAIN, "Card type \"%s\" already handled",
                 cardType);
      }

      /* search in parents */
      parent=GWEN_XMLNode_GetProperty(node, "extends", 0);
      if (!parent) {
        DBG_VERBOUS(LC_LOGDOMAIN, "Card type \"%s\" has no parent",
                    GWEN_XMLNode_GetProperty(node, "name", "(noname)"));
        break;
      }
      DBG_DEBUG(LC_LOGDOMAIN, "Searching for extended card \"%s\"", parent);
      node=GWEN_XMLNode_FindFirstTag(cardNodes,
                                     "card",
                                     "name",
                                     parent);
      if (!node) {
        DBG_WARN(LC_LOGDOMAIN,
                 "Extended card \"%s\" not found",
                 parent);
        break;
      }
      DBG_DEBUG(LC_LOGDOMAIN, "Searching in parent \"%s\"", parent);
    } /* while */
  }
  else {
    DBG_INFO(LC_LOGDOMAIN, "Card \"%s\" not found", cardType);
  }
  DBG_DEBUG(0, "Command \"%s\" not found", commandName);
  return 0;
}



GWEN_XMLNODE*
LC_Client_FindCommandInCardTypes(GWEN_XMLNODE *cardNodes,
                                 const GWEN_STRINGLIST *cardTypes,
                                 const char *commandName,
                                 const char *driverType,
                                 const char *readerType){
  GWEN_STRINGLIST *handled;
  GWEN_STRINGLISTENTRY *se;
  GWEN_XMLNODE *node=0;

  handled=GWEN_StringList_new();
  se=GWEN_StringList_FirstEntry(cardTypes);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    assert(s);

    DBG_INFO(LC_LOGDOMAIN,
             "Searching in card type \"%s\"", s);
    node=LC_Client_FindCommandInCardFamily(cardNodes,
                                           handled,
                                           s,
                                           commandName,
                                           driverType,
                                           readerType);
    if (node)
      break;
    se=GWEN_StringListEntry_Next(se);
  } /* while */

  return node;
}



GWEN_XMLNODE *LC_Client_FindCardCommand(LC_CLIENT *cl,
                                        LC_CARD *card,
                                        const char *commandName) {
  GWEN_XMLNODE *n;

  n=LC_Card_GetCardNode(card);
  if (n) {
    const char *cardName;
    GWEN_STRINGLIST *sl;

    cardName=GWEN_XMLNode_GetProperty(n, "name", 0);
    assert(cardName);
    DBG_INFO(LC_LOGDOMAIN, "Preselected card type \"%s\"", cardName);

    sl=GWEN_StringList_new();
    n=LC_Client_FindCommandInCardFamily(cl->cardNodes,
                                        sl,
                                        cardName,
                                        commandName,
                                        LC_Card_GetDriverType(card),
                                        LC_Card_GetReaderType(card));
    GWEN_StringList_free(sl);
    if (n)
      return n;
    return 0;
  }
  return LC_Client_FindCommandInCardTypes(cl->cardNodes,
                                          LC_Card_GetCardTypes(card),
                                          commandName,
                                          LC_Card_GetDriverType(card),
                                          LC_Card_GetReaderType(card));
}



GWEN_XMLNODE *LC_Client_FindResultInNode(GWEN_XMLNODE *node,
                                         int sw1, int sw2) {
  GWEN_XMLNODE *rnode;
  GWEN_XMLNODE *n;
  int lsw1, lsw2;

  DBG_DEBUG(0, "Searching for result type of %02x/%02x", sw1, sw2);
  while(node) {
    rnode=GWEN_XMLNode_FindNode(node,
                                GWEN_XMLNodeTypeTag,
                                "results");
    if (rnode) {
      /* first try exact match */
      n=GWEN_XMLNode_GetFirstTag(rnode);
      while(n) {
        if (1==sscanf(GWEN_XMLNode_GetProperty(n, "sw1", "-1"),
                      "%i", &lsw1) &&
            1==sscanf(GWEN_XMLNode_GetProperty(n, "sw2", "-1"),
                      "%i", &lsw2)) {
          DBG_VERBOUS(0, "Checking %02x/%02x", lsw1, lsw2);
          if (lsw1==sw1 && lsw2==sw2) {
            return n;
          }
        }
        else {
          DBG_WARN(0, "Bad SW1 or SW2 value");
        }
        n=GWEN_XMLNode_GetNextTag(n);
      } /* while */

      /* try SW1 only */
      n=GWEN_XMLNode_GetFirstTag(rnode);
      while(n) {
        if (1==sscanf(GWEN_XMLNode_GetProperty(n, "sw1", "-1"),
                      "%i", &lsw1) &&
            1==sscanf(GWEN_XMLNode_GetProperty(n, "sw2", "-1"),
                      "%i", &lsw2)) {
          if (lsw1==sw1 && lsw2==-1) {
            return n;
          }
        }
        else {
          DBG_WARN(0, "Bad SW1 or SW2 value");
        }
        n=GWEN_XMLNode_GetNextTag(n);
      } /* while */
    } /* if rnode */

    /* select parent */
    node=GWEN_XMLNode_GetParent(node);
  }

  return 0;
}



GWEN_XMLNODE *LC_Client_FindResult(LC_CLIENT *cl,
                                   GWEN_XMLNODE *cmdNode,
                                   int sw1, int sw2) {
  GWEN_XMLNODE *tmpNode;
  GWEN_XMLNODE *rnode;

  /* first find result in command node */
  rnode=LC_Client_FindResultInNode(cmdNode, sw1, sw2);
  if (rnode)
    return rnode;
  rnode=LC_Client_FindResultInNode(cmdNode, -1, -1);
  if (rnode)
    return rnode;

  /* try in node <commands> */
  tmpNode=GWEN_XMLNode_GetParent(cmdNode);
  if (!tmpNode)
    return 0;
  rnode=LC_Client_FindResultInNode(tmpNode, sw1, sw2);
  if (rnode)
    return rnode;
  rnode=LC_Client_FindResultInNode(tmpNode, -1, -1);
  if (rnode)
    return rnode;

  /* try in current card node */
  tmpNode=GWEN_XMLNode_GetParent(tmpNode);
  if (!tmpNode)
    return 0;
  rnode=LC_Client_FindResultInNode(tmpNode, sw1, sw2);
  if (rnode)
    return rnode;
  rnode=LC_Client_FindResultInNode(tmpNode, -1, -1);
  if (rnode)
    return rnode;

  /* try in parents */
  for(;;) {
    const char *parent;

    parent=GWEN_XMLNode_GetProperty(tmpNode, "extends", 0);
    if (!parent) {
      break;
    }
    tmpNode=GWEN_XMLNode_FindFirstTag(cl->cardNodes,
                                      "card",
                                      "name",
                                      parent);
    if (!tmpNode)
      break;

    rnode=LC_Client_FindResultInNode(tmpNode, sw1, sw2);
    if (rnode) {
      break;
    }
    rnode=LC_Client_FindResultInNode(tmpNode, -1, -1);
    if (rnode) {
      break;
    }
  } /* for */

  return rnode;
}



GWEN_XMLNODE *LC_Client_FindResponseInNode(GWEN_XMLNODE *cmd,
                                           const char *typ) {
  GWEN_XMLNODE *rnode;
  GWEN_XMLNODE *n;
  const char *ltyp;

  DBG_DEBUG(0, "Searching for response type \"%s\"", typ);
  rnode=GWEN_XMLNode_FindNode(cmd,
                              GWEN_XMLNodeTypeTag,
                              "responses");
  if (!rnode) {
    DBG_DEBUG(0, "No <responses> tag in command definition");
    return 0;
  }

  /* first try exact match */
  n=GWEN_XMLNode_GetFirstTag(rnode);
  while(n) {
    ltyp=GWEN_XMLNode_GetProperty(n, "type", 0);
    if (ltyp) {
      if (strcasecmp(ltyp, typ)==0)
        return n;
    }
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */

  /* then try a response without any type */
  n=GWEN_XMLNode_GetFirstTag(rnode);
  while(n) {
    ltyp=GWEN_XMLNode_GetProperty(n, "type", 0);
    if (!ltyp)
      return n;
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */

  return 0;
}



GWEN_XMLNODE *LC_Client_FindResponse(LC_CLIENT *cl,
                                     GWEN_XMLNODE *cmdNode,
                                     const char *typ) {
  GWEN_XMLNODE *tmpNode;
  GWEN_XMLNODE *rnode;

  /* first find response in command node */
  rnode=LC_Client_FindResponseInNode(cmdNode, typ);
  if (rnode)
    return rnode;

  /* try in node <commands> */
  tmpNode=GWEN_XMLNode_GetParent(cmdNode);
  if (!tmpNode)
    return 0;
  rnode=LC_Client_FindResponseInNode(tmpNode, typ);
  if (rnode)
    return rnode;

  /* try in current card node */
  tmpNode=GWEN_XMLNode_GetParent(tmpNode);
  if (!tmpNode)
    return 0;
  rnode=LC_Client_FindResponseInNode(tmpNode, typ);
  if (rnode)
    return rnode;

  /* try in parents */
  for(;;) {
    const char *parent;

    parent=GWEN_XMLNode_GetProperty(tmpNode, "extends", 0);
    if (!parent) {
      break;
    }
    tmpNode=GWEN_XMLNode_FindFirstTag(cl->cardNodes,
                                      "card",
                                      "name",
                                      parent);
    if (!tmpNode)
      break;

    rnode=LC_Client_FindResponseInNode(tmpNode, typ);
    if (rnode) {
      break;
    }
  } /* for */

  return rnode;
}



LC_CLIENT_RESULT LC_Client__BuildApdu(LC_CLIENT *cl,
				      GWEN_XMLNODE *node,
				      GWEN_DB_NODE *cmdData,
				      GWEN_BUFFER *gbuf) {
  GWEN_XMLNODE *sendNode;
  GWEN_XMLNODE *dataNode;
  GWEN_XMLNODE *apduNode;
  GWEN_BUFFER *dataBuffer;
  unsigned int i;
  int j;

  assert(cl);

  sendNode=GWEN_XMLNode_FindNode(node, GWEN_XMLNodeTypeTag, "send");
  if (!sendNode) {
    DBG_INFO(LC_LOGDOMAIN,
	     "No <send> tag in command definition, do not execute");
    return LC_Client_ResultDontExecute;
  }

  apduNode=GWEN_XMLNode_FindNode(sendNode,
                                 GWEN_XMLNodeTypeTag, "apdu");
  if (!apduNode) {
    DBG_ERROR(LC_LOGDOMAIN, "No <apdu> tag in command definition");
    abort();
  }

  dataBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  dataNode=GWEN_XMLNode_FindNode(sendNode,
                                 GWEN_XMLNodeTypeTag, "data");
  if (dataNode) {
    /* there is a data node, sample data */
    if (GWEN_MsgEngine_CreateMessageFromNode(cl->msgEngine,
                                             dataNode,
                                             dataBuffer,
                                             cmdData)) {
      DBG_ERROR(LC_LOGDOMAIN, "Error creating data for APDU");
      GWEN_Buffer_free(dataBuffer);
      GWEN_Buffer_AppendString(gbuf, "Error creating APDU data from command");
      return -1;
    }
  }

  if (GWEN_MsgEngine_CreateMessageFromNode(cl->msgEngine,
                                           apduNode,
                                           gbuf,
                                           cmdData)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error creating APDU");
    GWEN_Buffer_free(dataBuffer);
    GWEN_Buffer_AppendString(gbuf, "Error creating APDU from command");
    return -1;
  }

  i=GWEN_Buffer_GetUsedBytes(dataBuffer);
  if (i) {
    GWEN_Buffer_AppendByte(gbuf, (unsigned char)i);
    GWEN_Buffer_AppendBuffer(gbuf, dataBuffer);
  }
  GWEN_Buffer_free(dataBuffer);

  j=0;
  if (1!=sscanf(GWEN_XMLNode_GetProperty(apduNode, "lr", "0"),
                "%i", &j))
    j=0;

  if (j!=-1) {
    j=GWEN_DB_GetIntValue(cmdData, "lr", 0, -1);
    if (j==-1) {
      if (1!=sscanf(GWEN_XMLNode_GetProperty(apduNode, "lr", "-1"),
                    "%i", &j))
        j=-1;
    }
  }
  if (j>=0)
    GWEN_Buffer_AppendByte(gbuf, (unsigned char)j);

  return 0;
}



int LC_Client_ParseResult(LC_CLIENT *cl,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFER *gbuf,
                          GWEN_DB_NODE *rspData){
  unsigned int i;
  int sw1, sw2;
  GWEN_DB_NODE *dbTmp;
  GWEN_XMLNODE *rnode;

  GWEN_Buffer_Rewind(gbuf); /* just in case ... */
  i=GWEN_Buffer_GetUsedBytes(gbuf);
  if (i<2) {
    DBG_ERROR(LC_LOGDOMAIN, "Answer too small (less than 2 bytes)");
    return -1;
  }
  sw1=(unsigned char)(GWEN_Buffer_GetStart(gbuf)[i-2]);
  sw2=(unsigned char)(GWEN_Buffer_GetStart(gbuf)[i-1]);
  GWEN_Buffer_Crop(gbuf, 0, i-2);
  /* store result */
  dbTmp=GWEN_DB_GetGroup(rspData,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "result");
  assert(dbTmp);
  GWEN_DB_SetIntValue(dbTmp, GWEN_DB_FLAGS_DEFAULT,
                      "sw1", sw1);
  GWEN_DB_SetIntValue(dbTmp, GWEN_DB_FLAGS_DEFAULT,
                      "sw2", sw2);

  rnode=LC_Client_FindResult(cl, node, sw1, sw2);
  if (rnode) {
    const char *t;
    GWEN_XMLNODE *tnode;
    GWEN_BUFFER *txtbuf;
    int first;

    t=GWEN_XMLNode_GetProperty(rnode, "type", "success");
    DBG_INFO(0, "Result is: %s", t);
    GWEN_DB_SetCharValue(dbTmp,
                         GWEN_DB_FLAGS_DEFAULT,
                         "type", t);
    /* get text */
    txtbuf=GWEN_Buffer_new(0, 256, 0, 1);
    first=1;
    tnode=GWEN_XMLNode_GetFirstData(rnode);
    while(tnode) {
      const char *p;

      p=GWEN_XMLNode_GetData(tnode);
      if (p) {
        if (!first)
          GWEN_Buffer_AppendByte(txtbuf, ' ');
        GWEN_Buffer_AppendString(txtbuf, p);
      }
      if (first)
        first=0;
      tnode=GWEN_XMLNode_GetNextData(tnode);
    } /* while */

    if (GWEN_Buffer_GetUsedBytes(txtbuf))
      GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_DEFAULT, "text",
                           GWEN_Buffer_GetStart(txtbuf));
    GWEN_Buffer_free(txtbuf);
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN,
              "Result for %02x/%02x not found, assuming error",
              sw1, sw2);
    GWEN_DB_SetCharValue(dbTmp,
                         GWEN_DB_FLAGS_DEFAULT,
                         "type", "error");
    GWEN_DB_SetCharValue(dbTmp,
                         GWEN_DB_FLAGS_DEFAULT,
                         "text", "Result not found");
  }

  return 0;
}



int LC_Client_ParseResponse(LC_CLIENT *cl,
                            GWEN_XMLNODE *node,
                            GWEN_BUFFER *gbuf,
                            GWEN_DB_NODE *rspData){
  GWEN_DB_NODE *dbTmp;
  GWEN_XMLNODE *rnode;
  const char *p;

  assert(cl);

  GWEN_Buffer_Rewind(gbuf); /* just in case ... */

  p=GWEN_DB_GetCharValue(rspData, "result/type", 0, 0);
  if (!p) {
    DBG_ERROR(LC_LOGDOMAIN, "No result type given");
    return -1;
  }
  dbTmp=GWEN_DB_GetGroup(rspData,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "response");
  if (!dbTmp) {
    DBG_ERROR(LC_LOGDOMAIN, "No matching response tag found");
    return -1;
  }

  rnode=LC_Client_FindResponse(cl, node, p);
  if (!rnode) {
    DBG_DEBUG(0, "Did not find response");
    if (GWEN_Buffer_GetUsedBytes(gbuf)) {
      GWEN_DB_SetBinValue(dbTmp,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "data",
                          GWEN_Buffer_GetStart(gbuf),
                          GWEN_Buffer_GetUsedBytes(gbuf));
    }
  }
  else {
    if (GWEN_MsgEngine_ParseMessage(cl->msgEngine,
                                    rnode,
                                    gbuf,
                                    dbTmp,
                                    GWEN_MSGENGINE_READ_FLAGS_DEFAULT)){
      DBG_ERROR(LC_LOGDOMAIN, "Error parsing response");
      return -1;
    }
  }

  return 0;
}



int LC_Client_ParseAnswer(LC_CLIENT *cl,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFER *gbuf,
                          GWEN_DB_NODE *rspData){
  assert(cl);

  if (LC_Client_ParseResult(cl, node, gbuf, rspData)) {
    DBG_INFO(0, "Error parsing result");
    return -1;
  }

  if (LC_Client_ParseResponse(cl, node, gbuf, rspData)){
    DBG_INFO(0, "Error parsing response");
    return -1;
  }

  return 0;
}



LC_CLIENT_RESULT LC_Client_BuildApdu(LC_CLIENT *cl,
                                     LC_CARD *card,
                                     const char *command,
                                     GWEN_DB_NODE *cmdData,
                                     GWEN_BUFFER *buf) {
  GWEN_XMLNODE *node;
  LC_CLIENT_RESULT res;

  DBG_INFO(LC_LOGDOMAIN, "Building APDU for command \"%s\"", command);
  /* lookup card command */
  DBG_INFO(LC_LOGDOMAIN, "- looking up command");
  node=LC_Card_FindCommand(card, command);
  if (!node) {
    DBG_INFO(LC_LOGDOMAIN, "Command \"%s\" not found",
             command);
    return LC_Client_ResultNotFound;
  }

  /* build APDU */
  DBG_INFO(LC_LOGDOMAIN, "- building APDU");
  res=LC_Client__BuildApdu(cl, node, cmdData, buf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN,
	     "Error building APDU for command \"%s\" (%d)",
	     command, res);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Client_ExecCommand(LC_CLIENT *cl,
                                       LC_CARD *card,
                                       const char *commandName,
                                       GWEN_DB_NODE *cmdData,
                                       GWEN_DB_NODE *rspData) {
  GWEN_XMLNODE *node;
  GWEN_BUFFER *buf;
  GWEN_BUFFER *rbuf;
  LC_CLIENT_RESULT res;
  LC_CLIENT_CMDTARGET t;
  const char *s;

  DBG_INFO(LC_LOGDOMAIN, "Executing command \"%s\"", commandName);

  /* lookup card command */
  DBG_INFO(LC_LOGDOMAIN, "- looking up command");
  node=LC_Card_FindCommand(card, commandName);
  if (!node) {
    DBG_INFO(LC_LOGDOMAIN, "Command \"%s\" not found",
             commandName);
    return LC_Client_ResultNotFound;
  }

  /* determine target of the command */
  DBG_INFO(LC_LOGDOMAIN, "- determining target");
  t=LC_Client_CmdTargetCard;
  s=GWEN_XMLNode_GetProperty(node, "target", "card");
  if (s) {
    if (strcasecmp(s, "card")==0)
      t=LC_Client_CmdTargetCard;
    else if (strcasecmp(s, "reader")==0)
      t=LC_Client_CmdTargetReader;
    else {
      DBG_ERROR(LC_LOGDOMAIN,
                "Invalid target given in command \"%s\": %s",
                commandName, s);
      return LC_Client_ResultCfgError;
    }
  }

  /* build APDU */
  DBG_INFO(LC_LOGDOMAIN, "- building APDU");
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Client__BuildApdu(cl, node, cmdData, buf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN,
	     "Error building APDU for command \"%s\" (%d)",
	     commandName, res);
    GWEN_Buffer_free(buf);
    return res;
  }

  /* send APDU */
  DBG_INFO(LC_LOGDOMAIN, "- sending APDU, waiting for answer");
  rbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_ExecApdu(card,
                       GWEN_Buffer_GetStart(buf),
                       GWEN_Buffer_GetUsedBytes(buf),
                       rbuf,
                       t);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    GWEN_Buffer_free(rbuf);
    GWEN_Buffer_free(buf);
    return res;
  }
  GWEN_Buffer_free(buf);

  /* parse answer */
  DBG_INFO(LC_LOGDOMAIN, "- parsing response");
  if (LC_Client_ParseAnswer(cl, node, rbuf, rspData)) {
    DBG_INFO(LC_LOGDOMAIN, "Error parsing answer");
    GWEN_Buffer_free(rbuf);
    return LC_Client_ResultCmdError;
  }

  /* store response data */
  if (GWEN_Buffer_GetUsedBytes(rbuf)) {
    GWEN_DB_SetBinValue(rspData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "data",
                        GWEN_Buffer_GetStart(rbuf),
                        GWEN_Buffer_GetUsedBytes(rbuf));
  }
  GWEN_Buffer_free(rbuf);

  /* check for error result of command */
  s=GWEN_DB_GetCharValue(rspData, "result/type", 0, "error");
  if (strcasecmp(s, "success")!=0) {
    DBG_INFO(LC_LOGDOMAIN, "Command execution error flagged by card (%s)",
             s?s:"(none)");
    return LC_Client_ResultCmdError;
  }

  /* done */
  return LC_Client_ResultOk;
}









