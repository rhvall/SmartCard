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

#include "ct_card.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/ct_be.h>
#include <gwenhywfar/cryptdefs.h>
#include <gwenhywfar/gui.h>



int LC_Crypt_Token_ResultToError(LC_CLIENT_RESULT res) {
  int rv;

  switch(res) {
  case LC_Client_ResultOk:
    rv=0;
    break;
  case LC_Client_ResultWait:
    rv=GWEN_ERROR_TIMEOUT;
    break;
  case LC_Client_ResultIpcError:
  case LC_Client_ResultCmdError:
  case LC_Client_ResultDataError:
    rv=GWEN_ERROR_IO;
    break;

  case LC_Client_ResultAborted:
    rv=GWEN_ERROR_USER_ABORTED;
    break;

  case LC_Client_ResultInvalid:
    rv=GWEN_ERROR_INVALID;
    break;

  case LC_Client_ResultNoData:
    rv=GWEN_ERROR_NO_DATA;
    break;

  case LC_Client_ResultCardRemoved:
    rv=GWEN_ERROR_REMOVED;
    break;

  case LC_Client_ResultNotSupported:
    rv=GWEN_ERROR_NOT_SUPPORTED;
    break;

  case LC_Client_ResultInternal:
  case LC_Client_ResultGeneric:
  default:
    rv=GWEN_ERROR_GENERIC;
    break;
  }

  return rv;
}


