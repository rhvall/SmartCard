

LC_CLIENT_RESULT CHIPCARD_CB
  LC_Card__IsoReadBinary(LC_CARD *card,
                         uint32_t flags,
                         int offset,
                         int size,
                         GWEN_BUFFER *buf){
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  unsigned int bs;
  const void *p;

  DBG_INFO(LC_LOGDOMAIN, "Reading binary %04x:%04x", offset, size);

  if (flags & LC_CARD_ISO_FLAGS_EFID_MASK) {
    if (offset>255) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Offset too high when implicitly selecting EF "
		"(%u)", flags);
      return LC_Client_ResultInvalid;
    }
    /* modify offset: highbyte is p1, lowbyte is p2 */
    offset|=0x8000;
    offset|=((flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<8);
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "offset", offset);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "lr", size);

  res=LC_Card_ExecCommand(card, "IsoReadBinary", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  /* successful */
  if (buf) {
    p=GWEN_DB_GetBinValue(dbResp,
                          "response/data",
                          0,
                          0, 0,
                          &bs);
    if (p && bs) {
      GWEN_Buffer_AppendBytes(buf, p, bs);
    }
    else {
      DBG_WARN(LC_LOGDOMAIN, "No data in response");
    }
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoUpdateBinary(LC_CARD *card,
                         uint32_t flags,
                         int offset,
                         const char *ptr,
                         unsigned int size){
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;

  DBG_DEBUG(LC_LOGDOMAIN, "Writing binary %04x:%04x", offset, size);

  if (flags & LC_CARD_ISO_FLAGS_EFID_MASK) {
    if (offset>255) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Offset too high when implicitly selecting EF "
		"(%u)", flags);
      return LC_Client_ResultInvalid;
    }
    /* modify offset: highbyte is p1, lowbyte is p2 */
    offset|=0x8000;
    offset|=((flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<8);
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "offset", offset);
  if (ptr) {
    if (size) {
      GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                          "data", ptr, size);
    }
  }

  res=LC_Card_ExecCommand(card, "IsoUpdateBinary", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  /* successful */
  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoWriteBinary(LC_CARD *card,
                        uint32_t flags,
                        int offset,
                        const char *ptr,
                        unsigned int size){
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;

  DBG_DEBUG(LC_LOGDOMAIN, "Writing binary %04x:%04x", offset, size);

  if (flags & LC_CARD_ISO_FLAGS_EFID_MASK) {
    if (offset>255) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Offset too high when implicitly selecting EF "
                "(%u)", flags);
      return LC_Client_ResultInvalid;
    }
    /* modify offset: highbyte is p1, lowbyte is p2 */
    offset|=0x8000;
    offset|=((flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<8);
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "offset", offset);
  if (ptr) {
    if (size) {
      GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                          "data", ptr, size);
    }
  }

  res=LC_Card_ExecCommand(card, "IsoWriteBinary", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  /* successful */
  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoEraseBinary(LC_CARD *card,
                        uint32_t flags,
                        int offset,
                        unsigned int size){
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;

  DBG_DEBUG(LC_LOGDOMAIN, "Erasing binary %04x:%04x", offset, size);

  if (flags & LC_CARD_ISO_FLAGS_EFID_MASK) {
    if (offset>255) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Offset too high when implicitly selecting EF "
                "(%u)", flags);
      return LC_Client_ResultInvalid;
    }
    /* modify offset: highbyte is p1, lowbyte is p2 */
    offset|=0x8000;
    offset|=((flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<8);
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "offset", offset);
  if (size!=0)
    GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                        "len", size);

  res=LC_Card_ExecCommand(card, "IsoEraseBinary", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  /* successful */
  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoReadRecord(LC_CARD *card,
                       uint32_t flags,
                       int recNum,
                       GWEN_BUFFER *buf){
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  unsigned int bs;
  const void *p;
  unsigned char p2;

  p2=(flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<3;
  if ((flags & LC_CARD_ISO_FLAGS_RECSEL_MASK)!=
      LC_CARD_ISO_FLAGS_RECSEL_GIVEN) {
    DBG_ERROR(LC_LOGDOMAIN,
              "Invalid flags %u"
	      " (only RECSEL_GIVEN is allowed)", flags)
      return LC_Client_ResultInvalid;
  }
  p2|=0x04;

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "recNum", recNum);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "p2", p2);

  res=LC_Card_ExecCommand(card, "IsoReadRecord", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  /* successful */
  if (buf) {
    p=GWEN_DB_GetBinValue(dbResp,
                          "response/data",
                          0,
                          0, 0,
                          &bs);
    if (p && bs) {
      GWEN_Buffer_AppendBytes(buf, p, bs);
    }
    else {
      DBG_WARN(LC_LOGDOMAIN, "No data in response");
    }
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoWriteRecord(LC_CARD *card,
                        uint32_t flags,
                        int recNum,
                        const char *ptr,
                        unsigned int size) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "recNum", recNum);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "p2",
                      ((flags & LC_CARD_ISO_FLAGS_RECSEL_MASK)>>5) |
                      ((flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<3));
  if (ptr && size) {
    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                        "data", ptr, size);
  }
  res=LC_Card_ExecCommand(card, "IsoWriteRecord", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;

}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoUpdateRecord(LC_CARD *card,
                         uint32_t flags,
                         int recNum,
                         const char *ptr,
                         unsigned int size) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "recNum", recNum);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "p2",
                      ((flags & LC_CARD_ISO_FLAGS_RECSEL_MASK)>>5) |
                      ((flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<3));
  if (ptr && size) {
    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                        "data", ptr, size);
  }
  res=LC_Card_ExecCommand(card, "IsoUpdateRecord", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoAppendRecord(LC_CARD *card,
                         uint32_t flags,
                         const char *ptr,
                         unsigned int size) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "p2",
                      (flags & LC_CARD_ISO_FLAGS_EFID_MASK)<<3);

  if (ptr && size) {
    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                        "data", ptr, size);
  }
  res=LC_Card_ExecCommand(card, "IsoAppendRecord", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoVerifyPin(LC_CARD *card,
                      uint32_t flags,
                      const LC_PININFO *pi,
                      const unsigned char *ptr,
                      unsigned int size,
                      int *triesLeft) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  GWEN_DB_NODE *dbT;
  LC_CLIENT_RESULT res;
  const char *cmd;

  if (triesLeft)
    *triesLeft=-1;

  switch(LC_PinInfo_GetEncoding(pi)) {
  case GWEN_Crypt_PinEncoding_Bin:
    cmd="IsoVerifyPin_Bin";
    break;
  case GWEN_Crypt_PinEncoding_Bcd:
    cmd="IsoVerifyPin_Bcd";
    break;
  case GWEN_Crypt_PinEncoding_Ascii:
    cmd="IsoVerifyPin_Ascii";
    break;
  case GWEN_Crypt_PinEncoding_FPin2:
    cmd="IsoVerifyPin_Fpin2";
    break;
  default:
    DBG_ERROR(LC_LOGDOMAIN, "Unhandled pin encoding \"%s\"",
              GWEN_Crypt_PinEncoding_toString(LC_PinInfo_GetEncoding(pi)));
    return LC_Client_ResultInvalid;
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  dbT=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "pinInfo");
  assert(dbT);
  LC_PinInfo_toDb(pi, dbT);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "pid", LC_PinInfo_GetId(pi));

  if (ptr && size) {
    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                        "pin", ptr, size);
  }
  res=LC_Card_ExecCommand(card, cmd, dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    if (res==LC_Client_ResultCmdError && triesLeft) {
      if (LC_Card_GetLastSW1(card)==0x63) {
        int c;

        c=LC_Card_GetLastSW2(card);
        if (c>=0xc0)
          *triesLeft=(c & 0xf);
      }
    }
    return res;
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoModifyPin(LC_CARD *card,
                      uint32_t flags,
                      const LC_PININFO *pi,
                      const unsigned char *oldptr,
                      unsigned int oldsize,
                      const unsigned char *newptr,
                      unsigned int newsize,
                      int *triesLeft) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  GWEN_DB_NODE *dbT;
  LC_CLIENT_RESULT res;
  const char *cmd;

  if (triesLeft)
    *triesLeft=-1;

  switch(LC_PinInfo_GetEncoding(pi)) {
  case GWEN_Crypt_PinEncoding_Bin:
    cmd="IsoModifyPin_Bin";
    break;
  case GWEN_Crypt_PinEncoding_Bcd:
    cmd="IsoModifyPin_Bcd";
    break;
  case GWEN_Crypt_PinEncoding_Ascii:
    cmd="IsoModifyPin_Ascii";
    break;
  case GWEN_Crypt_PinEncoding_FPin2:
    cmd="IsoModifyPin_Fpin2";
    break;
  default:
    DBG_ERROR(LC_LOGDOMAIN, "Unhandled pin encoding \"%s\"",
              GWEN_Crypt_PinEncoding_toString(LC_PinInfo_GetEncoding(pi)));
    return LC_Client_ResultInvalid;
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  dbT=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "pinInfo");
  assert(dbT);
  LC_PinInfo_toDb(pi, dbT);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "pid", LC_PinInfo_GetId(pi));

  if (oldptr && oldsize) {
    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                        "oldpin", oldptr, oldsize);
  }
  if (newptr && newsize) {
    GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                        "newpin", newptr, newsize);
  }
  res=LC_Card_ExecCommand(card, cmd, dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    if (res==LC_Client_ResultCmdError && triesLeft) {
      if (LC_Card_GetLastSW1(card)==0x63) {
        int c;

        c=LC_Card_GetLastSW2(card);
        if (c>=0xc0)
          *triesLeft=(c & 0xf);
      }
    }
    return res;
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoPerformVerification(LC_CARD *card,
                                uint32_t flags,
                                const LC_PININFO *pi,
                                int *triesLeft) {
  GWEN_DB_NODE *dbReq=0;
  GWEN_DB_NODE *dbResp;
  GWEN_DB_NODE *dbT;
  LC_CLIENT_RESULT res;
  const char *cmd;

  if (triesLeft)
    *triesLeft=-1;

  switch(LC_PinInfo_GetEncoding(pi)) {
  case GWEN_Crypt_PinEncoding_Bin:
    cmd="IsoPerformVerification_Bin";
    break;
  case GWEN_Crypt_PinEncoding_Bcd:
    cmd="IsoPerformVerification_Bcd";
    break;
  case GWEN_Crypt_PinEncoding_Ascii:
    cmd="IsoPerformVerification_Ascii";
    break;
  case GWEN_Crypt_PinEncoding_FPin2:
    cmd="IsoPerformVerification_Fpin2";
    break;
  default:
    DBG_ERROR(LC_LOGDOMAIN, "Unhandled pin encoding \"%s\"",
              GWEN_Crypt_PinEncoding_toString(LC_PinInfo_GetEncoding(pi)));
    return LC_Client_ResultInvalid;
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  dbT=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "pinInfo");
  assert(dbT);
  LC_PinInfo_toDb(pi, dbT);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "pid", LC_PinInfo_GetId(pi));

  res=LC_Card_ExecCommand(card, cmd, dbReq, dbResp);
  DBG_DEBUG(LC_LOGDOMAIN, "ExecCommand returned %d", res);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    if (res==LC_Client_ResultCmdError && triesLeft) {
      if (LC_Card_GetLastSW1(card)==0x63) {
        int c;

        c=LC_Card_GetLastSW2(card);
        if (c>=0xc0)
          *triesLeft=(c & 0xf);
      }
    }
    return res;
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoPerformModification(LC_CARD *card,
                                uint32_t flags,
                                const LC_PININFO *pi,
                                int *triesLeft) {
  GWEN_DB_NODE *dbReq=0;
  GWEN_DB_NODE *dbResp;
  GWEN_DB_NODE *dbT;
  LC_CLIENT_RESULT res;
  const char *cmd;

  if (triesLeft)
    *triesLeft=-1;

  switch(LC_PinInfo_GetEncoding(pi)) {
  case GWEN_Crypt_PinEncoding_Bin:
    cmd="IsoPerformModification_Bin";
    break;
  case GWEN_Crypt_PinEncoding_Bcd:
    cmd="IsoPerformModification_Bcd";
    break;
  case GWEN_Crypt_PinEncoding_Ascii:
    cmd="IsoPerformModification_Ascii";
    break;
  case GWEN_Crypt_PinEncoding_FPin2:
    cmd="IsoPerformModification_Fpin2";
    break;
  default:
    DBG_ERROR(LC_LOGDOMAIN, "Unhandled pin encoding \"%s\"",
              GWEN_Crypt_PinEncoding_toString(LC_PinInfo_GetEncoding(pi)));
    return LC_Client_ResultInvalid;
  }

  dbReq=GWEN_DB_Group_new("request");
  dbResp=GWEN_DB_Group_new("response");
  dbT=GWEN_DB_GetGroup(dbReq, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "pinInfo");
  assert(dbT);
  LC_PinInfo_toDb(pi, dbT);
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "pid", LC_PinInfo_GetId(pi));

  res=LC_Card_ExecCommand(card, cmd, dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    if (res==LC_Client_ResultCmdError && triesLeft) {
      if (LC_Card_GetLastSW1(card)==0x63) {
        int c;

        c=LC_Card_GetLastSW2(card);
        if (c>=0xc0)
          *triesLeft=(c & 0xf);
      }
    }
    return res;
  }

  GWEN_DB_Group_free(dbResp);
  GWEN_DB_Group_free(dbReq);
  return res;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoManageSe(LC_CARD *card,
                     int tmpl, int kids, int kidp, int ar) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbResp;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *dbuf;

  assert(card);

  LC_Card_SetLastResult(card, 0, 0, 0, 0);

  dbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (kids) {
    GWEN_Buffer_AppendByte(dbuf, 0x84);
    GWEN_Buffer_AppendByte(dbuf, 1);
    GWEN_Buffer_AppendByte(dbuf, kids);
  }

  if (kidp) {
    GWEN_Buffer_AppendByte(dbuf, 0x83);
    GWEN_Buffer_AppendByte(dbuf, 1);
    GWEN_Buffer_AppendByte(dbuf, kidp);
  }

  if (ar!=-1) {
    GWEN_Buffer_AppendByte(dbuf, 0x80);
    GWEN_Buffer_AppendByte(dbuf, 1);
    GWEN_Buffer_AppendByte(dbuf, ar);
  }

  dbReq=GWEN_DB_Group_new("request");
  GWEN_DB_SetIntValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "template", tmpl);
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "data",
                      GWEN_Buffer_GetStart(dbuf),
                      GWEN_Buffer_GetUsedBytes(dbuf));
  GWEN_Buffer_free(dbuf);

  dbResp=GWEN_DB_Group_new("response");
  res=LC_Card_ExecCommand(card, "IsoManageSE", dbReq, dbResp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbResp);
    return res;
  }
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbResp);
  return LC_Client_ResultOk;
}




LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoEncipher(LC_CARD *card,
                     const char *ptr,
                     unsigned int size,
                     GWEN_BUFFER *codeBuf) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbRsp;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);

  /* put data */
  dbReq=GWEN_DB_Group_new("request");
  dbRsp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "data", ptr, size);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_ExecCommand(card, "IsoEncipher", dbReq, dbRsp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }

  /* extract the encoded data */
  p=GWEN_DB_GetBinValue(dbRsp, "response/data", 0, 0, 0, &bs);
  if (!p || !bs) {
    DBG_ERROR(LC_LOGDOMAIN, "No data returned by card");
    GWEN_DB_Dump(dbRsp, 2);
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }
  GWEN_Buffer_AppendBytes(codeBuf, p, bs);
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbRsp);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB
LC_Card__IsoDecipher(LC_CARD *card,
                     const char *ptr,
                     unsigned int size,
                     GWEN_BUFFER *plainBuf) {
  GWEN_DB_NODE *dbReq;
  GWEN_DB_NODE *dbRsp;
  LC_CLIENT_RESULT res;
  const void *p;
  unsigned int bs;

  assert(card);

  /* put hash */
  dbReq=GWEN_DB_Group_new("request");
  dbRsp=GWEN_DB_Group_new("response");
  GWEN_DB_SetBinValue(dbReq, GWEN_DB_FLAGS_DEFAULT,
                      "data", ptr, size);
  LC_Card_SetLastResult(card, 0, 0, 0, 0);
  res=LC_Card_ExecCommand(card, "IsoDecipher", dbReq, dbRsp);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }

  /* extract the decoded data */
  p=GWEN_DB_GetBinValue(dbRsp, "response/data", 0, 0, 0, &bs);
  if (!p || !bs) {
    DBG_ERROR(LC_LOGDOMAIN, "No data returned by card");
    GWEN_DB_Group_free(dbReq);
    GWEN_DB_Group_free(dbRsp);
    return res;
  }
  GWEN_Buffer_AppendBytes(plainBuf, p, bs);
  GWEN_DB_Group_free(dbReq);
  GWEN_DB_Group_free(dbRsp);

  return LC_Client_ResultOk;
}







