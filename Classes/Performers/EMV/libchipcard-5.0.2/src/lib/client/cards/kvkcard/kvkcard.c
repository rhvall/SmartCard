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


#include "kvkcard_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>
#include <chipcard/chipcard.h>
#include <chipcard/tlv.h>
#include <chipcard/cards/memorycard.h>


GWEN_INHERIT(LC_CARD, LC_KVKCARD)



int LC_KVKCard_ExtendCard(LC_CARD *card){
  LC_KVKCARD *kvk;
  int rv;

  rv=LC_MemoryCard_ExtendCard(card);
  if (rv) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not extend card as memory card");
    return rv;
  }

  GWEN_NEW_OBJECT(LC_KVKCARD, kvk);

  kvk->openFn=LC_Card_GetOpenFn(card);
  kvk->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, LC_KVKCard_Open);
  LC_Card_SetCloseFn(card, LC_KVKCard_Close);

  GWEN_INHERIT_SETDATA(LC_CARD, LC_KVKCARD, card, kvk,
                       LC_KVKCard_freeData);
  return 0;
}



int LC_KVKCard_UnextendCard(LC_CARD *card){
  LC_KVKCARD *kvk;
  int rv;

  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);
  LC_Card_SetOpenFn(card, kvk->openFn);
  LC_Card_SetCloseFn(card, kvk->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_KVKCARD, card);

  rv=LC_MemoryCard_UnextendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here");
  }
  return rv;
}



void GWENHYWFAR_CB LC_KVKCard_freeData(void *bp, void *p){
  LC_KVKCARD *kvk;

  assert(bp);
  assert(p);
  kvk=(LC_KVKCARD*)p;
  GWEN_DB_Group_free(kvk->dbData);
  GWEN_FREE_OBJECT(kvk);
}



