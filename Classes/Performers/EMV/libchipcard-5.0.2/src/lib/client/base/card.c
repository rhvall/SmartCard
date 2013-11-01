/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define LC_CARD_EXTEND_CLIENT


#include "card_p.h"
#include "client_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gwentime.h>
#include <chipcard/chipcard.h>

#ifdef OS_WIN32
# define CM_IOCTL_GET_FEATURE_REQUEST SCARD_CTL_CODE(3400)

# define FEATURE_VERIFY_PIN_START  0x01 /* OMNIKEY Proposal */
# define FEATURE_VERIFY_PIN_FINISH 0x02 /* OMNIKEY Proposal */
# define FEATURE_MODIFY_PIN_START  0x03 /* OMNIKEY Proposal */
# define FEATURE_MODIFY_PIN_FINISH 0x04 /* OMNIKEY Proposal */
# define FEATURE_GET_KEY_PRESSED   0x05 /* OMNIKEY Proposal */
# define FEATURE_VERIFY_PIN_DIRECT 0x06 /* USB CCID PIN Verify */
# define FEATURE_MODIFY_PIN_DIRECT 0x07 /* USB CCID PIN Modify */
# define FEATURE_MCT_READERDIRECT  0x08 /* KOBIL Proposal */
# define FEATURE_MCT_UNIVERSAL     0x09 /* KOBIL Proposal */
# define FEATURE_IFD_PIN_PROP      0x0A /* Gemplus Proposal */
# define FEATURE_ABORT             0x0B /* SCM Proposal */

/* Set structure elements aligment on bytes
 * http://gcc.gnu.org/onlinedocs/gcc/Structure_002dPacking-Pragmas.html */
#pragma pack(push, 1)

/* the structure must be 6-bytes long */
typedef struct {
  uint8_t tag;
  uint8_t length;
  uint32_t value;
} PCSC_TLV_STRUCTURE;

#pragma pack(pop)


#elif defined (OS_DARWIN)
# define SCARD_CTL_CODE(code) (0x42000000 + (code))
# define CM_IOCTL_GET_FEATURE_REQUEST SCARD_CTL_CODE(3400)

# define FEATURE_VERIFY_PIN_START  0x01 /* OMNIKEY Proposal */
# define FEATURE_VERIFY_PIN_FINISH 0x02 /* OMNIKEY Proposal */
# define FEATURE_MODIFY_PIN_START  0x03 /* OMNIKEY Proposal */
# define FEATURE_MODIFY_PIN_FINISH 0x04 /* OMNIKEY Proposal */
# define FEATURE_GET_KEY_PRESSED   0x05 /* OMNIKEY Proposal */
# define FEATURE_VERIFY_PIN_DIRECT 0x06 /* USB CCID PIN Verify */
# define FEATURE_MODIFY_PIN_DIRECT 0x07 /* USB CCID PIN Modify */
# define FEATURE_MCT_READERDIRECT  0x08 /* KOBIL Proposal */
# define FEATURE_MCT_UNIVERSAL     0x09 /* KOBIL Proposal */
# define FEATURE_IFD_PIN_PROP      0x0A /* Gemplus Proposal */
# define FEATURE_ABORT             0x0B /* SCM Proposal */

/* Set structure elements aligment on bytes
 * http://gcc.gnu.org/onlinedocs/gcc/Structure_002dPacking-Pragmas.html */
#ifdef __APPLE__
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

/* the structure must be 6-bytes long */
typedef struct {
  uint8_t tag;
  uint8_t length;
  uint32_t value;
} PCSC_TLV_STRUCTURE;

#ifdef __APPLE__
#pragma pack()
#else
#pragma pack(pop)
#endif

#else
# include <PCSC/reader.h>
#endif


#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(LC_CARD, LC_Card)
GWEN_INHERIT_FUNCTIONS(LC_CARD)
GWEN_LIST2_FUNCTIONS(LC_CARD, LC_Card)


LC_CARD *LC_Card_new(LC_CLIENT *cl,
		     SCARDHANDLE scardHandle,
		     const char *readerName,
		     DWORD protocol,
		     const char *cardType,
                     uint32_t rflags,
                     const unsigned char *atrBuf,
                     unsigned int atrLen) {
  LC_CARD *cd;

  assert(cl);
  assert(cardType);

  GWEN_NEW_OBJECT(LC_CARD, cd);
  GWEN_LIST_INIT(LC_CARD, cd);
  GWEN_INHERIT_INIT(LC_CARD, cd);
  cd->client=cl;
  cd->cardType=strdup(cardType);
  cd->readerFlags=rflags;
  cd->cardTypes=GWEN_StringList_new();
  cd->dbCommandCache=GWEN_DB_Group_new("commandCache");
  cd->usage=1;
  if (atrBuf && atrLen) {
    cd->atr=GWEN_Buffer_new(0, atrLen, 0, 1);
    GWEN_Buffer_AppendBytes(cd->atr, (const char*)atrBuf, atrLen);
  }

  cd->openFn=LC_Card__Open;
  cd->closeFn=LC_Card__Close;

  /* determine card types by comparing the ATR to known ATRs */
  if (cd->atr) {
    int rv;

    rv=LC_Client_AddCardTypesByAtr(cl, cd);
    if (rv) {
      if (rv==1) {
        DBG_WARN(LC_LOGDOMAIN, "Unknown card type (no matching ATR)");
      }
      else {
        DBG_ERROR(LC_LOGDOMAIN, "Error determining card types");
      }
    }
  }

  cd->readerName=strdup(readerName);
  cd->scardHandle=scardHandle;
  cd->protocol=protocol;

  return cd;
}