LC_CLIENT_RESULT LC_Card_IsoReadBinary(LC_CARD *card,
                                       uint32_t flags,
				       int offset,
                                       int size,
                                       GWEN_BUFFER *buf) {
  assert(card);
  if (card->readBinaryFn)
    return card->readBinaryFn(card, flags, offset, size, buf);
  else
    return LC_Card__IsoReadBinary(card, flags, offset, size, buf);
}



LC_CLIENT_RESULT LC_Card_IsoWriteBinary(LC_CARD *card,
					uint32_t flags,
					int offset,
					const char *ptr,
                                        unsigned int size) {
  assert(card);
  if (card->writeBinaryFn)
    return card->writeBinaryFn(card, flags, offset, ptr, size);
  else
    return LC_Card__IsoWriteBinary(card, flags, offset, ptr, size);
}



LC_CLIENT_RESULT LC_Card_IsoUpdateBinary(LC_CARD *card,
					 uint32_t flags,
					 int offset,
					 const char *ptr,
                                         unsigned int size) {
  assert(card);
  if (card->updateBinaryFn)
    return card->updateBinaryFn(card, flags, offset, ptr, size);
  else
    return LC_Card__IsoUpdateBinary(card, flags, offset, ptr, size);
}




LC_CLIENT_RESULT LC_Card_IsoEraseBinary(LC_CARD *card,
					uint32_t flags,
					int offset,
                                        unsigned int size) {
  assert(card);
  if (card->eraseBinaryFn)
    return card->eraseBinaryFn(card, flags, offset, size);
  else
    return LC_Card__IsoEraseBinary(card, flags, offset, size);
}



