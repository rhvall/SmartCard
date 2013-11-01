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

#define CHIPCARD_NOWARN_DEPRECATED

#include "egkcard_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>
#include <chipcard/chipcard.h>
#include <chipcard/cards/processorcard.h>

#include <zlib.h>


GWEN_INHERIT(LC_CARD, LC_EGKCARD)



int LC_EgkCard_ExtendCard(LC_CARD *card){
  LC_EGKCARD *egk;
  int rv;

  rv=LC_ProcessorCard_ExtendCard(card);
  if (rv) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not extend card as processor card");
    return rv;
  }

  GWEN_NEW_OBJECT(LC_EGKCARD, egk);

  egk->openFn=LC_Card_GetOpenFn(card);
  egk->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, LC_EgkCard_Open);
  LC_Card_SetCloseFn(card, LC_EgkCard_Close);

  GWEN_INHERIT_SETDATA(LC_CARD, LC_EGKCARD, card, egk,
                       LC_EgkCard_freeData);
  return 0;
}



int LC_EgkCard_UnextendCard(LC_CARD *card){
  LC_EGKCARD *egk;
  int rv;

  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);
  LC_Card_SetOpenFn(card, egk->openFn);
  LC_Card_SetCloseFn(card, egk->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_EGKCARD, card);

  rv=LC_ProcessorCard_UnextendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here");
  }
  return rv;
}



void GWENHYWFAR_CB LC_EgkCard_freeData(void *bp, void *p){
  LC_EGKCARD *egk;

  assert(bp);
  assert(p);
  egk=(LC_EGKCARD*)p;
  GWEN_FREE_OBJECT(egk);
}



LC_CLIENT_RESULT CHIPCARD_CB LC_EgkCard_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_EGKCARD *egk;

  DBG_INFO(LC_LOGDOMAIN, "Opening card as EGK card");

  assert(card);
  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);

  res=egk->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_EgkCard_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    egk->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_EgkCard_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_EGKCARD *egk;

  DBG_INFO(LC_LOGDOMAIN, "Opening eGK card");

  assert(card);
  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);

  res=LC_Card_SelectCard(card, "egk");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_Card_SelectApp(card, "egk");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Selecting MF...");
  res=LC_Card_SelectMf(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Selecting DF...");
  res=LC_Card_SelectDf(card, "DF_HCA");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_EgkCard_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_EGKCARD *egk;

  assert(card);
  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);

  res=egk->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}



LC_CLIENT_RESULT LC_EgkCard_VerifyPin(LC_CARD *card, const char *pin){
  LC_EGKCARD *egk;
  LC_CLIENT_RESULT res;
  LC_PININFO *pi;
  int triesLeft=-1;

  assert(card);
  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);

  pi=LC_Card_GetPinInfoByName(card, "ch_pin");
  assert(pi);
  res=LC_Card_IsoVerifyPin(card, 0, pi,
			   (const unsigned char*)pin, strlen(pin),
			   &triesLeft);
  LC_PinInfo_free(pi);
  return res;
}



LC_CLIENT_RESULT LC_EgkCard_SecureVerifyPin(LC_CARD *card){
  LC_EGKCARD *egk;
  LC_CLIENT_RESULT res;
  LC_PININFO *pi;
  int triesLeft=-1;

  assert(card);
  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);

  pi=LC_Card_GetPinInfoByName(card, "ch_pin");
  assert(pi);
  res=LC_Card_IsoPerformVerification(card, 0, pi, &triesLeft);
  LC_PinInfo_free(pi);
  return res;
}