void LC_Card_free(LC_CARD *cd){
  if (cd) {
    assert(cd->usage>0);
    cd->usage--;
    if (cd->usage==0) {
      GWEN_INHERIT_FINI(LC_CARD, cd);
      if (cd->connected) {
        DBG_WARN(LC_LOGDOMAIN,
                 "Card to be deleted is still connected");
      }
      free(cd->cardType);
      free(cd->lastResult);
      free(cd->lastText);
      GWEN_StringList_free(cd->cardTypes);
      GWEN_Buffer_free(cd->atr);
      GWEN_DB_Group_free(cd->dbCommandCache);
      GWEN_LIST_FINI(LC_CARD, cd);
      GWEN_FREE_OBJECT(cd);
    }
  }
}



void LC_Card_List2_freeAll(LC_CARD_LIST2 *l){
  if (l) {
    LC_CARD_LIST2_ITERATOR *cit;

    cit=LC_Card_List2_First(l);
    if (cit) {
      LC_CARD *card;

      card=LC_Card_List2Iterator_Data(cit);
      while(card) {
        LC_CARD *next;

        next=LC_Card_List2Iterator_Next(cit);
        LC_Card_free(card);
        card=next;
      } /* while */
      LC_Card_List2Iterator_free(cit);
    }
    LC_Card_List2_free(l);
  }
}



SCARDHANDLE LC_Card_GetSCardHandle(const LC_CARD *card) {
  assert(card);
  return card->scardHandle;
}



int LC_Card_IsConnected(const LC_CARD *card) {
  assert(card);
  return card->connected;
}



void LC_Card_SetConnected(LC_CARD *card, int b) {
  assert(card);
  card->connected=b;
}



const char *LC_Card_GetReaderType(const LC_CARD *cd) {
  assert(cd);
  return cd->readerType;
}



void LC_Card_SetReaderType(LC_CARD *cd, const char *s) {
  assert(cd);
  free(cd->readerType);
  if (s)
    cd->readerType=strdup(s);
  else
    cd->readerType=0;
}



const char *LC_Card_GetDriverType(const LC_CARD *cd) {
  assert(cd);
  return cd->driverType;
}



void LC_Card_SetDriverType(LC_CARD *cd, const char *s) {
  assert(cd);
  free(cd->driverType);
  if (s)
    cd->driverType=strdup(s);
  else
    cd->driverType=0;
}



const GWEN_STRINGLIST *LC_Card_GetCardTypes(const LC_CARD *cd){
  assert(cd);
  return cd->cardTypes;
}



int LC_Card_AddCardType(LC_CARD *cd, const char *s) {
  assert(cd);
  return GWEN_StringList_AppendString(cd->cardTypes, s, 0, 1);
}



void LC_Card_SetLastResult(LC_CARD *cd,
                           const char *result,
                           const char *text,
                           int sw1, int sw2){
  assert(cd);
  free(cd->lastResult);
  free(cd->lastText);
  if (result)
    cd->lastResult=strdup(result);
  else
    cd->lastResult=0;
  if (text)
    cd->lastText=strdup(text);
  else
    cd->lastText=0;
  cd->lastSW1=sw1;
  cd->lastSW2=sw2;
}


int LC_Card_GetLastSW1(const LC_CARD *cd){
  assert(cd);
  return cd->lastSW1;
}



int LC_Card_GetLastSW2(const LC_CARD *cd){
  assert(cd);
  return cd->lastSW2;
}



const char *LC_Card_GetLastResult(const LC_CARD *cd){
  assert(cd);
  return cd->lastResult;
}



const char *LC_Card_GetLastText(const LC_CARD *cd){
  assert(cd);
  return cd->lastText;
}



LC_CLIENT *LC_Card_GetClient(const LC_CARD *cd){
  assert(cd);
  return cd->client;
}



uint32_t LC_Card_GetReaderFlags(const LC_CARD *cd){
  assert(cd);
  return cd->readerFlags;
}



const char *LC_Card_GetCardType(const LC_CARD *cd){
  assert(cd);
  return cd->cardType;
}



void LC_Card_SetCardType(LC_CARD *cd, const char *ct){
  assert(cd);
  assert(ct);

  free(cd->cardType);
  cd->cardType=strdup(ct);
}



unsigned int LC_Card_GetAtr(const LC_CARD *cd, const unsigned char **pbuf){
  assert(cd);
  if (cd->atr) {
    unsigned int len;

    len=GWEN_Buffer_GetUsedBytes(cd->atr);
    if (len) {
      *pbuf=(const unsigned char*)GWEN_Buffer_GetStart(cd->atr);
      return len;
    }
  }
  return 0;
}