LC_CLIENT_RESULT LC_Card_IsoReadRecord(LC_CARD *card,
				       uint32_t flags,
				       int recNum,
                                       GWEN_BUFFER *buf) {
  assert(card);
  if (card->readRecordFn)
    return card->readRecordFn(card, flags, recNum, buf);
  else
    return LC_Card__IsoReadRecord(card, flags, recNum, buf);
}



LC_CLIENT_RESULT LC_Card_IsoWriteRecord(LC_CARD *card,
					uint32_t flags,
					int recNum,
					const char *ptr,
                                        unsigned int size) {
  assert(card);
  if (card->writeRecordFn)
    return card->writeRecordFn(card, flags, recNum, ptr, size);
  else
    return LC_Card__IsoWriteRecord(card, flags, recNum, ptr, size);
}



LC_CLIENT_RESULT LC_Card_IsoAppendRecord(LC_CARD *card,
                                         uint32_t flags,
                                         const char *ptr,
                                         unsigned int size) {
  assert(card);
  if (card->appendRecordFn)
    return card->appendRecordFn(card, flags, ptr, size);
  else
    return LC_Card__IsoAppendRecord(card, flags, ptr, size);
}




LC_CLIENT_RESULT LC_Card_IsoUpdateRecord(LC_CARD *card,
					 uint32_t flags,
					 int recNum,
					 const char *ptr,
                                         unsigned int size) {
  assert(card);
  if (card->updateRecordFn)
    return card->updateRecordFn(card, flags, recNum, ptr, size);
  else
    return LC_Card__IsoUpdateRecord(card, flags, recNum, ptr, size);
}