LC_CLIENT_RESULT LC_KVKCard_ReadCardData(LC_CARD *card){
  LC_CLIENT_RESULT res;
  const char *p;
  unsigned int size;
  unsigned int pos;
  unsigned int j;
  LC_TLV *tlv;
  GWEN_DB_NODE *dbData;
  GWEN_BUFFER *mbuf;
  LC_KVKCARD *kvk;
  int checksumOk;

  assert(card);
  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);

  GWEN_DB_Group_free(kvk->dbData);
  kvk->dbData=0;

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  DBG_DEBUG(LC_LOGDOMAIN, "Reading card data header");
  res=LC_Card_IsoReadBinary(card, 0, 0x1e, 5, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  GWEN_Buffer_Rewind(mbuf);
  p=GWEN_Buffer_GetStart(mbuf);
  pos=0;
  size=GWEN_Buffer_GetBytesLeft(mbuf);

  /* get tag type */
  DBG_DEBUG(LC_LOGDOMAIN, "Determining card data length");
  if (size<2) {
    DBG_ERROR(LC_LOGDOMAIN, "Too few bytes for BER-TLV");
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }
  j=(unsigned char)(p[pos]);
  if ((j & 0x1f)==0x1f) {
    pos++;
    if (pos>=size) {
      DBG_ERROR(LC_LOGDOMAIN, "Too few bytes");
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }
    j=(unsigned char)(p[pos]);
  }
  else
    j&=0x1f;

  /* get length */
  pos++;
  if (pos>=size) {
    DBG_ERROR(LC_LOGDOMAIN, "Too few bytes");
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }
  j=(unsigned char)(p[pos]);
  if (j & 0x80) {
    if (j==0x81) {
      pos++;
      if (pos>=size) {
	DBG_ERROR(LC_LOGDOMAIN, "Too few bytes");
	GWEN_Buffer_free(mbuf);
	return LC_Client_ResultDataError;
      }
      j=(unsigned char)(p[pos]);
    } /* 0x81 */
    else if (j==0x82) {
      if (pos+1>=size) {
	DBG_ERROR(LC_LOGDOMAIN, "Too few bytes");
	GWEN_Buffer_free(mbuf);
	return LC_Client_ResultDataError;
      }
      pos++;
      j=((unsigned char)(p[pos]))<<8;
      pos++;
      j+=(unsigned char)(p[pos]);
    } /* 0x82 */
    else {
      DBG_ERROR(LC_LOGDOMAIN, "Unexpected tag length modifier %02x", j);
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }
  } /* if tag length modifier */
  pos++;

  /* j now contains the tag data size, add tag header length */
  j+=pos;
  /* sub size of already read data */
  j-=size;
  GWEN_Buffer_IncrementPos(mbuf, size);

  /* now read the rest */
  DBG_DEBUG(LC_LOGDOMAIN, "Reading rest of card data");
  res=LC_Card_IsoReadBinary(card, 0, size+0x1e, j, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  DBG_DEBUG(LC_LOGDOMAIN, "Parsing data...");
  GWEN_Buffer_Rewind(mbuf);
  dbData=GWEN_DB_Group_new("kvkData");
  if (LC_Card_ParseData(card, "kvkdata", mbuf, dbData)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error in KVK data");
    GWEN_DB_Group_free(dbData);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  /* perform checksum test */
  checksumOk=0;
  GWEN_Buffer_Rewind(mbuf);
  tlv=LC_TLV_fromBuffer(mbuf, 1);
  if (tlv) {
    if (LC_TLV_GetTagLength(tlv)) {
      GWEN_Buffer_SetPos(mbuf,
                         LC_TLV_GetTagSize(tlv)-LC_TLV_GetTagLength(tlv));

      while(GWEN_Buffer_GetBytesLeft(mbuf)) {
	LC_TLV *tlvLoop;

	tlvLoop=LC_TLV_fromBuffer(mbuf, 1);
	if (!tlvLoop) {
	  DBG_ERROR(LC_LOGDOMAIN, "Bad TLV in KVK data (pos=%d)",
		    GWEN_Buffer_GetPos(mbuf));
	  GWEN_DB_Group_free(dbData);
	  GWEN_Buffer_free(mbuf);
	  return LC_Client_ResultDataError;
        }
	if (LC_TLV_GetTagType(tlvLoop)==0x0e) {
	  unsigned int i;
          unsigned char checkSum;

	  /* checksum tag */
	  p=GWEN_Buffer_GetStart(mbuf);
	  size=GWEN_Buffer_GetPos(mbuf);
	  checkSum=0;
          for (i=0; i<size; i++)
            checkSum^=(unsigned char)(*p++);

          if (checkSum) {
	    DBG_ERROR(LC_LOGDOMAIN, "Bad checksum in kvk card (%02x)", checkSum);
	    LC_TLV_free(tlvLoop);
	    GWEN_DB_Group_free(dbData);
	    GWEN_Buffer_free(mbuf);
	    return LC_Client_ResultDataError;
	  }
	  DBG_INFO(LC_LOGDOMAIN, "Checksum ok");
          checksumOk=1;
          break;
	}
	LC_TLV_free(tlvLoop);
      } /* while */
    }
    else {
      DBG_ERROR(LC_LOGDOMAIN, "Empty card");
      GWEN_DB_Group_free(dbData);
      GWEN_Buffer_free(mbuf);
      return LC_Client_ResultDataError;
    }
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN, "Internal: Bad TLVs in KVK data");
    GWEN_DB_Group_free(dbData);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  if (!checksumOk) {
    DBG_ERROR(LC_LOGDOMAIN, "Bad/missing checksum");
    GWEN_DB_Group_free(dbData);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  /* store card data */
  kvk->dbData=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			       "kvk/data");
  if (kvk->dbData)
    GWEN_DB_UnlinkGroup(kvk->dbData);
  GWEN_DB_Group_free(dbData);
  GWEN_Buffer_free(mbuf);

  return LC_Client_ResultOk;
}




LC_CLIENT_RESULT CHIPCARD_CB LC_KVKCard_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_KVKCARD *kvk;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening card as KVK card");

  assert(card);
  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);

  res=kvk->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_KVKCard_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    kvk->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_KVKCard_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_KVKCARD *kvk;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening KVK card");

  assert(card);
  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);

  DBG_DEBUG(LC_LOGDOMAIN, "Selecting KVK card and app");
  res=LC_Card_SelectCard(card, "kvk");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }
  res=LC_Card_SelectApp(card, "kvk");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }

  DBG_DEBUG(LC_LOGDOMAIN, "Selecting MF...");
  res=LC_Card_SelectMf(card);
  if (res!=LC_Client_ResultOk) {
    if (res==LC_Client_ResultDontExecute) {
      DBG_INFO(LC_LOGDOMAIN, "Not executing SelectMF");
    }
    else if (res==LC_Client_ResultCmdError) {
      DBG_WARN(LC_LOGDOMAIN, "Could not select MF, ignoring");
    }
    else {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
      return res;
    }
  }

  res=LC_KVKCard_ReadCardData(card);
  if (res!=LC_Client_ResultOk){
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    return res;
  }


  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_KVKCard_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_KVKCARD *kvk;

  assert(card);
  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);

  res=kvk->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}