uint32_t LC_Card_GetFeatureCode(const LC_CARD *cd, int idx) {
  assert(cd);
  assert(idx<LC_PCSC_MAX_FEATURES);
  return cd->featureCode[idx];
}



const char *LC_Card_GetReaderName(const LC_CARD *card) {
  assert(card);
  return card->readerName;
}



DWORD LC_Card_GetProtocol(const LC_CARD *card) {
  assert(card);
  return card->protocol;
}



void LC_Card_Dump(const LC_CARD *cd, int insert) {
  int k;
  GWEN_STRINGLISTENTRY *se;
  GWEN_DB_NODE *dbT;

  assert(cd);
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Card\n");
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr,
          "==================="
          "==================="
          "==================="
          "==================\n");
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Card type     : %s\n", cd->cardType);
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Driver type   : %s\n", cd->driverType);
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Reader type   : %s\n", cd->readerType);
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Card types    :");
  se=GWEN_StringList_FirstEntry(cd->cardTypes);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    assert(s);
    fprintf(stderr, " %s", s);
    se=GWEN_StringListEntry_Next(se);
  } /* while */
  fprintf(stderr, "\n");
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Reader flags  : ");

  dbT=GWEN_DB_Group_new("flags");
  LC_ReaderFlags_toDb(dbT, "flags", cd->readerFlags);
  for (k=0; k<32; k++) {
    const char *s;

    s=GWEN_DB_GetCharValue(dbT, "flags", k, 0);
    if (!s)
      break;
    if (k)
      fprintf(stderr, ", ");
    fprintf(stderr, "%s", s);
  }
  fprintf(stderr, "\n");
  GWEN_DB_Group_free(dbT);

  if (cd->atr) {
    for (k=0; k<insert; k++)
      fprintf(stderr, " ");
    fprintf(stderr, "ATR\n");
    for (k=0; k<insert; k++)
      fprintf(stderr, " ");
    fprintf(stderr,
            "-------------------"
            "-------------------"
            "-------------------"
            "------------------\n");
    GWEN_Text_DumpString(GWEN_Buffer_GetStart(cd->atr),
                         GWEN_Buffer_GetUsedBytes(cd->atr),
                         insert+2);
  }
  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr,
          "==================="
          "==================="
          "==================="
          "==================\n");
}