static
int LC_Crypt_Token__GetPin(GWEN_CRYPT_TOKEN *ct,
			   LC_CARD *hcard,
			   int pid,
			   GWEN_CRYPT_PINTYPE pt,
			   GWEN_CRYPT_PINENCODING pe,
			   uint32_t flags,
			   unsigned char *buffer,
			   unsigned int minLength,
			   unsigned int maxLength,
			   unsigned int *pinLength,
			   uint32_t guiid) {
  int rv;

  rv=GWEN_Crypt_Token_GetPin(ct,
			     pt, pe, flags,
			     buffer,
			     minLength, maxLength,
			     pinLength,
			     guiid);
  if (rv==GWEN_ERROR_DEFAULT_VALUE) {
    LC_CLIENT_RESULT res;

    res=LC_Card_GetInitialPin(hcard, pid, buffer, maxLength,
                              pinLength);
    if (res) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
      return GWEN_ERROR_IO;
    }

    if (pe!=GWEN_Crypt_PinEncoding_Ascii) {
      rv=GWEN_Crypt_TransformPin(GWEN_Crypt_PinEncoding_Ascii,
				 pe,
				 buffer,
				 maxLength,
				 pinLength);
      if (rv) {
        DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  }
  else if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



static
int LC_Crypt_Token__ChangePin(GWEN_CRYPT_TOKEN *ct,
			      LC_CARD *hcard,
			      GWEN_CRYPT_PINTYPE pt,
			      int initial,
			      uint32_t guiid) {
  LC_CLIENT_RESULT res;
  LC_PININFO *pi;
  int maxErrors;
  int currentErrors;

  assert(hcard);

  if (pt==GWEN_Crypt_PinType_Manage)
    pi=LC_Card_GetPinInfoByName(hcard, "eg_pin");
  else
    pi=LC_Card_GetPinInfoByName(hcard, "ch_pin");
  assert(pi);

  if (LC_PinInfo_GetAllowChange(pi)==0) {
    DBG_ERROR(LC_LOGDOMAIN,
              "Change of Pin is not allowed for this card");
    LC_PinInfo_free(pi);
    return GWEN_ERROR_INVALID;
  }

  res=LC_Card_GetPinStatus(hcard,
                           LC_PinInfo_GetId(pi),
                           &maxErrors,
                           &currentErrors);
  if (res!=LC_Client_ResultNotSupported) {
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Unable to read status of pin %x (%d)",
                LC_PinInfo_GetId(pi),
                res);
      LC_PinInfo_free(pi);
      return LC_Crypt_Token_ResultToError(res);
    }

    if ((currentErrors!=maxErrors) &&
        (GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY)){
      DBG_ERROR(LC_LOGDOMAIN,
                "Bad pin entered at least once before, aborting");
      LC_PinInfo_free(pi);
      return GWEN_ERROR_ABORTED;
    }
  }

  if (!initial && (pt!=GWEN_Crypt_PinType_Manage) &&
      (LC_Card_GetReaderFlags(hcard) & LC_READER_FLAGS_KEYPAD)) {
    int mres;
    int triesLeft=-1;

    DBG_INFO(LC_LOGDOMAIN,"Terminal has a keypad, will ask for pin.");
    /* tell the user about pin verification */
    mres=GWEN_Crypt_Token_BeginEnterPin(ct, pt, guiid);
    if (mres) {
      DBG_ERROR(LC_LOGDOMAIN, "Error in user interaction");
      LC_PinInfo_free(pi);
      return mres;
    }

    res=LC_Card_IsoPerformModification(hcard, 0, pi, &triesLeft);

    if (res!=LC_Client_ResultOk) {
      /* tell the user about end of pin verification */
      GWEN_Crypt_Token_EndEnterPin(ct, pt, 0, guiid);
      DBG_ERROR(LC_LOGDOMAIN, "sw1=%02x sw2=%02x (%s)",
                LC_Card_GetLastSW1(hcard),
                LC_Card_GetLastSW2(hcard),
                LC_Card_GetLastText(hcard));
      LC_PinInfo_free(pi);

      if (LC_Card_GetLastSW1(hcard)==0x63) {
	switch (LC_Card_GetLastSW2(hcard)) {
        case 0xc0: /* no error left */
          return GWEN_ERROR_BAD_PIN_0_LEFT;
        case 0xc1: /* one left */
          return GWEN_ERROR_BAD_PIN_1_LEFT;
        case 0xc2: /* two left */
          return GWEN_ERROR_BAD_PIN_2_LEFT;
        default:   /* unknown error */
          return GWEN_ERROR_BAD_PIN;
        } // switch
      }
      else if (LC_Card_GetLastSW1(hcard)==0x69 &&
               LC_Card_GetLastSW2(hcard)==0x83) {
        DBG_ERROR(LC_LOGDOMAIN, "Card unusable");
        return GWEN_ERROR_IO;
      }
      else if (LC_Card_GetLastSW1(hcard)==0x64 &&
               LC_Card_GetLastSW2(hcard)==0x01) {
        DBG_ERROR(LC_LOGDOMAIN, "Aborted by user");
        return GWEN_ERROR_USER_ABORTED;
      }
      else {
        return GWEN_ERROR_IO;
      }
    } /* if not ok */
    else {
      /* PIN ok */
      DBG_INFO(LC_LOGDOMAIN, "Pin ok");
      GWEN_Crypt_Token_EndEnterPin(ct, pt, 1, guiid);
    }
  } /* if hasKeyPad */
  else {
    unsigned char pinBuffer1[64];
    unsigned char pinBuffer2[64];
    unsigned int pinLength1;
    unsigned int pinLength2;
    int mres;
    int pinMaxLen;
    uint32_t pflags=0;
    GWEN_CRYPT_PINENCODING pe;
    int triesLeft=-1;

    DBG_INFO(LC_LOGDOMAIN, "No keypad (or disabled), will ask for PIN");
    memset(pinBuffer1, 0, sizeof(pinBuffer1));
    memset(pinBuffer2, 0, sizeof(pinBuffer2));

    pe=LC_PinInfo_GetEncoding(pi);
    if (pt==GWEN_Crypt_PinType_Manage)
      pflags|=GWEN_GUI_INPUT_FLAGS_ALLOW_DEFAULT;
    pflags|=GWEN_GUI_INPUT_FLAGS_NUMERIC;
    pinLength1=0;
    pinMaxLen=LC_PinInfo_GetMaxLength(pi);
    if (!pinMaxLen || pinMaxLen>sizeof(pinBuffer1)-1)
      pinMaxLen=sizeof(pinBuffer1)-1;
    if (initial) {
      LC_CLIENT_RESULT res;

      res=LC_Card_GetInitialPin(hcard,
                                LC_PinInfo_GetId(pi),
                                pinBuffer1, pinMaxLen,
                                &pinLength1);
      if (res) {
        DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
        mres=GWEN_ERROR_IO;
      }
      else
        mres=0;
    }
    else
      mres=LC_Crypt_Token__GetPin(ct,
				  hcard,
				  LC_PinInfo_GetId(pi),
				  pt,
				  pe,
				  pflags,
				  pinBuffer1,
				  LC_PinInfo_GetMinLength(pi),
				  pinMaxLen,
				  &pinLength1,
				  guiid);
    if (mres!=0) {
      DBG_ERROR(LC_LOGDOMAIN, "Error asking for PIN, aborting");
      memset(pinBuffer1, 0, sizeof(pinBuffer1));
      memset(pinBuffer2, 0, sizeof(pinBuffer2));
      LC_PinInfo_free(pi);
      return mres;
    }

    if (pinLength1<pinMaxLen && LC_PinInfo_GetFiller(pi)) {
      int i;
      unsigned char c;

      c=(unsigned char)LC_PinInfo_GetFiller(pi);
      for (i=pinLength1; i<pinMaxLen; i++)
        pinBuffer1[i]=c;
      pinLength1=pinMaxLen;
    }

    /* get new pin */
    if (pt==GWEN_Crypt_PinType_Manage)
      pflags|=GWEN_GUI_INPUT_FLAGS_ALLOW_DEFAULT;
    pflags|=GWEN_GUI_INPUT_FLAGS_NUMERIC;
    pflags|=GWEN_GUI_INPUT_FLAGS_CONFIRM;
    pinLength2=0;
    pinMaxLen=LC_PinInfo_GetMaxLength(pi);
    if (!pinMaxLen || pinMaxLen>sizeof(pinBuffer2)-1)
      pinMaxLen=sizeof(pinBuffer2)-1;
    mres=LC_Crypt_Token__GetPin(ct,
				hcard,
				LC_PinInfo_GetId(pi),
				pt,
				pe,
				pflags,
				pinBuffer2,
				LC_PinInfo_GetMinLength(pi),
				pinMaxLen,
				&pinLength2,
				guiid);
    if (mres!=0) {
      DBG_ERROR(LC_LOGDOMAIN, "Error asking for PIN, aborting");
      memset(pinBuffer1, 0, sizeof(pinBuffer1));
      memset(pinBuffer2, 0, sizeof(pinBuffer2));
      LC_PinInfo_free(pi);
      return mres;
    }

    if (pinLength2<pinMaxLen && LC_PinInfo_GetFiller(pi)) {
      int i;
      unsigned char c;

      c=(unsigned char)LC_PinInfo_GetFiller(pi);
      for (i=pinLength2; i<pinMaxLen; i++)
        pinBuffer2[i]=c;
      pinLength2=pinMaxLen;
    }

    DBG_INFO(LC_LOGDOMAIN, "Modifying the PIN");
    res=LC_Card_IsoModifyPin(hcard,
                             0, pi,
                             pinBuffer1,
                             pinLength1,
                             pinBuffer2,
                             pinLength2,
                             &triesLeft);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "sw1=%02x sw2=%02x (%s)",
                LC_Card_GetLastSW1(hcard),
                LC_Card_GetLastSW2(hcard),
                LC_Card_GetLastText(hcard));
      LC_PinInfo_free(pi);

      if (LC_Card_GetLastSW1(hcard)==0x63) {
        /* TODO: Set Pin status */
        switch (LC_Card_GetLastSW2(hcard)) {
        case 0xc0: /* no error left */
	  return GWEN_ERROR_BAD_PIN_0_LEFT;
        case 0xc1: /* one left */
          return GWEN_ERROR_BAD_PIN_1_LEFT;
        case 0xc2: /* two left */
          return GWEN_ERROR_BAD_PIN_2_LEFT;
        default:
          return GWEN_ERROR_BAD_PIN;
        } // switch
      }
      else if (LC_Card_GetLastSW1(hcard)==0x69 &&
               LC_Card_GetLastSW2(hcard)==0x83) {
        /* TODO: Set Pin status */
        DBG_ERROR(LC_LOGDOMAIN, "Card unusable");
        return GWEN_ERROR_IO;
      }
      else if (LC_Card_GetLastSW1(hcard)==0x64 &&
               LC_Card_GetLastSW2(hcard)==0x01) {
        return GWEN_ERROR_USER_ABORTED;
      }
      else {
        DBG_ERROR(LC_LOGDOMAIN, "Unknown error");
        return GWEN_ERROR_IO;
      }
    } // if not ok
    else {
      DBG_INFO(LC_LOGDOMAIN, "PIN ok");
      /* TODO: Set Pin Status */
    }
  } // if no keyPad
  LC_PinInfo_free(pi);

  return 0;
}