LC_CLIENT_RESULT LC_EgkCard_Unzip(const char *src, unsigned int slen,
				  GWEN_BUFFER *tbuf) {
  unsigned char buffer[256];
  z_stream strm;
  int rv;
  int first=1;

  memset(&strm, 0, sizeof(strm));

  strm.next_in=(Bytef*) src;
  strm.avail_in=slen;

  strm.next_out=buffer;
  strm.avail_out=sizeof(buffer);
  strm.opaque=Z_NULL;
  strm.zalloc=Z_NULL;
  strm.zfree=Z_NULL;
  strm.msg=NULL;

  rv=inflateInit2(&strm, 15+16);
  if (rv!=Z_OK) {
    switch(rv) {
    case Z_VERSION_ERROR:
      DBG_ERROR(LC_LOGDOMAIN, "Non-matching version of ZLIB");
      return LC_Client_ResultGeneric;
    case Z_STREAM_ERROR:
      DBG_ERROR(LC_LOGDOMAIN, "inflateInit: stream error (%d, %s)",
		rv, (strm.msg)?strm.msg:"NULL");
      return LC_Client_ResultDataError;
    default:
      DBG_ERROR(LC_LOGDOMAIN, "inflateInit: %d (%s)",
		rv, (strm.msg)?strm.msg:"NULL");
      return LC_Client_ResultGeneric;
    }
  }

  for (;;) {
    unsigned int inflated;

    strm.next_out=buffer;
    strm.avail_out=sizeof(buffer);
    rv=inflate(&strm, Z_NO_FLUSH);
    inflated=sizeof(buffer)-strm.avail_out;
    if (inflated)
      GWEN_Buffer_AppendBytes(tbuf, (const char*)buffer, inflated);
    if (rv==Z_STREAM_END || rv==Z_BUF_ERROR)
      break;
    if (rv!=Z_OK) {
      DBG_ERROR(LC_LOGDOMAIN, "inflate: %d (%s)",
		rv, (strm.msg)?strm.msg:"NULL");
      inflateEnd(&strm);
      return LC_Client_ResultIoError;
    }
    if (first)
      first=0;
  }

  rv=inflateEnd(&strm);
  if (rv!=Z_OK) {
    DBG_ERROR(LC_LOGDOMAIN, "inflateEnd: %d (%s)",
	      rv, (strm.msg)?strm.msg:"NULL");
    return LC_Client_ResultIoError;
  }

  return 0;
}



LC_CLIENT_RESULT LC_EgkCard_ReadRawPd(LC_CARD *card, GWEN_BUFFER *buf){
  LC_EGKCARD *egk;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *lbuf;
  int size;
  const unsigned char *p;

  assert(card);
  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);

  res=LC_Card_SelectEf(card, "EF_PD");
  if (res!=LC_Client_ResultOk)
    return res;

  lbuf=GWEN_Buffer_new(0, 2, 0, 1);
  res=LC_Card_IsoReadBinary(card, 0, 0, 2, lbuf);
  if (res!=LC_Client_ResultOk) {
    GWEN_Buffer_free(lbuf);
    return res;
  }

  if (GWEN_Buffer_GetUsedBytes(lbuf)<2) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid response size (%d)",
	      GWEN_Buffer_GetUsedBytes(lbuf));
    GWEN_Buffer_free(lbuf);
    return LC_Client_ResultDataError;
  }

  p=(const unsigned char*)GWEN_Buffer_GetStart(lbuf);
  assert(p);
  size=(*(p++))<<8;
  size+=*p;
  if (size<2) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid size spec in data (%d)", size);
    GWEN_Buffer_free(lbuf);
    return LC_Client_ResultDataError;
  }
  size-=2;

  GWEN_Buffer_Reset(lbuf);

  if (size) {
    res=LC_Card_ReadBinary(card, 2, size, lbuf);
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
      GWEN_Buffer_free(lbuf);
      return res;
    }
  }

  res=LC_EgkCard_Unzip(GWEN_Buffer_GetStart(lbuf),
		       GWEN_Buffer_GetUsedBytes(lbuf),
                       buf);
  GWEN_Buffer_free(lbuf);

  return res;
}



LC_CLIENT_RESULT LC_EgkCard_ReadPd(LC_CARD *card, GWEN_BUFFER *buf){
  return LC_EgkCard_ReadRawPd(card, buf);
}