LC_CLIENT_RESULT LC_Card_ReadFeatures(LC_CARD *card) {
  LONG rv;
  unsigned char rbuffer[300];
  DWORD rblen;

  assert(card);

  /* get control codes */
  DBG_INFO(LC_LOGDOMAIN, "Reading control codes for CCID features");
  rv=SCardControl(card->scardHandle,
		  CM_IOCTL_GET_FEATURE_REQUEST,
                  NULL,
                  0,
                  rbuffer,
                  sizeof(rbuffer),
                  &rblen);
  if (rv!=SCARD_S_SUCCESS) {
    DBG_INFO(LC_LOGDOMAIN,
	     "SCardControl: %04lx", (long unsigned int) rv);
  }
  else {
    int cnt;
    PCSC_TLV_STRUCTURE *tlv;
    int i;

    /* clear keypad flag; if there is TLV indicating the reader has a keypad and
     * the driver supports it we set the flag upon encounter of the tlv */
    card->readerFlags&=~LC_READER_FLAGS_KEYPAD;
    cnt=rblen/sizeof(PCSC_TLV_STRUCTURE);
    tlv=(PCSC_TLV_STRUCTURE*)rbuffer;
    for (i=0; i<cnt; i++) {
      uint32_t v;

      v=tlv[i].value;
#ifdef LC_ENDIAN_LITTLE
      v=((v & 0xff000000)>>24) |
        ((v & 0x00ff0000)>>8) |
        ((v & 0x0000ff00)<<8) |
        ((v & 0x000000ff)<<24);
#endif
      DBG_INFO(LC_LOGDOMAIN, "Feature %d: %08x", tlv[i].tag, v);
      if (tlv[i].tag==FEATURE_VERIFY_PIN_DIRECT)
	card->readerFlags|=LC_READER_FLAGS_KEYPAD;
      if (tlv[i].tag<LC_PCSC_MAX_FEATURES) {
	card->featureCode[tlv[i].tag]=v;
      }
    }
  }

  /* done */
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Card_Open(LC_CARD *card) {
  LONG rv;

  assert(card);

  rv=LC_Card_ReadFeatures(card);
  if (rv!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", (int) rv);
  }

  LC_Card_SetLastResult(card, 0, 0, -1, -1);
  if (!card->openFn) {
    DBG_DEBUG(LC_LOGDOMAIN, "No OpenFn set");
    return LC_Client_ResultOk;
  }
  return card->openFn(card);
}



LC_CLIENT_RESULT LC_Card_Close(LC_CARD *card) {
  LC_CLIENT_RESULT res;

  assert(card);
  LC_Card_SetLastResult(card, 0, 0, -1, -1);
  if (!card->closeFn) {
    DBG_DEBUG(LC_LOGDOMAIN, "No CloseFn set");
    res=LC_Client_ResultOk;
  }
  else
    res=card->closeFn(card);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_Card__Open(LC_CARD *card) {
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_Card__Close(LC_CARD *card) {
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Card_ExecApdu(LC_CARD *card,
                                  const char *apdu,
                                  unsigned int len,
                                  GWEN_BUFFER *rbuf,
                                  LC_CLIENT_CMDTARGET t) {
  assert(card);
  assert(card->client);
  LC_Card_SetLastResult(card, 0, 0, -1, -1);
  return LC_Client_ExecApdu(card->client,
                            card,
                            apdu,
                            len,
                            rbuf,
                            t);
}



LC_CLIENT_RESULT LC_Card_ExecCommand(LC_CARD *card,
                                     const char *commandName,
                                     GWEN_DB_NODE *cmdData,
                                     GWEN_DB_NODE *rspData) {
  LC_CLIENT_RESULT res;

  assert(card);
  assert(card->client);
  LC_Card_SetLastResult(card, 0, 0, -1, -1);
  res=LC_Client_ExecCommand(card->client,
                            card,
                            commandName,
                            cmdData,
                            rspData);
  return res;
}



GWEN_XMLNODE *LC_Card_FindCommand(LC_CARD *card,
                                  const char *commandName) {
  GWEN_DB_NODE *db;
  GWEN_XMLNODE *node;

  assert(card);
  assert(commandName);

  db=card->dbCommandCache;
  if (card->driverType) {
    db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, card->driverType);
    assert(db);
  }
  if (card->readerType) {
    db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, card->readerType);
    assert(db);
  }

  node=(GWEN_XMLNODE*)GWEN_DB_GetPtrValue(db, commandName, 0, 0);
  if (node==0) {
    node=LC_Client_FindCardCommand(card->client, card, commandName);
    if (node)
      GWEN_DB_SetPtrValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          commandName,
                          (void*) node);
  }
  else {
    DBG_INFO(LC_LOGDOMAIN, "Found command \"%s\" in cache", commandName);
  }

  return node;
}



LC_CLIENT_RESULT LC_Card_BuildApdu(LC_CARD *card,
                                   const char *command,
                                   GWEN_DB_NODE *cmdData,
                                   GWEN_BUFFER *gbuf) {
  assert(card);
  assert(card->client);
  return LC_Client_BuildApdu(card->client,
                             card,
                             command,
                             cmdData,
                             gbuf);

}



LC_CLIENT_RESULT LC_Card_SelectApp(LC_CARD *card, const char *appName) {
  GWEN_XMLNODE *node;

  node=LC_Client_GetAppNode(card->client, appName);
  if (node==0) {
    DBG_INFO(LC_LOGDOMAIN, "App not found");
    return LC_Client_ResultNotFound;
  }
  card->appNode=node;
  card->dfNode=0;
  card->efNode=0;
  return LC_Client_ResultOk;
}



GWEN_XMLNODE *LC_Card_GetAppNode(const LC_CARD *card) {
  assert(card);
  return card->appNode;
}



LC_CLIENT_RESULT LC_Card_SelectCard(LC_CARD *card, const char *s) {
  assert(card);
  if (s==0)
    card->cardNode=0;
  else {
    GWEN_XMLNODE *node;

    node=LC_Client_GetCardNode(card->client, s);
    if (node==0) {
      DBG_INFO(LC_LOGDOMAIN, "Card type not found");
      return LC_Client_ResultNotFound;
    }
    card->cardNode=node;
    DBG_INFO(LC_LOGDOMAIN, "Clearing command cache");
    GWEN_DB_ClearGroup(card->dbCommandCache, NULL);
  }
  return LC_Client_ResultOk;
}



GWEN_XMLNODE *LC_Card_GetCardNode(const LC_CARD *card) {
  assert(card);
  return card->cardNode;
}




GWEN_XMLNODE *LC_Card_FindFile(LC_CARD *card,
                               const char *type,
                               const char *fname) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *currDF;
  int isSameLevel;

  currDF=card->dfNode;
  if (!currDF)
    currDF=card->appNode;

  isSameLevel=1;
  while(currDF) {
    n=GWEN_XMLNode_FindNode(currDF, GWEN_XMLNodeTypeTag, "files");
    if (n) {
      n=GWEN_XMLNode_FindFirstTag(n, type, "name", fname);
      if (n) {
        if (isSameLevel) {
          return n;
        }
        if (atoi(GWEN_XMLNode_GetProperty(n, "inAnyDF", "0"))!=0) {
          DBG_DEBUG(LC_LOGDOMAIN, "Returning file from level above");
          return n;
        }
      }
    }
    currDF=GWEN_XMLNode_GetParent(currDF);
    isSameLevel=0;
  }
  DBG_DEBUG(LC_LOGDOMAIN, "%s \"%s\" not found", type, fname);
  return 0;
}