static
int LC_Crypt_Token__EnterPin(GWEN_CRYPT_TOKEN *ct,
			     LC_CARD *hcard,
			     GWEN_CRYPT_PINTYPE pt,
			     uint32_t guiid) {
  LC_CLIENT_RESULT res;
  LC_PININFO *pi;
  int maxErrors;
  int currentErrors;

  assert(hcard);

  if (pt==GWEN_Crypt_PinType_Manage) {
    pi=LC_Card_GetPinInfoByName(hcard, "eg_pin");
  }
  else {
    pi=LC_Card_GetPinInfoByName(hcard, "ch_pin");
  }
  assert(pi);

#if 0
  if (pi) {
    GWEN_DB_NODE *dbDEBUG;

    dbDEBUG=GWEN_DB_Group_new("PinInfo");
    LC_PinInfo_toDb(pi, dbDEBUG);
    GWEN_DB_Dump(dbDEBUG, stderr, 2);
    GWEN_DB_Group_free(dbDEBUG);
  }
#endif

  res=LC_Card_GetPinStatus(hcard,
                           LC_PinInfo_GetId(pi),
                           &maxErrors,
                           &currentErrors);
  if (res!=LC_Client_ResultNotSupported) {
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN,
                "Unable to read status of pin %x (%d)",
                LC_PinInfo_GetId(pi),
                res);
      LC_PinInfo_free(pi);
      return LC_Crypt_Token_ResultToError(res);
    }

    if ((currentErrors!=maxErrors) &&
        !(GWEN_Crypt_Token_GetModes(ct) &
	  GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY)
       ){
      DBG_ERROR(LC_LOGDOMAIN,
                "Bad pin entered at least once before, aborting");
      LC_PinInfo_free(pi);
      return GWEN_ERROR_ABORTED;
    }
  }
  else {
    DBG_INFO(LC_LOGDOMAIN,
	     "Unable to read pin status for pin %02x (not supported)",
	     LC_PinInfo_GetId(pi));
  }

  if ((pt!=GWEN_Crypt_PinType_Manage) &&
      (LC_Card_GetReaderFlags(hcard) & LC_READER_FLAGS_KEYPAD) &&
      !(GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY)) {
    uint32_t bid;
    int triesLeft=-1;

    DBG_INFO(LC_LOGDOMAIN,"Terminal has a keypad, will ask for pin.");
    /* tell the user about pin verification */
    bid=GWEN_Crypt_Token_BeginEnterPin(ct, pt, guiid);
    if (bid==0) {
      DBG_ERROR(LC_LOGDOMAIN, "Error in user interaction");
      LC_PinInfo_free(pi);
      return GWEN_ERROR_GENERIC;
    }

    res=LC_Card_IsoPerformVerification(hcard, 0, pi, &triesLeft);

    if (res!=LC_Client_ResultOk) {
      /* tell the user about end of pin verification */
      GWEN_Crypt_Token_EndEnterPin(ct, pt, 0, bid);
      DBG_ERROR(LC_LOGDOMAIN, "sw1=%02x sw2=%02x (%s)",
                LC_Card_GetLastSW1(hcard),
                LC_Card_GetLastSW2(hcard),
                LC_Card_GetLastText(hcard));
      LC_PinInfo_free(pi);

      if (LC_Card_GetLastSW1(hcard)==0x63) {
	switch (LC_Card_GetLastSW2(hcard)) {
        case 0xc0: /* no error left */
	  return GWEN_ERROR_BAD_PIN_0_LEFT;
        case 0xc1: /* one left */
          return GWEN_ERROR_BAD_PIN_1_LEFT;
        case 0xc2: /* two left */
          return GWEN_ERROR_BAD_PIN_2_LEFT;
        default:   /* unknown error */
          return GWEN_ERROR_BAD_PIN;
        } // switch
      }
      else if (LC_Card_GetLastSW1(hcard)==0x69 &&
               LC_Card_GetLastSW2(hcard)==0x83) {
        DBG_ERROR(LC_LOGDOMAIN, "Card unusable");
        return GWEN_ERROR_IO;
      }
      else if (LC_Card_GetLastSW1(hcard)==0x64 &&
               LC_Card_GetLastSW2(hcard)==0x01) {
        DBG_ERROR(LC_LOGDOMAIN, "Aborted by user");
        return GWEN_ERROR_USER_ABORTED;
      }
      else {
        if (triesLeft>=0) {
          switch (triesLeft) {
          case 0: /* no error left */
            return GWEN_ERROR_BAD_PIN_0_LEFT;
          case 1: /* one left */
            return GWEN_ERROR_BAD_PIN_1_LEFT;
          case 2: /* two left */
            return GWEN_ERROR_BAD_PIN_2_LEFT;
          default:   /* unknown count */
            return GWEN_ERROR_BAD_PIN;
          } // switch
        }

        return GWEN_ERROR_IO;
      }
    } /* if not ok */
    else {
      /* PIN ok */
      DBG_INFO(LC_LOGDOMAIN, "Pin ok");
      GWEN_Crypt_Token_EndEnterPin(ct, pt, 1, bid);
    }
  } /* if hasKeyPad */
  else {
    unsigned char pinBuffer[64];
    int mres;
    int pinMaxLen;
    unsigned int pinLength;
    unsigned int origPinLength;
    uint32_t pflags=0;
    GWEN_CRYPT_PINENCODING pe;
    int triesLeft=-1;

    DBG_INFO(LC_LOGDOMAIN, "No keypad (or disabled), will ask for PIN");
    memset(pinBuffer, 0, sizeof(pinBuffer));

    pe=LC_PinInfo_GetEncoding(pi);
    if (pt==GWEN_Crypt_PinType_Manage)
      pflags|=GWEN_GUI_INPUT_FLAGS_ALLOW_DEFAULT;
    pflags|=GWEN_GUI_INPUT_FLAGS_NUMERIC;
    pinLength=0;
    pinMaxLen=LC_PinInfo_GetMaxLength(pi);
    if (!pinMaxLen || pinMaxLen>sizeof(pinBuffer)-1)
      pinMaxLen=sizeof(pinBuffer)-1;
    mres=LC_Crypt_Token__GetPin(ct,
				hcard,
				LC_PinInfo_GetId(pi),
				pt,
				pe,
				pflags,
				pinBuffer,
				LC_PinInfo_GetMinLength(pi),
				pinMaxLen,
				&pinLength,
				guiid);
    if (mres!=0) {
      DBG_ERROR(LC_LOGDOMAIN, "Error asking for PIN, aborting");
      memset(pinBuffer, 0, sizeof(pinBuffer));
      LC_PinInfo_free(pi);
      return mres;
    }
    origPinLength=pinLength;

    if (pinLength<pinMaxLen && LC_PinInfo_GetFiller(pi)) {
      int i;
      unsigned char c;

      c=(unsigned char)LC_PinInfo_GetFiller(pi);
      for (i=pinLength; i<pinMaxLen; i++)
        pinBuffer[i]=c;
      pinLength=pinMaxLen;
    }

    DBG_INFO(LC_LOGDOMAIN, "Verifying the PIN");
    res=LC_Card_IsoVerifyPin(hcard,
                             0,
                             pi,
                             pinBuffer,
                             pinLength,
                             &triesLeft);
    if (res!=LC_Client_ResultOk) {
      DBG_ERROR(LC_LOGDOMAIN, "sw1=%02x sw2=%02x (%s)",
                LC_Card_GetLastSW1(hcard),
                LC_Card_GetLastSW2(hcard),
                LC_Card_GetLastText(hcard));

      LC_PinInfo_free(pi);

      if (LC_Card_GetLastSW1(hcard)==0x63) {
        /* set pin status */
	GWEN_Crypt_Token_SetPinStatus(ct,
				      pt,
				      pe,
				      pflags,
				      pinBuffer,
				      origPinLength,
				      0,
				      guiid);

        switch (LC_Card_GetLastSW2(hcard)) {
        case 0xc0: /* no error left */
          return GWEN_ERROR_BAD_PIN_0_LEFT;
        case 0xc1: /* one left */
          return GWEN_ERROR_BAD_PIN_1_LEFT;
        case 0xc2: /* two left */
          return GWEN_ERROR_BAD_PIN_2_LEFT;
        default:
          return GWEN_ERROR_BAD_PIN;
        } // switch
      }
      else if (LC_Card_GetLastSW1(hcard)==0x69 &&
               LC_Card_GetLastSW2(hcard)==0x83) {
        /* set pin status */
	GWEN_Crypt_Token_SetPinStatus(ct,
				      pt,
				      pe,
				      pflags,
				      pinBuffer,
				      origPinLength,
				      0,
				      guiid);
        DBG_ERROR(LC_LOGDOMAIN, "Card unusable");
        return GWEN_ERROR_IO;
      }
      else if (LC_Card_GetLastSW1(hcard)==0x64 &&
               LC_Card_GetLastSW2(hcard)==0x01) {
        return GWEN_ERROR_USER_ABORTED;
      }
      else {
        if (triesLeft>=0) {
          /* set pin status */
	  GWEN_Crypt_Token_SetPinStatus(ct,
					pt,
					pe,
					pflags,
					pinBuffer,
					origPinLength,
					0,
					guiid);
	  switch (triesLeft) {
          case 0: /* no error left */
            return GWEN_ERROR_BAD_PIN_0_LEFT;
          case 1: /* one left */
	    return GWEN_ERROR_BAD_PIN_1_LEFT;
          case 2: /* two left */
            return GWEN_ERROR_BAD_PIN_2_LEFT;
          default:   /* unknown count */
            return GWEN_ERROR_BAD_PIN;
          } // switch
        }
        DBG_ERROR(LC_LOGDOMAIN, "Unknown error");
        return GWEN_ERROR_IO;
      }
    } // if not ok
    else {
      DBG_INFO(LC_LOGDOMAIN, "PIN ok");
      /* set pin status */
      GWEN_Crypt_Token_SetPinStatus(ct,
				    pt,
				    pe,
				    pflags,
				    pinBuffer,
				    origPinLength,
				    1,
				    guiid);
    }
  } // if no keyPad
  LC_PinInfo_free(pi);

  return 0;
}



int LC_Crypt_Token_VerifyPin(GWEN_CRYPT_TOKEN *ct,
			     LC_CARD *hcard,
			     GWEN_CRYPT_PINTYPE pt,
			     uint32_t guiid) {
  int rv;

  /* enter pin */
  rv=LC_Crypt_Token__EnterPin(ct, hcard, pt, guiid);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "Error in pin input");
    return rv;
  }

  return 0;
}



int LC_Crypt_Token_ChangePin(GWEN_CRYPT_TOKEN *ct,
			     LC_CARD *hcard,
			     GWEN_CRYPT_PINTYPE pt,
			     int initial,
			     uint32_t guiid) {
  int rv;

  if (pt!=GWEN_Crypt_PinType_Access &&
      pt==GWEN_Crypt_PinType_Manage) {
    DBG_ERROR(LC_LOGDOMAIN, "Unknown pin type \"%s\"",
              GWEN_Crypt_PinType_toString(pt));
    return GWEN_ERROR_INVALID;
  }

  /* enter pin */
  rv=LC_Crypt_Token__ChangePin(ct, hcard, pt, initial, guiid);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "Error in pin input");
    return rv;
  }

  return 0;
}