LC_CLIENT_RESULT LC_Card_IsoVerifyPin(LC_CARD *card,
                                      uint32_t flags,
                                      const LC_PININFO *pi,
                                      const unsigned char *ptr,
                                      unsigned int size,
                                      int *triesLeft) {
  assert(card);
  if (card->verifyPinFn)
    return card->verifyPinFn(card, flags, pi, ptr, size,
			     triesLeft);
  else
    return LC_Card__IsoVerifyPin(card, flags, pi, ptr, size,
                                 triesLeft);
}



LC_CLIENT_RESULT LC_Card_IsoModifyPin(LC_CARD *card,
                                      uint32_t flags,
                                      const LC_PININFO *pi,
                                      const unsigned char *oldptr,
                                      unsigned int oldsize,
                                      const unsigned char *newptr,
                                      unsigned int newsize,
                                      int *triesLeft) {
  assert(card);
  if (card->modifyPinFn)
    return card->modifyPinFn(card, flags, pi,
                             oldptr, oldsize,
                             newptr, newsize,
                             triesLeft);
  else
    return LC_Card__IsoModifyPin(card, flags, pi,
				 oldptr, oldsize,
                                 newptr, newsize,
                                 triesLeft);
}



LC_CLIENT_RESULT LC_Card_IsoPerformVerification(LC_CARD *card,
                                                uint32_t flags,
                                                const LC_PININFO *pi,
                                                int *triesLeft) {
  assert(card);
  if (card->performVerificationFn)
    return card->performVerificationFn(card, flags, pi,
                                       triesLeft);
  else
    return LC_Card__IsoPerformVerification(card, flags, pi,
                                           triesLeft);
}