LC_CLIENT_RESULT LC_Card_SelectMf(LC_CARD *card) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbRsp;
  LC_CLIENT_RESULT res;

  dbReq=GWEN_DB_Group_new("request");
  dbRsp=GWEN_DB_Group_new("response");
  res=LC_Card_ExecCommand(card, "SelectMF", dbReq, dbRsp);
  GWEN_DB_Group_free(dbRsp);
  GWEN_DB_Group_free(dbReq);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }
  card->dfNode=0;

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Card_SelectDf(LC_CARD *card, const char *fname) {
  GWEN_XMLNODE *n;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbRsp;
  const char *cmd;
  int fid;
  LC_CLIENT_RESULT res;

  n=LC_Card_FindFile(card, "DF", fname);
  if (!n) {
    DBG_ERROR(LC_LOGDOMAIN, "DF \"%s\" not found", fname);
    return LC_Client_ResultCmdError;
  }

  if (1!=sscanf(GWEN_XMLNode_GetProperty(n, "sid", "-1"), "%i", &fid)){
    DBG_ERROR(LC_LOGDOMAIN, "Bad id for DF \"%s\"", fname);
    return LC_Client_ResultCmdError;
  }

  dbReq=GWEN_DB_Group_new("request");
  if (fid==-1) {
    GWEN_BUFFER *buf;
    const char *lid;

    buf=GWEN_Buffer_new(0, 64, 0, 1);
    lid=GWEN_XMLNode_GetProperty(n, "lid", 0);
    if (!lid) {
      DBG_ERROR(LC_LOGDOMAIN, "No long id given in XML file");
      GWEN_Buffer_free(buf);
      GWEN_DB_Group_free(dbReq);
      return LC_Client_ResultDataError;
    }
    if (GWEN_Text_FromHexBuffer(lid, buf)) {
      DBG_ERROR(LC_LOGDOMAIN, "Bad long id given in XML file");
      GWEN_Buffer_free(buf);
      GWEN_DB_Group_free(dbReq);
      return LC_Client_ResultDataError;
    }

    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "fileId",
                        GWEN_Buffer_GetStart(buf),
                        GWEN_Buffer_GetUsedBytes(buf));
    cmd="SelectDFL";

  }
  else {
    GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "fileId", fid);
    cmd="SelectDFS";
  }

  dbRsp=GWEN_DB_Group_new("response");
  res=LC_Card_ExecCommand(card, cmd, dbReq, dbRsp);
  GWEN_DB_Group_free(dbRsp);
  GWEN_DB_Group_free(dbReq);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }
  card->dfNode=n;
  card->efNode=0;

  return LC_Client_ResultOk;
}



GWEN_XMLNODE *LC_Card_GetDfNode(const LC_CARD *card) {
  assert(card);
  return card->dfNode;
}



LC_CLIENT_RESULT LC_Card_SelectEf(LC_CARD *card, const char *fname) {
  GWEN_XMLNODE *n;
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbRsp;
  const char *cmd;
  int fid;
  LC_CLIENT_RESULT res;

  n=LC_Card_FindFile(card, "EF", fname);
  if (!n) {
    DBG_ERROR(LC_LOGDOMAIN, "EF \"%s\" not found", fname);
    return LC_Client_ResultCmdError;
  }

  if (1!=sscanf(GWEN_XMLNode_GetProperty(n, "sid", "-1"), "%i", &fid)){
    DBG_ERROR(LC_LOGDOMAIN, "Bad id for DF \"%s\"", fname);
    return LC_Client_ResultCmdError;
  }

  dbReq=GWEN_DB_Group_new("request");
  if (fid==-1) {
    GWEN_BUFFER *buf;
    const char *lid;

    buf=GWEN_Buffer_new(0, 64, 0, 1);
    lid=GWEN_XMLNode_GetProperty(n, "lid", 0);
    if (!lid) {
      DBG_ERROR(LC_LOGDOMAIN, "No long id given in XML file");
      GWEN_Buffer_free(buf);
      GWEN_DB_Group_free(dbReq);
      return LC_Client_ResultDataError;
    }
    if (GWEN_Text_FromHexBuffer(lid, buf)) {
      DBG_ERROR(LC_LOGDOMAIN, "Bad long id given in XML file");
      GWEN_Buffer_free(buf);
      GWEN_DB_Group_free(dbReq);
      return LC_Client_ResultDataError;
    }

    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "fileId",
                        GWEN_Buffer_GetStart(buf),
                        GWEN_Buffer_GetUsedBytes(buf));
    cmd="SelectEFL";

  }
  else {
    GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "fileId", fid);
    cmd="SelectEFS";
  }

  dbRsp=GWEN_DB_Group_new("response");
  res=LC_Card_ExecCommand(card, cmd, dbReq, dbRsp);
  GWEN_DB_Group_free(dbRsp);
  GWEN_DB_Group_free(dbReq);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }
  card->efNode=n;

  return LC_Client_ResultOk;
}



GWEN_XMLNODE *LC_Card_GetEfNode(const LC_CARD *card) {
  assert(card);
  return card->efNode;
}