LC_CLIENT_RESULT LC_EgkCard_ReadRawVd(LC_CARD *card, GWEN_BUFFER *buf){
  LC_EGKCARD *egk;
  LC_CLIENT_RESULT res;
  GWEN_BUFFER *lbuf;
  int offs1, offs2;
  int end1, end2;
  int size1, size2;
  const unsigned char *p;

  assert(card);
  egk=GWEN_INHERIT_GETDATA(LC_CARD, LC_EGKCARD, card);
  assert(egk);

  res=LC_Card_SelectEf(card, "EF_VD");
  if (res!=LC_Client_ResultOk)
    return res;

  lbuf=GWEN_Buffer_new(0, 8, 0, 1);
  res=LC_Card_IsoReadBinary(card, 0, 0, 8, lbuf);
  if (res!=LC_Client_ResultOk) {
    GWEN_Buffer_free(lbuf);
    return res;
  }

  if (GWEN_Buffer_GetUsedBytes(lbuf)<8) {
    DBG_ERROR(LC_LOGDOMAIN, "Invalid response size (%d)",
	      GWEN_Buffer_GetUsedBytes(lbuf));
    GWEN_Buffer_free(lbuf);
    return LC_Client_ResultDataError;
  }

  p=(const unsigned char*)GWEN_Buffer_GetStart(lbuf);
  assert(p);
  offs1=(*(p++))<<8;
  offs1+=(*(p++));
  end1=(*(p++))<<8;
  end1+=(*(p++));
  size1=end1-offs1+1;

  offs2=(*(p++))<<8;
  offs2+=(*(p++));
  end2=(*(p++))<<8;
  end2+=(*(p++));
  size2=end2-offs2+1;

  GWEN_Buffer_Reset(lbuf);

  if (offs1!=0xffff && end1!=0xffff && size1>0) {
    res=LC_Card_ReadBinary(card, offs1, size1, lbuf);
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
      GWEN_Buffer_free(lbuf);
      return res;
    }
  }

  res=LC_EgkCard_Unzip(GWEN_Buffer_GetStart(lbuf),
		       GWEN_Buffer_GetUsedBytes(lbuf),
		       buf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    GWEN_Buffer_free(lbuf);
    return res;
  }

  GWEN_Buffer_Reset(lbuf);

  if (offs2!=0xffff && end2!=0xffff && size2>0) {
    res=LC_Card_ReadBinary(card, offs2, size2, lbuf);
    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
      GWEN_Buffer_free(lbuf);
      return res;
    }
  }

  res=LC_EgkCard_Unzip(GWEN_Buffer_GetStart(lbuf),
		       GWEN_Buffer_GetUsedBytes(lbuf),
		       buf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    GWEN_Buffer_free(lbuf);
    return res;
  }

  GWEN_Buffer_free(lbuf);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_EgkCard_ReadVd(LC_CARD *card, GWEN_BUFFER *buf){
  return LC_EgkCard_ReadRawVd(card, buf);
}