LC_CLIENT_RESULT LC_Card_IsoPerformModification(LC_CARD *card,
                                                uint32_t flags,
                                                const LC_PININFO *pi,
                                                int *triesLeft) {
  assert(card);
  if (card->performModificationFn)
    return card->performModificationFn(card, flags, pi,
                                       triesLeft);
  else
    return LC_Card__IsoPerformModification(card, flags, pi,
                                           triesLeft);
}



LC_CLIENT_RESULT LC_Card_IsoManageSe(LC_CARD *card,
                                     int tmpl, int kids, int kidp, int ar) {
  assert(card);
  if (card->manageSeFn)
    return card->manageSeFn(card, tmpl, kids, kidp, ar);
  else
    return LC_Card__IsoManageSe(card, tmpl, kids, kidp, ar);
}



LC_CLIENT_RESULT LC_Card_IsoEncipher(LC_CARD *card,
                                     const char *ptr,
                                     unsigned int size,
                                     GWEN_BUFFER *codeBuf) {
  assert(card);
  if (card->encipherFn)
    return card->encipherFn(card, ptr, size, codeBuf);
  else
    return LC_Card__IsoEncipher(card, ptr, size, codeBuf);
}



LC_CLIENT_RESULT LC_Card_IsoDecipher(LC_CARD *card,
                                     const char *ptr,
                                     unsigned int size,
                                     GWEN_BUFFER *plainBuf) {
  assert(card);
  if (card->decipherFn)
    return card->decipherFn(card, ptr, size, plainBuf);
  else
    return LC_Card__IsoDecipher(card, ptr, size, plainBuf);
}