LC_PININFO *LC_Card_GetPinInfoById(LC_CARD *card, uint32_t pid) {
  GWEN_XMLNODE *n;

  n=card->efNode;
  if (!n)
    n=card->dfNode;
  if (!n)
    n=card->appNode;
  if (!n) {
    DBG_INFO(LC_LOGDOMAIN, "No XML node");
    return 0;
  }

  while(n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "pins", 0, 0);
    while (nn) {
      GWEN_XMLNODE *nnn;

      nnn=GWEN_XMLNode_FindFirstTag(nn, "pin", 0, 0);
      while(nnn) {
        const char *s;

        s=GWEN_XMLNode_GetProperty(nnn, "id", 0);
        if (s) {
          int i;

          if (sscanf(s, "%i", &i)==1) {
            if (i==(int)pid) {
              LC_PININFO *pi;

              pi=LC_PinInfo_new();
              LC_PinInfo_SetId(pi, pid);
              s=GWEN_XMLNode_GetProperty(nnn, "name", 0);
              LC_PinInfo_SetName(pi, s);
              if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "minLen", "0"),
                            "%i", &i))
                LC_PinInfo_SetMinLength(pi, i);
              if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "maxLen", "0"),
                            "%i", &i))
                LC_PinInfo_SetMaxLength(pi, i);
              if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "allowChange", "0"),
                            "%i", &i))
                LC_PinInfo_SetAllowChange(pi, i);
              if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "filler", "0"),
                            "%i", &i))
                LC_PinInfo_SetFiller(pi, i);
              s=GWEN_XMLNode_GetProperty(nnn, "encoding", 0);
              if (s)
                LC_PinInfo_SetEncoding(pi, GWEN_Crypt_PinEncoding_fromString(s));
              return pi;
            }
          }
        }
        nnn=GWEN_XMLNode_FindNextTag(nnn, "pin", 0, 0);
      }

      nn=GWEN_XMLNode_FindNextTag(nn, "pins", 0, 0);
    }

    n=GWEN_XMLNode_GetParent(n);
  }

  return 0;
}



LC_PININFO *LC_Card_GetPinInfoByName(LC_CARD *card, const char *name) {
  GWEN_XMLNODE *n;

  assert(card);
  assert(card->usage);

  n=card->efNode;
  if (!n) {
    DBG_DEBUG(LC_LOGDOMAIN, "No EF node");
    n=card->dfNode;
  }
  if (!n) {
    DBG_DEBUG(LC_LOGDOMAIN, "No DF node");
    n=card->appNode;
  }
  if (!n) {
    DBG_INFO(LC_LOGDOMAIN, "No XML node");
    return 0;
  }

  while(n) {
    GWEN_XMLNODE *nn;

    DBG_DEBUG(LC_LOGDOMAIN, "Searching in \"%s\" (%s)",
              GWEN_XMLNode_GetProperty(n, "name", "(none)"),
              GWEN_XMLNode_GetData(n));

    nn=GWEN_XMLNode_FindFirstTag(n, "pins", 0, 0);
    while (nn) {
      GWEN_XMLNODE *nnn;

      nnn=GWEN_XMLNode_FindFirstTag(nn, "pin", 0, 0);
      while(nnn) {
        const char *s;
        int i;

        s=GWEN_XMLNode_GetProperty(nnn, "id", 0);
        if (s && sscanf(s, "%i", &i)==1) {
          s=GWEN_XMLNode_GetProperty(nnn, "name", 0);
          if (s && strcasecmp(s, name)==0) {
            LC_PININFO *pi;

            pi=LC_PinInfo_new();
            LC_PinInfo_SetId(pi, (uint32_t)i);
            s=GWEN_XMLNode_GetProperty(nnn, "name", 0);
            LC_PinInfo_SetName(pi, s);
            if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "minLen", "0"),
                          "%i", &i))
              LC_PinInfo_SetMinLength(pi, i);
            if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "maxLen", "0"),
                          "%i", &i))
              LC_PinInfo_SetMaxLength(pi, i);
            if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "allowChange", "0"),
                          "%i", &i))
              LC_PinInfo_SetAllowChange(pi, i);
            if (1==sscanf(GWEN_XMLNode_GetProperty(nnn, "filler", "0"),
                          "%i", &i))
              LC_PinInfo_SetFiller(pi, i);
            s=GWEN_XMLNode_GetProperty(nnn, "encoding", 0);
            if (s)
              LC_PinInfo_SetEncoding(pi, GWEN_Crypt_PinEncoding_fromString(s));
            return pi;
          }
        }
        nnn=GWEN_XMLNode_FindNextTag(nnn, "pin", 0, 0);
      }

      nn=GWEN_XMLNode_FindNextTag(nn, "pins", 0, 0);
    }

    n=GWEN_XMLNode_GetParent(n);
  }

  return 0;
}