LC_CLIENT_RESULT LC_EgkCard_ReadPersonalData_old(GWEN_XMLNODE *n,
						 LC_HI_PERSONAL_DATA *d) {
  const char *s;
  GWEN_XMLNODE *nn;
  
  s=GWEN_XMLNode_GetCharValue(n, "Versicherten_ID", NULL);
  LC_HIPersonalData_SetInsuranceId(d, s);
  
  s=GWEN_XMLNode_GetCharValue(n, "Geburtsdatum", NULL);
  if (s) {
    GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
    LC_HIPersonalData_SetDateOfBirth(d, ti);
    GWEN_Time_free(ti);
  }
  s=GWEN_XMLNode_GetCharValue(n, "Vorname", NULL);
  LC_HIPersonalData_SetPrename(d, s);
  s=GWEN_XMLNode_GetCharValue(n, "Nachname", NULL);
  LC_HIPersonalData_SetName(d, s);
  s=GWEN_XMLNode_GetCharValue(n, "Sex", "1");
  if (s) {
    if (strcasecmp(s, "1")==0)
      LC_HIPersonalData_SetSex(d, LC_HIPersonalData_SexMale);
    else if (strcasecmp(s, "2")==0)
      LC_HIPersonalData_SetSex(d, LC_HIPersonalData_SexFemale);
    else {
      DBG_WARN(LC_LOGDOMAIN, "Unknown sex \"%s\"", s);
    }
  }
  
  nn=GWEN_XMLNode_FindFirstTag(n,
			       "Anschrift",
			       NULL, NULL);
  if (nn) {
    s=GWEN_XMLNode_GetCharValue(nn, "Postleitzahl", NULL);
    LC_HIPersonalData_SetAddrZipCode(d, s);
    s=GWEN_XMLNode_GetCharValue(nn, "Ort", NULL);
    LC_HIPersonalData_SetAddrCity(d, s);
    s=GWEN_XMLNode_GetCharValue(nn, "Wohnsitzlaendercode", NULL);
    LC_HIPersonalData_SetAddrCountry(d, s);
    nn=GWEN_XMLNode_FindFirstTag(nn,
				 "Adresse",
				 NULL, NULL);
    if (nn) {
      s=GWEN_XMLNode_GetCharValue(nn, "Strasse", NULL);
      LC_HIPersonalData_SetAddrStreet(d, s);
      s=GWEN_XMLNode_GetCharValue(nn, "Hausnummer", NULL);
      LC_HIPersonalData_SetAddrHouseNum(d, s);
    }
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_EgkCard_ReadPersonalData_3_0_0(GWEN_XMLNODE *n,
						   LC_HI_PERSONAL_DATA *d) {
  n=GWEN_XMLNode_FindFirstTag(n,
			      "Versicherter",
			      NULL, NULL);
  if (n) {
    const char *s;
    GWEN_XMLNODE *nn;

    s=GWEN_XMLNode_GetCharValue(n, "Versicherten_ID", NULL);
    LC_HIPersonalData_SetInsuranceId(d, s);

    nn=GWEN_XMLNode_FindFirstTag(n,
				 "Person",
				 NULL, NULL);
    if (nn) {
      GWEN_XMLNODE *nnn;

      s=GWEN_XMLNode_GetCharValue(nn, "Geburtsdatum", NULL);
      if (s) {
	GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
	LC_HIPersonalData_SetDateOfBirth(d, ti);
	GWEN_Time_free(ti);
      }
      s=GWEN_XMLNode_GetCharValue(nn, "Vorname", NULL);
      LC_HIPersonalData_SetPrename(d, s);
      s=GWEN_XMLNode_GetCharValue(nn, "Nachname", NULL);
      LC_HIPersonalData_SetName(d, s);
      s=GWEN_XMLNode_GetCharValue(nn, "Sex", "1");
      if (s) {
	if (strcasecmp(s, "1")==0)
	  LC_HIPersonalData_SetSex(d, LC_HIPersonalData_SexMale);
	else if (strcasecmp(s, "2")==0)
	  LC_HIPersonalData_SetSex(d, LC_HIPersonalData_SexFemale);
	else {
	  DBG_WARN(LC_LOGDOMAIN, "Unknown sex \"%s\"", s);
	}
      }

      s=GWEN_XMLNode_GetCharValue(nn, "Titel", NULL);
      LC_HIPersonalData_SetTitle(d, s);

      s=GWEN_XMLNode_GetCharValue(nn, "Namenszusatz", NULL);
      LC_HIPersonalData_SetNameSuffix(d, s);

      nnn=GWEN_XMLNode_FindFirstTag(nn,
				    "StrassenAdresse",
				    NULL, NULL);
      if (nnn) {
	GWEN_XMLNODE *nnnn;

	s=GWEN_XMLNode_GetCharValue(nnn, "Postleitzahl", NULL);
	LC_HIPersonalData_SetAddrZipCode(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Ort", NULL);
	LC_HIPersonalData_SetAddrCity(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Strasse", NULL);
	LC_HIPersonalData_SetAddrStreet(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Hausnummer", NULL);
	LC_HIPersonalData_SetAddrHouseNum(d, s);

	nnnn=GWEN_XMLNode_FindFirstTag(nnn,
				       "Land",
				       NULL, NULL);
	if (nnnn) {
	  s=GWEN_XMLNode_GetCharValue(nnnn, "Wohnsitzlaendercode", NULL);
	  LC_HIPersonalData_SetAddrCountry(d, s);
	}
      }
    }
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_EgkCard_ReadPersonalData_5_1_0(GWEN_XMLNODE *n,
						   LC_HI_PERSONAL_DATA *d) {
  n=GWEN_XMLNode_FindFirstTag(n,
			      "Versicherter",
			      NULL, NULL);
  if (n) {
    const char *s;
    GWEN_XMLNODE *nn;

    s=GWEN_XMLNode_GetCharValue(n, "Versicherten_ID", NULL);
    LC_HIPersonalData_SetInsuranceId(d, s);

    nn=GWEN_XMLNode_FindFirstTag(n,
				 "Person",
				 NULL, NULL);
    if (nn) {
      GWEN_XMLNODE *nnn;

      s=GWEN_XMLNode_GetCharValue(nn, "Geburtsdatum", NULL);
      if (s) {
	GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
	LC_HIPersonalData_SetDateOfBirth(d, ti);
	GWEN_Time_free(ti);
      }
      s=GWEN_XMLNode_GetCharValue(nn, "Vorname", NULL);
      LC_HIPersonalData_SetPrename(d, s);
      s=GWEN_XMLNode_GetCharValue(nn, "Nachname", NULL);
      LC_HIPersonalData_SetName(d, s);
      s=GWEN_XMLNode_GetCharValue(nn, "Sex", "M");
      if (s) {
	if (strcasecmp(s, "M")==0)
	  LC_HIPersonalData_SetSex(d, LC_HIPersonalData_SexMale);
	else if (strcasecmp(s, "W")==0)
	  LC_HIPersonalData_SetSex(d, LC_HIPersonalData_SexFemale);
	else {
	  DBG_WARN(LC_LOGDOMAIN, "Unknown sex \"%s\"", s);
	}
      }
      s=GWEN_XMLNode_GetCharValue(nn, "Titel", NULL);
      LC_HIPersonalData_SetTitle(d, s);

      s=GWEN_XMLNode_GetCharValue(nn, "Namenszusatz", NULL);
      LC_HIPersonalData_SetNameSuffix(d, s);

      nnn=GWEN_XMLNode_FindFirstTag(nn,
				    "StrassenAdresse",
				    NULL, NULL);
      if (nnn) {
	GWEN_XMLNODE *nnnn;

	s=GWEN_XMLNode_GetCharValue(nnn, "Postleitzahl", NULL);
	LC_HIPersonalData_SetAddrZipCode(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Ort", NULL);
	LC_HIPersonalData_SetAddrCity(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Strasse", NULL);
	LC_HIPersonalData_SetAddrStreet(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Hausnummer", NULL);
	LC_HIPersonalData_SetAddrHouseNum(d, s);

	nnnn=GWEN_XMLNode_FindFirstTag(nnn,
				       "Land",
				       NULL, NULL);
	if (nnnn) {
	  s=GWEN_XMLNode_GetCharValue(nnnn, "Wohnsitzlaendercode", NULL);
	  LC_HIPersonalData_SetAddrCountry(d, s);
	}
      }
    }
  }

  return LC_Client_ResultOk;
}


LC_CLIENT_RESULT LC_EgkCard_ReadPersonalData(LC_CARD *card,
					     LC_HI_PERSONAL_DATA **pData) {
  GWEN_BUFFER *dbuf;
  LC_CLIENT_RESULT res;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_EgkCard_ReadPd(card, dbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    GWEN_Buffer_free(dbuf);
    return res;
  }
  else {
    GWEN_XMLNODE *root;
    GWEN_XMLNODE *n;

    root=GWEN_XMLNode_fromString(GWEN_Buffer_GetStart(dbuf),
				 GWEN_Buffer_GetUsedBytes(dbuf),
				 GWEN_XML_FLAGS_HANDLE_HEADERS |
				 GWEN_XML_FLAGS_HANDLE_NAMESPACES);
    if (root==NULL) {
      DBG_INFO(LC_LOGDOMAIN, "Invalid XML string");
      GWEN_Buffer_free(dbuf);
      return LC_Client_ResultDataError;
    }
    GWEN_Buffer_free(dbuf);

    GWEN_XMLNode_StripNamespaces(root);

    /*GWEN_XMLNode_Dump(root, stderr, 2);*/

    n=GWEN_XMLNode_FindFirstTag(root,
				"UC_PersoenlicheVersichertendatenXML",
				NULL, NULL);
    if (n) {
      const char *s;
      LC_HI_PERSONAL_DATA *d;

      d=LC_HIPersonalData_new();

      s=GWEN_XMLNode_GetProperty(n, "CDM_VERSION", NULL);
      if (s) {
	if (GWEN_Text_ComparePattern(s, "5.*", 0)!=-1)
	  res=LC_EgkCard_ReadPersonalData_5_1_0(n, d);
	else if (GWEN_Text_ComparePattern(s, "3.*", 0)!=-1)
	  res=LC_EgkCard_ReadPersonalData_3_0_0(n, d);
	else {
	  DBG_WARN(LC_LOGDOMAIN,
		   "Unhandled CDM_VERSION [%s], trying 5.1.0", s);
	  res=LC_EgkCard_ReadPersonalData_5_1_0(n, d);
	}
      }
      else
	res=LC_EgkCard_ReadPersonalData_old(n, d);

      if (res!=LC_Client_ResultOk) {
	DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
	LC_HIPersonalData_free(d);
	GWEN_XMLNode_free(root);
	return res;
      }

      *pData=d;
    }

    GWEN_XMLNode_free(root);
  }

  return LC_Client_ResultOk;
}




LC_CLIENT_RESULT LC_EgkCard_ReadInsuranceData_old(GWEN_XMLNODE *n,
						  LC_HI_INSURANCE_DATA *d) {
  const char *s;
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_FindFirstTag(n,
			       "Versicherungsschutz",
			       NULL, NULL);
  if (nn) {
    s=GWEN_XMLNode_GetCharValue(nn, "Begin", NULL);
    if (s) {
      GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
      LC_HIInsuranceData_SetCoverBegin(d, ti);
      GWEN_Time_free(ti);
    }
    s=GWEN_XMLNode_GetCharValue(nn, "Ende", NULL);
    if (s) {
      GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
      LC_HIInsuranceData_SetCoverEnd(d, ti);
      GWEN_Time_free(ti);
    }
    s=GWEN_XMLNode_GetCharValue(nn, "Kostentraegerkennung", NULL);
    DBG_ERROR(0, "KT-Kennung: %s", s);
    LC_HIInsuranceData_SetInstitutionId(d, s);
    s=GWEN_XMLNode_GetCharValue(nn, "Name", NULL);
    LC_HIInsuranceData_SetInstitutionName(d, s);
  }
  else {
    DBG_INFO(LC_LOGDOMAIN,
	     "XML element \"Versicherungsschutz\" not found");
  }
  nn=GWEN_XMLNode_FindFirstTag(n,
			       "Zusatzinfos",
			       NULL, NULL);
  if (nn)
    nn=GWEN_XMLNode_FindFirstTag(nn,
				 "Zusatzinfos_GKV",
				 NULL, NULL);
  if (nn) {
    s=GWEN_XMLNode_GetCharValue(nn, "Rechtskreis", NULL);
    LC_HIInsuranceData_SetGroup(d, s);
    s=GWEN_XMLNode_GetCharValue(nn, "Versichertenart", NULL);
    LC_HIInsuranceData_SetStatus(d, s);
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_EgkCard_ReadInsuranceData_3_0_0(GWEN_XMLNODE *n,
						    LC_HI_INSURANCE_DATA *d) {
  const char *s;
  GWEN_XMLNODE *nn;

  n=GWEN_XMLNode_FindFirstTag(n,
			      "Versicherter",
			      NULL, NULL);
  if (n) {
    nn=GWEN_XMLNode_FindFirstTag(n,
				 "Versicherungsschutz",
				 NULL, NULL);
    if (nn) {
      GWEN_XMLNODE *nnn;

      s=GWEN_XMLNode_GetCharValue(nn, "Beginn", NULL);
      if (s) {
	GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
	LC_HIInsuranceData_SetCoverBegin(d, ti);
	GWEN_Time_free(ti);
      }
      s=GWEN_XMLNode_GetCharValue(nn, "Ende", NULL);
      if (s) {
	GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
	LC_HIInsuranceData_SetCoverEnd(d, ti);
	GWEN_Time_free(ti);
      }

      nnn=GWEN_XMLNode_FindFirstTag(nn,
				    "Kostentraeger",
				    NULL, NULL);
      if (nnn) {
	s=GWEN_XMLNode_GetCharValue(nnn, "Kostentraegerkennung", NULL);
	LC_HIInsuranceData_SetInstitutionId(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Name", NULL);
	LC_HIInsuranceData_SetInstitutionName(d, s);
      }
    }
    else {
      DBG_INFO(LC_LOGDOMAIN,
	       "XML element \"Versicherungsschutz\" not found");
    }
    nn=GWEN_XMLNode_FindFirstTag(n,
				 "Zusatzinfos",
				 NULL, NULL);
    if (nn)
      nn=GWEN_XMLNode_FindFirstTag(nn,
				   "ZusatzinfosGKV",
				   NULL, NULL);
    if (nn) {
      s=GWEN_XMLNode_GetCharValue(nn, "Rechtskreis", NULL);
      LC_HIInsuranceData_SetGroup(d, s);
      s=GWEN_XMLNode_GetCharValue(nn, "Versichertenart", NULL);
      LC_HIInsuranceData_SetStatus(d, s);
    }
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_EgkCard_ReadInsuranceData_5_1_0(GWEN_XMLNODE *n,
						    LC_HI_INSURANCE_DATA *d) {
  const char *s;
  GWEN_XMLNODE *nn;

  n=GWEN_XMLNode_FindFirstTag(n,
			      "Versicherter",
			      NULL, NULL);
  if (n) {
    nn=GWEN_XMLNode_FindFirstTag(n,
				 "Versicherungsschutz",
				 NULL, NULL);
    if (nn) {
      GWEN_XMLNODE *nnn;

      s=GWEN_XMLNode_GetCharValue(nn, "Beginn", NULL);
      if (s) {
	GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
	LC_HIInsuranceData_SetCoverBegin(d, ti);
	GWEN_Time_free(ti);
      }
      s=GWEN_XMLNode_GetCharValue(nn, "Ende", NULL);
      if (s) {
	GWEN_TIME *ti=GWEN_Time_fromUtcString(s, "YYYYMMDD");
	LC_HIInsuranceData_SetCoverEnd(d, ti);
	GWEN_Time_free(ti);
      }

      nnn=GWEN_XMLNode_FindFirstTag(nn,
				    "Kostentraeger",
				    NULL, NULL);
      if (nnn) {
	s=GWEN_XMLNode_GetCharValue(nnn, "Kostentraegerkennung", NULL);
	LC_HIInsuranceData_SetInstitutionId(d, s);
	s=GWEN_XMLNode_GetCharValue(nnn, "Name", NULL);
	LC_HIInsuranceData_SetInstitutionName(d, s);
      }
    }
    else {
      DBG_INFO(LC_LOGDOMAIN,
	       "XML element \"Versicherungsschutz\" not found");
    }
    nn=GWEN_XMLNode_FindFirstTag(n,
				 "Zusatzinfos",
				 NULL, NULL);
    if (nn)
      nn=GWEN_XMLNode_FindFirstTag(nn,
				   "ZusatzinfosGKV",
				   NULL, NULL);
    if (nn) {
      s=GWEN_XMLNode_GetCharValue(nn, "Rechtskreis", NULL);
      LC_HIInsuranceData_SetGroup(d, s);
      s=GWEN_XMLNode_GetCharValue(nn, "Versichertenart", NULL);
      LC_HIInsuranceData_SetStatus(d, s);
    }
  }

  return LC_Client_ResultOk;
}




LC_CLIENT_RESULT LC_EgkCard_ParseInsuranceData(GWEN_XMLNODE *root,
					       LC_HI_INSURANCE_DATA **pData) {
  LC_CLIENT_RESULT res;

  GWEN_XMLNODE *n;
  LC_HI_INSURANCE_DATA *d=NULL;

  n=GWEN_XMLNode_FindFirstTag(root,
			      "UC_allgemeineVersicherungsdatenXML",
			      NULL, NULL);
  if (n) {
    const char *s;

    d=LC_HIInsuranceData_new();
    s=GWEN_XMLNode_GetProperty(n, "CDM_VERSION", NULL);
    if (s) {
      DBG_INFO(LC_LOGDOMAIN, "CDM_VERSION is [%s]", s);
      if (GWEN_Text_ComparePattern(s, "5.*", 0)!=-1) {
	DBG_INFO(LC_LOGDOMAIN, "Reading as 5.1.0");
	res=LC_EgkCard_ReadInsuranceData_5_1_0(n, d);
      }
      else if (GWEN_Text_ComparePattern(s, "3.*", 0)!=-1) {
	DBG_INFO(LC_LOGDOMAIN, "Reading as 3.0.0");
	res=LC_EgkCard_ReadInsuranceData_3_0_0(n, d);
      }
      else {
	DBG_WARN(LC_LOGDOMAIN,
		 "Unhandled CDM_VERSION [%s], trying 5.1.0", s);
	res=LC_EgkCard_ReadInsuranceData_5_1_0(n, d);
      }
    }
    else {
      DBG_INFO(LC_LOGDOMAIN,
	       "Missing CDM_VERSION, trying old data type");
      /*GWEN_XMLNode_Dump(n, stderr, 2);*/
      res=LC_EgkCard_ReadInsuranceData_old(n, d);
    }

    if (res!=LC_Client_ResultOk) {
      DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
      LC_HIInsuranceData_free(d);
      return res;
    }
  }
  else {
    DBG_ERROR(LC_LOGDOMAIN, "UC_allgemeineVersicherungsdatenXML not found, data follows:");
    GWEN_XMLNode_Dump(root, 2);
    return LC_Client_ResultNotFound;
  }

  *pData=d;
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_EgkCard_ReadInsuranceData(LC_CARD *card,
					      LC_HI_INSURANCE_DATA **pData) {
  GWEN_BUFFER *dbuf;
  LC_CLIENT_RESULT res;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_EgkCard_ReadVd(card, dbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here (%d)", res);
    GWEN_Buffer_free(dbuf);
    return res;
  }
  else {
    GWEN_XMLNODE *root;

    root=GWEN_XMLNode_fromString(GWEN_Buffer_GetStart(dbuf),
				 GWEN_Buffer_GetUsedBytes(dbuf),
				 GWEN_XML_FLAGS_HANDLE_HEADERS |
				 GWEN_XML_FLAGS_HANDLE_NAMESPACES);
    if (root==NULL) {
      DBG_INFO(LC_LOGDOMAIN, "Invalid XML string");
      GWEN_Buffer_free(dbuf);
      return LC_Client_ResultDataError;
    }
    GWEN_Buffer_free(dbuf);

    GWEN_XMLNode_StripNamespaces(root);
    res=LC_EgkCard_ParseInsuranceData(root, pData);

    GWEN_XMLNode_free(root);
    return res;
  }
}