LC_CLIENT_RESULT LC_Card_IsoSign(LC_CARD *card,
                                 const char *ptr,
                                 unsigned int size,
                                 GWEN_BUFFER *sigBuf) {
  assert(card);
  if (card->signFn)
    return card->signFn(card, ptr, size, sigBuf);
  else
    return LC_Client_ResultNotSupported;
}



LC_CLIENT_RESULT LC_Card_IsoVerify(LC_CARD *card,
                                   const char *dptr,
                                   unsigned int dsize,
                                   const char *sigptr,
                                   unsigned int sigsize) {
  assert(card);
  if (card->verifyFn)
    return card->verifyFn(card, dptr, dsize, sigptr, sigsize);
  else
    return LC_Client_ResultNotSupported;
}







void LC_Card_SetIsoReadBinaryFn(LC_CARD *card, LC_CARD_ISOREADBINARY_FN f) {
  assert(card);
  card->readBinaryFn=f;
}



void LC_Card_SetIsoWriteBinaryFn(LC_CARD *card, LC_CARD_ISOWRITEBINARY_FN f){
  assert(card);
  card->writeBinaryFn=f;
}



void LC_Card_SetIsoUpdateBinaryFn(LC_CARD *card,
                                  LC_CARD_ISOUPDATEBINARY_FN f){
  assert(card);
  card->updateBinaryFn=f;
}



void LC_Card_SetIsoEraseBinaryFn(LC_CARD *card, LC_CARD_ISOERASEBINARY_FN f) {
  assert(card);
  card->eraseBinaryFn=f;
}



void LC_Card_SetIsoReadRecordFn(LC_CARD *card, LC_CARD_ISOREADRECORD_FN f){
  assert(card);
  card->readRecordFn=f;
}



void LC_Card_SetIsoWriteRecordFn(LC_CARD *card, LC_CARD_ISOWRITERECORD_FN f){
  assert(card);
  card->writeRecordFn=f;
}



void LC_Card_SetIsoUpdateRecordFn(LC_CARD *card,
                                  LC_CARD_ISOUPDATERECORD_FN f) {
  assert(card);
  card->updateRecordFn=f;
}



void LC_Card_SetIsoAppendRecordFn(LC_CARD *card,
                                  LC_CARD_ISOAPPENDRECORD_FN f){
  assert(card);
  card->appendRecordFn=f;
}



void LC_Card_SetIsoVerifyPinFn(LC_CARD *card, LC_CARD_ISOVERIFYPIN_FN f) {
  assert(card);
  card->verifyPinFn=f;
}



void LC_Card_SetIsoModifyPinFn(LC_CARD *card, LC_CARD_ISOMODIFYPIN_FN f) {
  assert(card);
  card->modifyPinFn=f;
}



void LC_Card_SetIsoPerformVerificationFn(LC_CARD *card,
                                         LC_CARD_ISOPERFORMVERIFICATION_FN f){
  assert(card);
  card->performVerificationFn=f;
}



void LC_Card_SetIsoPerformModificationFn(LC_CARD *card,
                                         LC_CARD_ISOPERFORMMODIFICATION_FN f){
  assert(card);
  card->performModificationFn=f;
}



void LC_Card_SetIsoManageSeFn(LC_CARD *card, LC_CARD_ISOMANAGESE_FN f) {
  assert(card);
  card->manageSeFn=f;
}



void LC_Card_SetIsoSignFn(LC_CARD *card, LC_CARD_ISOSIGN_FN f) {
  assert(card);
  card->signFn=f;
}



void LC_Card_SetIsoVerifyFn(LC_CARD *card, LC_CARD_ISOVERIFY_FN f) {
  assert(card);
  card->verifyFn=f;
}



void LC_Card_SetIsoEncipherFn(LC_CARD *card, LC_CARD_ISOENCIPHER_FN f) {
  assert(card);
  card->encipherFn=f;
}



void LC_Card_SetIsoDecipherFn(LC_CARD *card, LC_CARD_ISODECIPHER_FN f) {
  assert(card);
  card->decipherFn=f;
}