LC_CLIENT_RESULT LC_Card_ParseData(LC_CARD *card,
                                   const char *format,
                                   GWEN_BUFFER *buf,
                                   GWEN_DB_NODE *dbData) {
  GWEN_XMLNODE *dataNode;
  GWEN_MSGENGINE *e;

  /* find format node */
  assert(card->appNode);
  e=LC_Client_GetMsgEngine(card->client);
  assert(e);
  if (!GWEN_Buffer_GetBytesLeft(buf)) {
    DBG_ERROR(LC_LOGDOMAIN, "End of buffer reached");
    return LC_Client_ResultNoData;
  }
  dataNode=GWEN_XMLNode_FindFirstTag(card->appNode, "formats", 0, 0);
  if (dataNode==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No formats for this card application");
    return LC_Client_ResultNotFound;
  }

  dataNode=GWEN_XMLNode_FindFirstTag(dataNode, "format", "name", format);
  if (!dataNode) {
    DBG_ERROR(LC_LOGDOMAIN, "Format \"%s\" not found", format);
    return LC_Client_ResultNotFound;
  }

  /* node found, parse data */
  DBG_DEBUG(LC_LOGDOMAIN, "Parsing data");
  if (GWEN_MsgEngine_ParseMessage(e,
                                  dataNode,
                                  buf,
                                  dbData,
                                  GWEN_MSGENGINE_READ_FLAGS_DEFAULT)){
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing data in format \"%s\"", format);
    return LC_Client_ResultDataError;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Card_CreateData(LC_CARD *card,
                                    const char *format,
                                    GWEN_BUFFER *buf,
                                    GWEN_DB_NODE *dbData){
  GWEN_XMLNODE *dataNode;
  GWEN_MSGENGINE *e;

  /* find record node */
  assert(card->appNode);
  e=LC_Client_GetMsgEngine(card->client);
  assert(e);

  dataNode=GWEN_XMLNode_FindFirstTag(card->appNode, "formats", 0, 0);
  if (dataNode==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No formats for this card application");
    return LC_Client_ResultNoData;
  }

  dataNode=GWEN_XMLNode_FindFirstTag(dataNode, "format", "name", format);
  if (!dataNode) {
    DBG_ERROR(LC_LOGDOMAIN, "Format \"%s\" not found", format);
    return LC_Client_ResultNoData;
  }

  /* node found, parse data */
  DBG_DEBUG(LC_LOGDOMAIN, "Creating data");
  if (GWEN_MsgEngine_CreateMessageFromNode(e,
                                           dataNode,
                                           buf,
                                           dbData)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error creating data for format \"%s\"", format);
    return LC_Client_ResultDataError;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Card_ParseRecord(LC_CARD *card,
                                     int recNum,
                                     GWEN_BUFFER *buf,
                                     GWEN_DB_NODE *dbRecord){
  GWEN_XMLNODE *recordNode;
  GWEN_MSGENGINE *e;

  /* find record node */
  assert(card->efNode);
  e=LC_Client_GetMsgEngine(card->client);
  assert(e);
  if (!GWEN_Buffer_GetBytesLeft(buf)) {
    DBG_ERROR(LC_LOGDOMAIN, "End of buffer reached");
    return LC_Client_ResultNoData;
  }
  recordNode=GWEN_XMLNode_FindFirstTag(card->efNode, "record", 0, 0);
  while(recordNode) {
    int lrecNum;

    if (1==sscanf(GWEN_XMLNode_GetProperty(recordNode,
                                           "recnum", "-1"),
                  "%i", &lrecNum)) {
      if (lrecNum!=-1 && recNum==lrecNum)
        break;
    }
    recordNode=GWEN_XMLNode_FindNextTag(recordNode, "record", 0, 0);
  } /* while */
  if (!recordNode)
    recordNode=GWEN_XMLNode_FindFirstTag(card->efNode,"record", 0, 0);

  if (recordNode) {
    /* node found, parse data */
    DBG_DEBUG(LC_LOGDOMAIN, "Parsing record data");
    if (GWEN_MsgEngine_ParseMessage(e,
                                    recordNode,
                                    buf,
                                    dbRecord,
                                    GWEN_MSGENGINE_READ_FLAGS_DEFAULT)){
      DBG_ERROR(LC_LOGDOMAIN, "Error parsing response");
      return LC_Client_ResultDataError;
    }
  } /* if record found */
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Record not found");
    return LC_Client_ResultNotFound;
  }
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Card_CreateRecord(LC_CARD *card,
                                      int recNum,
                                      GWEN_BUFFER *buf,
                                      GWEN_DB_NODE *dbRecord){
  GWEN_XMLNODE *recordNode;
  GWEN_MSGENGINE *e;

  /* find record node */
  assert(card->efNode);
  e=LC_Client_GetMsgEngine(card->client);
  assert(e);
  recordNode=GWEN_XMLNode_FindFirstTag(card->efNode, "record", 0, 0);
  while(recordNode) {
    int lrecNum;

    if (1==sscanf(GWEN_XMLNode_GetProperty(recordNode,
                                           "recnum", "-1"),
                  "%i", &lrecNum)) {
      if (lrecNum!=-1 && recNum==lrecNum)
        break;
    }
    recordNode=GWEN_XMLNode_FindNextTag(recordNode, "record", 0, 0);
  } /* while */
  if (!recordNode)
    recordNode=GWEN_XMLNode_FindFirstTag(card->efNode,"record", 0, 0);

  if (recordNode) {
    /* node found, parse data */
    DBG_DEBUG(LC_LOGDOMAIN, "Creating record data");
    if (GWEN_MsgEngine_CreateMessageFromNode(e,
                                             recordNode,
                                             buf,
                                             dbRecord)) {
      DBG_ERROR(LC_LOGDOMAIN, "Error creating record");
      return LC_Client_ResultDataError;
    }
  } /* if record found */
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Record not found");
    return LC_Client_ResultNotFound;
  }
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_Card_GetPinStatus(LC_CARD *card,
                                      unsigned int pid,
                                      int *maxErrors,
                                      int *currentErrors) {
  assert(card);
  if (card->getPinStatusFn) {
    return card->getPinStatusFn(card, pid, maxErrors, currentErrors);
  }
  else {
    DBG_INFO(LC_LOGDOMAIN,
             "no getInitialPin function set");
    return LC_Client_ResultNotSupported;
  }
}



LC_CLIENT_RESULT LC_Card_GetInitialPin(LC_CARD *card,
                                       int id,
                                       unsigned char *buffer,
                                       unsigned int maxLen,
                                       unsigned int *pinLength) {
  assert(card);
  if (card->getInitialPinFn) {
    return card->getInitialPinFn(card, id, buffer, maxLen, pinLength);
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN,
              "no getInitialPin function set");
    return LC_Client_ResultNotSupported;
  }
}



LC_CARD_OPEN_FN LC_Card_GetOpenFn(const LC_CARD *card){
  assert(card);
  return card->openFn;
}



void LC_Card_SetOpenFn(LC_CARD *card, LC_CARD_OPEN_FN fn){
  assert(card);
  card->openFn=fn;
}



LC_CARD_CLOSE_FN LC_Card_GetCloseFn(const LC_CARD *card){
  assert(card);
  return card->closeFn;
}



void LC_Card_SetCloseFn(LC_CARD *card, LC_CARD_CLOSE_FN fn){
  assert(card);
  card->closeFn=fn;
}



void LC_Card_SetGetPinStatusFn(LC_CARD *card, LC_CARD_GETPINSTATUS_FN fn) {
  assert(card);
  card->getPinStatusFn=fn;
}



void LC_Card_SetGetInitialPinFn(LC_CARD *card, LC_CARD_GETINITIALPIN_FN fn){
  assert(card);
  card->getInitialPinFn=fn;
}



void LC_Card_CreateResultString(const LC_CARD *card,
                                const char *lastCommand,
                                LC_CLIENT_RESULT res,
                                GWEN_BUFFER *buf) {
  const char *s;

  switch(res) {
  case LC_Client_ResultOk:
    s="Ok.";
    break;
  case LC_Client_ResultWait:
    s="Timeout.";
    break;
  case LC_Client_ResultIpcError:
    s="IPC error.";
    break;
  case LC_Client_ResultCmdError:
    s="Command error.";
    break;
  case LC_Client_ResultDataError:
    s="Data error.";
    break;
  case LC_Client_ResultAborted:
    s="Aborted.";
    break;
  case LC_Client_ResultInvalid:
    s="Invalid argument to command.";
    break;
  case LC_Client_ResultInternal:
    s="Internal error.";
    break;
  case LC_Client_ResultGeneric:
    s="Generic error.";
    break;
  default:
    s="Unknown error.";
    break;
  }

  GWEN_Buffer_AppendString(buf, "Result of \"");
  GWEN_Buffer_AppendString(buf, lastCommand);
  GWEN_Buffer_AppendString(buf, "\": ");
  GWEN_Buffer_AppendString(buf, s);

  if (res==LC_Client_ResultCmdError && card) {
    int sw1;
    int sw2;
    char numbuf[32];

    sw1=LC_Card_GetLastSW1(card);
    sw2=LC_Card_GetLastSW2(card);
    GWEN_Buffer_AppendString(buf, " (");
    if (sw1!=-1 && sw2!=-1) {
      GWEN_Buffer_AppendString(buf, " SW1=");
      snprintf(numbuf, sizeof(numbuf), "%02x", sw1);
      GWEN_Buffer_AppendString(buf, numbuf);
      GWEN_Buffer_AppendString(buf, " SW2=");
      snprintf(numbuf, sizeof(numbuf), "%02x", sw2);
      GWEN_Buffer_AppendString(buf, numbuf);
    }
    s=LC_Card_GetLastResult(card);
    if (s) {
      GWEN_Buffer_AppendString(buf, " result=");
      GWEN_Buffer_AppendString(buf, s);
    }
    s=LC_Card_GetLastText(card);
    if (s) {
      GWEN_Buffer_AppendString(buf, " text=");
      GWEN_Buffer_AppendString(buf, s);
    }
    GWEN_Buffer_AppendString(buf, " )");
  }
}



LC_CLIENT_RESULT LC_Card_ReadBinary(LC_CARD *card,
				    int offset,
				    int size,
				    GWEN_BUFFER *buf){
  int t;
  int bytesRead=0;
  LC_CLIENT_RESULT res;

  while(size>0) {
    int sw1;
    int sw2;

    if (size>252)
      t=252;
    else
      t=size;
    res=LC_Card_IsoReadBinary(card, 0,
                              offset, t, buf);
    if (res!=LC_Client_ResultOk) {
      if (res==LC_Client_ResultNoData && bytesRead)
        return LC_Client_ResultOk;
      return res;
    }

    size-=t;
    offset+=t;
    bytesRead+=t;

    /* check for EOF */
    sw1=LC_Card_GetLastSW1(card);
    sw2=LC_Card_GetLastSW2(card);
    if (sw1==0x62 && sw2==0x82) {
      DBG_DEBUG(LC_LOGDOMAIN, "EOF met after %d bytes (asked for %d bytes more)", bytesRead, size);
      break;
    }
  } /* while still data to read */

  return LC_Client_ResultOk;
}



#include "card_iso.c"