GWEN_DB_NODE *LC_KVKCard_GetCardData(const LC_CARD *card){
  LC_KVKCARD *kvk;

  assert(card);
  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);

  return kvk->dbData;
}



const char *LC_KvkCard_GetCardNumber(const LC_CARD *card) {
  LC_KVKCARD *kvk;

  assert(card);
  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);

  return GWEN_DB_GetCharValue(kvk->dbData, "cardNumber", 0, NULL);
}



LC_CLIENT_RESULT LC_KvkCard_ReadCardData(LC_CARD *card,
					 LC_HI_PERSONAL_DATA **pPersonal,
					 LC_HI_INSURANCE_DATA **pInsurance) {
  LC_KVKCARD *kvk;
  LC_HI_PERSONAL_DATA *pData;
  LC_HI_INSURANCE_DATA *iData;
  const char *s;

  assert(card);
  kvk=GWEN_INHERIT_GETDATA(LC_CARD, LC_KVKCARD, card);
  assert(kvk);

  pData=LC_HIPersonalData_new();
  iData=LC_HIInsuranceData_new();

  if (GWEN_Logger_GetLevel(LC_LOGDOMAIN)>GWEN_LoggerLevel_Info)
    GWEN_DB_Dump(kvk->dbData, 2);

  s=GWEN_DB_GetCharValue(kvk->dbData, "insuranceCompanyName", 0, NULL);
  LC_HIInsuranceData_SetInstitutionName(iData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "insuranceCompanyCode", 0, NULL);
  LC_HIInsuranceData_SetInstitutionId(iData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "insuranceNumber", 0, NULL);
  LC_HIPersonalData_SetInsuranceId(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "insuranceState", 0, NULL);
  LC_HIInsuranceData_SetStatus(iData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "eastOrWest", 0, NULL);
  LC_HIInsuranceData_SetGroup(iData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "title", 0, NULL);
  LC_HIPersonalData_SetTitle(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "foreName", 0, NULL);
  LC_HIPersonalData_SetPrename(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "name", 0, NULL);
  LC_HIPersonalData_SetName(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "nameSuffix", 0, NULL);
  LC_HIPersonalData_SetNameSuffix(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "dateOfBirth", 0, NULL);
  if (s) {
    GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "DDMMYYYY");
    LC_HIPersonalData_SetDateOfBirth(pData, ti);
    GWEN_Time_free(ti);
  }

  s=GWEN_DB_GetCharValue(kvk->dbData, "addrState", 0, NULL);
  LC_HIPersonalData_SetAddrState(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "addrCity", 0, NULL);
  LC_HIPersonalData_SetAddrCity(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "addrStreet", 0, NULL);
  LC_HIPersonalData_SetAddrStreet(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "addrPostalCode", 0, NULL);
  LC_HIPersonalData_SetAddrZipCode(pData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "insuranceState", 0, NULL);
  LC_HIInsuranceData_SetStatus(iData, s);

  s=GWEN_DB_GetCharValue(kvk->dbData, "bestBefore", 0, NULL);
  if (s) {
    GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "MMYY");
    LC_HIInsuranceData_SetCoverEnd(iData, ti);
    GWEN_Time_free(ti);
  }

  LC_HIPersonalData_SetAddrCountry(pData, "de");

  *pPersonal=pData;
  *pInsurance=iData;

  return LC_Client_ResultOk;
}





