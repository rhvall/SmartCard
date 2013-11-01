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


#include "geldkarte_p.h"
#include "geldkarte_blog_l.h"
#include "geldkarte_llog_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/text.h>
#include <chipcard/chipcard.h>
#include <chipcard/cards/processorcard.h>


GWEN_INHERIT(LC_CARD, LC_GELDKARTE)



int LC_GeldKarte_ExtendCard(LC_CARD *card){
  LC_GELDKARTE *gk;
  int rv;

  rv=LC_ProcessorCard_ExtendCard(card);
  if (rv) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not extend card as processor card");
    return rv;
  }

  GWEN_NEW_OBJECT(LC_GELDKARTE, gk);

  gk->openFn=LC_Card_GetOpenFn(card);
  gk->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, LC_GeldKarte_Open);
  LC_Card_SetCloseFn(card, LC_GeldKarte_Close);

  GWEN_INHERIT_SETDATA(LC_CARD, LC_GELDKARTE, card, gk,
                       LC_GeldKarte_freeData);
  return 0;
}



int LC_GeldKarte_UnextendCard(LC_CARD *card){
  LC_GELDKARTE *gk;
  int rv;

  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);
  LC_Card_SetOpenFn(card, gk->openFn);
  LC_Card_SetCloseFn(card, gk->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_GELDKARTE, card);

  rv=LC_ProcessorCard_UnextendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here");
  }
  return rv;
}



void GWENHYWFAR_CB LC_GeldKarte_freeData(void *bp, void *p){
  LC_GELDKARTE *gk;

  assert(bp);
  assert(p);
  gk=(LC_GELDKARTE*)p;
  GWEN_Buffer_free(gk->bin_ef_boerse_1);
  GWEN_DB_Group_free(gk->db_ef_boerse_1);
  GWEN_Buffer_free(gk->bin_ef_id_1);
  GWEN_DB_Group_free(gk->db_ef_id_1);
  GWEN_FREE_OBJECT(gk);
}



LC_CLIENT_RESULT CHIPCARD_CB LC_GeldKarte_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_GELDKARTE *gk;

  DBG_INFO(LC_LOGDOMAIN, "Opening card as Geldkarte");

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  GWEN_DB_Group_free(gk->db_ef_boerse_1);
  gk->db_ef_boerse_1=0;
  GWEN_Buffer_free(gk->bin_ef_boerse_1);
  gk->bin_ef_boerse_1=0;

  GWEN_DB_Group_free(gk->db_ef_id_1);
  gk->db_ef_id_1=0;
  GWEN_Buffer_free(gk->bin_ef_id_1);
  gk->bin_ef_id_1=0;

  if (strcasecmp(LC_Card_GetCardType(card), "PROCESSOR")!=0) {
    DBG_ERROR(LC_LOGDOMAIN, "Not a processor card");
    return LC_Client_ResultNotSupported;
  }

  res=gk->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_GeldKarte_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    gk->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_GeldKarte_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_GELDKARTE *gk;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *dbRecord;

  DBG_INFO(LC_LOGDOMAIN, "Opening Geldkarte");

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  GWEN_DB_Group_free(gk->db_ef_boerse_1);
  gk->db_ef_boerse_1=0;
  GWEN_Buffer_free(gk->bin_ef_boerse_1);
  gk->bin_ef_boerse_1=0;

  GWEN_DB_Group_free(gk->db_ef_id_1);
  gk->db_ef_id_1=0;
  GWEN_Buffer_free(gk->bin_ef_id_1);
  gk->bin_ef_id_1=0;

  res=LC_Card_SelectCard(card, "geldkarte");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_Card_SelectApp(card, "geldkarte");
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

  /* read first record of EF_ID */
  DBG_INFO(LC_LOGDOMAIN, "Selecting EF...");
  res=LC_Card_SelectEf(card, "EF_ID");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Reading record...");
  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Parsing record...");
  GWEN_Buffer_Rewind(mbuf);
  dbRecord=GWEN_DB_Group_new("record");
  if (LC_Card_ParseRecord(card, 1, mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error in EF_ID");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  gk->db_ef_id_1=dbRecord;
  gk->bin_ef_id_1=mbuf;

  /* select DF_BOERSE */
  DBG_INFO(LC_LOGDOMAIN, "Selecting DF...");
  res=LC_Card_SelectDf(card, "DF_BOERSE");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Selecting EF...");
  res=LC_Card_SelectEf(card, "EF_BOERSE");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Reading record...");
  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, mbuf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return res;
  }

  DBG_INFO(LC_LOGDOMAIN, "Parsing record...");
  GWEN_Buffer_Rewind(mbuf);
  dbRecord=GWEN_DB_Group_new("record");
  if (LC_Card_ParseRecord(card, 1, mbuf, dbRecord)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error in EF_ID");
    GWEN_DB_Group_free(dbRecord);
    GWEN_Buffer_free(mbuf);
    return LC_Client_ResultDataError;
  }

  gk->db_ef_boerse_1=dbRecord;
  gk->bin_ef_boerse_1=mbuf;
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_GeldKarte_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  res=gk->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}



GWEN_DB_NODE *LC_GeldKarte_GetCardDataAsDb(const LC_CARD *card){
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  return gk->db_ef_id_1;
}



GWEN_BUFFER *LC_GeldKarte_GetCardDataAsBuffer(const LC_CARD *card){
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  return gk->bin_ef_id_1;
}



GWEN_DB_NODE *LC_GeldKarte_GetAccountDataAsDb(const LC_CARD *card){
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  return gk->db_ef_boerse_1;
}



GWEN_BUFFER *LC_GeldKarte_GetAccountDataAsBuffer(const LC_CARD *card){
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  return gk->bin_ef_boerse_1;
}



LC_CLIENT_RESULT LC_GeldKarte__ReadValues(LC_CARD *card,
					  GWEN_DB_NODE *dbData){
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbCurr;
  GWEN_BUFFER *buf;
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  res=LC_Card_SelectEf(card, "EF_BETRAG");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN, 1, buf);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(0, "here");
    GWEN_Buffer_free(buf);
    return res;
  }

  dbCurr=GWEN_DB_Group_new("values");
  GWEN_Buffer_Rewind(buf);
  if (LC_Card_ParseRecord(card, 1, buf, dbCurr)) {
    DBG_ERROR(LC_LOGDOMAIN, "Error parsing record");
    GWEN_DB_Group_free(dbCurr);
    GWEN_Buffer_free(buf);
    return LC_Client_ResultDataError;
  }
  else {
    GWEN_DB_AddGroupChildren(dbData, dbCurr);
  }
  GWEN_DB_Group_free(dbCurr);
  GWEN_Buffer_free(buf);

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_GeldKarte_ReadValues(LC_CARD *card,
                                         LC_GELDKARTE_VALUES *val) {
  LC_GELDKARTE *gk;
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbData;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  dbData=GWEN_DB_Group_new("values");
  res=LC_GeldKarte__ReadValues(card, dbData);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbData);
    return res;
  }

  if (val) {
    int i;

    if (1!=sscanf(GWEN_DB_GetCharValue(dbData, "loaded", 0, ""),
                  "%d", &i)) {
      DBG_WARN(LC_LOGDOMAIN, "Bad value for \"loaded\"");
      i=0;
    }
    LC_GeldKarte_Values_SetLoaded(val, i);

    if (1!=sscanf(GWEN_DB_GetCharValue(dbData, "maxload", 0, ""),
                  "%d", &i)) {
      DBG_WARN(LC_LOGDOMAIN, "Bad value for \"maxload\"");
      i=0;
    }
    LC_GeldKarte_Values_SetMaxLoad(val, i);

    if (1!=sscanf(GWEN_DB_GetCharValue(dbData, "maxtrans", 0, ""),
                  "%d", &i)) {
      DBG_WARN(LC_LOGDOMAIN, "Bad value for \"maxtrans\"");
      i=0;
    }
    LC_GeldKarte_Values_SetMaxXfer(val, i);
  }

  GWEN_DB_Group_free(dbData);
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_GeldKarte__ReadBLog(LC_CARD *card,
					int idx,
					GWEN_DB_NODE *dbData){
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbCurr;
  int i;
  unsigned int ctxCount;
  GWEN_BUFFER *buf;
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  res=LC_Card_SelectEf(card, "EF_BLOG");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  ctxCount=0;
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  for (i=1; i<16; i++) {
    unsigned int len;
    const char *formatName;

    DBG_INFO(LC_LOGDOMAIN, "Reading BLOG record %d", i);
    GWEN_Buffer_Reset(buf);
    res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                              idx?idx:i, buf);
    if (res!=LC_Client_ResultOk)
      break;
    dbCurr=GWEN_DB_Group_new("blog");
    GWEN_Buffer_Rewind(buf);

    len=GWEN_Buffer_GetUsedBytes(buf);
    if (len==0x24)
      formatName="blog_24";
    else if (len==0x25)
      formatName="blog_25";
    else {
      DBG_ERROR(LC_LOGDOMAIN, "Invalid size of BLOG (%d)", len);
      GWEN_Buffer_free(buf);
      return LC_Client_ResultDataError;
    }

    if (LC_Card_ParseData(card, formatName, buf, dbCurr)) {
      DBG_ERROR(LC_LOGDOMAIN, "Error parsing record %d", i);
      GWEN_DB_Group_free(dbCurr);
    }
    else {
      int bseq;
      int lseq;

      bseq=GWEN_DB_GetIntValue(dbCurr, "bseq", 0, 0);
      lseq=GWEN_DB_GetIntValue(dbCurr, "lseq", 0, 0);
      if (bseq!=0 && lseq!=0) {
	const void *p;
	unsigned int bs;

	p=GWEN_DB_GetBinValue(dbCurr, "merchantId", 0, 0, 0, &bs);
	if (p && bs) {
	  GWEN_BUFFER *hexbuf;
  
	  hexbuf=GWEN_Buffer_new(0, 32, 0, 1);
	  if (GWEN_Text_ToHexBuffer(p, bs, hexbuf, 0, 0, 0))
	    abort();
	  GWEN_DB_DeleteVar(dbCurr, "merchantId");
	  GWEN_DB_SetCharValue(dbCurr, GWEN_DB_FLAGS_OVERWRITE_VARS,
			       "merchantId",
			       GWEN_Buffer_GetStart(hexbuf));
	  GWEN_Buffer_free(hexbuf);
	}
	DBG_DEBUG(LC_LOGDOMAIN, "Adding BLOG entry %d", ctxCount);
	GWEN_DB_AddGroup(dbData, dbCurr);
	ctxCount++;

      }
      else {
	DBG_WARN(LC_LOGDOMAIN, "Entry %d is empty",
		 idx?idx:i);
	GWEN_DB_Group_free(dbCurr);
      }
    }
    if (idx)
      break;
  } /* for */
  GWEN_Buffer_free(buf);

  if (!ctxCount) {
    return LC_Client_ResultNoData;
  }
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_GeldKarte_ReadBLogs(LC_CARD *card,
					LC_GELDKARTE_BLOG_LIST2 *bll) {
  LC_GELDKARTE *gk;
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbData;
  int count;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  dbData=GWEN_DB_Group_new("blogs");
  res=LC_GeldKarte__ReadBLog(card, 0, dbData);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbData);
    return res;
  }

  dbCurr=GWEN_DB_FindFirstGroup(dbData, "blog");
  count=0;
  while(dbCurr) {
    LC_GELDKARTE_BLOG *blog;
    const char *d, *t;
    int v;

    blog=LC_GeldKarte_BLog_new();
    LC_GeldKarte_BLog_SetStatus(blog,
				GWEN_DB_GetIntValue(dbCurr, "status", 0, 0));
    LC_GeldKarte_BLog_SetBSeq(blog,
			      GWEN_DB_GetIntValue(dbCurr, "bseq", 0, 0));
    LC_GeldKarte_BLog_SetLSeq(blog,
			      GWEN_DB_GetIntValue(dbCurr, "lseq", 0, 0));
    LC_GeldKarte_BLog_SetHSeq(blog,
			      GWEN_DB_GetIntValue(dbCurr, "hseq", 0, 0));
    LC_GeldKarte_BLog_SetSSeq(blog,
			      GWEN_DB_GetIntValue(dbCurr, "sseq", 0, 0));
    if (1!=sscanf(GWEN_DB_GetCharValue(dbCurr, "value", 0, "0"),
		  "%d", &v))
      v=0;
    LC_GeldKarte_BLog_SetValue(blog, v);
    if (1!=sscanf(GWEN_DB_GetCharValue(dbCurr, "loaded", 0, "0"),
		  "%d", &v))
      v=0;
    LC_GeldKarte_BLog_SetLoaded(blog, v);
    LC_GeldKarte_BLog_SetMerchantId(blog,
                                    GWEN_DB_GetCharValue(dbCurr,
                                                         "merchantId",
                                                         0, 0));
    d=GWEN_DB_GetCharValue(dbCurr, "date", 0, 0);
    t=GWEN_DB_GetCharValue(dbCurr, "time", 0, 0);
    if (d && t) {
      if (strcmp(d, "00000000")!=0 &&
          strcmp(d, "000000")!=0) {
	if (strcmp(t, "000000")==0) {
	  GWEN_BUFFER *dbuf;
	  GWEN_TIME *ti;

	  dbuf=GWEN_Buffer_new(0, 9, 0, 1);
          GWEN_Buffer_AppendString(dbuf, d);
          if (strlen(d)<8)
            ti=GWEN_Time_fromString(GWEN_Buffer_GetStart(dbuf), "YYMMDD");
          else
            ti=GWEN_Time_fromString(GWEN_Buffer_GetStart(dbuf), "YYYYMMDD");
          if (ti) {
	    LC_GeldKarte_BLog_SetTime(blog, ti);
	    GWEN_Time_free(ti);
	  }
	  else {
	    DBG_INFO(LC_LOGDOMAIN, "No/bad date/time in EF_BLOG record");
	  }
	}
	else {
	  GWEN_BUFFER *dbuf;
	  GWEN_TIME *ti;

	  dbuf=GWEN_Buffer_new(0, 15, 0, 1);
	  GWEN_Buffer_AppendString(dbuf, d);
	  GWEN_Buffer_AppendString(dbuf, t);
          if (strlen(d)<8)
            ti=GWEN_Time_fromString(GWEN_Buffer_GetStart(dbuf),
                                    "YYMMDDhhmmss");
          else
            ti=GWEN_Time_fromString(GWEN_Buffer_GetStart(dbuf),
                                    "YYYYMMDDhhmmss");
	  if (ti) {
	    LC_GeldKarte_BLog_SetTime(blog, ti);
	    GWEN_Time_free(ti);
	  }
	  else {
	    DBG_INFO(LC_LOGDOMAIN, "No/bad date/time in EF_BLOG record");
	  }
	}
      }
    }
    if (bll) {
      LC_GeldKarte_BLog_List2_PushBack(bll, blog);
      DBG_INFO(LC_LOGDOMAIN, "Added BLOG entry to list");
    }
    else
      LC_GeldKarte_BLog_free(blog);
    count++;

    dbCurr=GWEN_DB_FindNextGroup(dbCurr, "blog");
  }

  if (!count) {
    return LC_Client_ResultNoData;
  }
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_GeldKarte__ReadLLog(LC_CARD *card,
                                        int idx,
                                        GWEN_DB_NODE *dbData){
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbCurr;
  int i;
  unsigned int ctxCount;
  GWEN_BUFFER *buf;
  LC_GELDKARTE *gk;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  res=LC_Card_SelectEf(card, "EF_LLOG");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  ctxCount=0;
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  for (i=1; i<4; i++) {
    DBG_INFO(LC_LOGDOMAIN, "Reading LLOG record %d", i);
    GWEN_Buffer_Reset(buf);
    res=LC_Card_IsoReadRecord(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                              idx?idx:i, buf);
    if (res!=LC_Client_ResultOk)
      break;
    dbCurr=GWEN_DB_Group_new("llog");
    GWEN_Buffer_Rewind(buf);
    if (LC_Card_ParseRecord(card, idx?idx:i, buf, dbCurr)) {
      DBG_ERROR(LC_LOGDOMAIN, "Error parsing record %d", idx?idx:i);
      GWEN_DB_Group_free(dbCurr);
    }
    else {
      int bseq;
      int lseq;

      bseq=GWEN_DB_GetIntValue(dbCurr, "bseq", 0, 0);
      lseq=GWEN_DB_GetIntValue(dbCurr, "lseq", 0, 0);
      if (bseq!=0 && lseq!=0) {
	DBG_DEBUG(LC_LOGDOMAIN, "Adding LLOG entry %d", ctxCount);
	GWEN_DB_AddGroup(dbData, dbCurr);
	ctxCount++;
      }
      else {
	DBG_WARN(LC_LOGDOMAIN, "Entry %d is empty",
		 idx?idx:i);
	GWEN_DB_Group_free(dbCurr);
      }
    }
    if (idx)
      break;
  } /* for */
  GWEN_Buffer_free(buf);

  if (!ctxCount) {
    return LC_Client_ResultNoData;
  }
  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_GeldKarte_ReadLLogs(LC_CARD *card,
                                        LC_GELDKARTE_LLOG_LIST2 *bll) {
  LC_GELDKARTE *gk;
  LC_CLIENT_RESULT res;
  GWEN_DB_NODE *dbCurr;
  GWEN_DB_NODE *dbData;
  int count;

  assert(card);
  gk=GWEN_INHERIT_GETDATA(LC_CARD, LC_GELDKARTE, card);
  assert(gk);

  dbData=GWEN_DB_Group_new("llogs");
  res=LC_GeldKarte__ReadLLog(card, 0, dbData);
  if (res!=LC_Client_ResultOk) {
    GWEN_DB_Group_free(dbData);
    return res;
  }

  dbCurr=GWEN_DB_FindFirstGroup(dbData, "llog");
  count=0;
  while(dbCurr) {
    LC_GELDKARTE_LLOG *llog;
    const char *d, *t;
    int v;

    llog=LC_GeldKarte_LLog_new();
    LC_GeldKarte_LLog_SetStatus(llog,
				GWEN_DB_GetIntValue(dbCurr, "status", 0, 0));
    LC_GeldKarte_LLog_SetBSeq(llog,
			      GWEN_DB_GetIntValue(dbCurr, "bseq", 0, 0));
    LC_GeldKarte_LLog_SetLSeq(llog,
			      GWEN_DB_GetIntValue(dbCurr, "lseq", 0, 0));

    if (1!=sscanf(GWEN_DB_GetCharValue(dbCurr, "value", 0, "0"),
		  "%d", &v))
      v=0;
    LC_GeldKarte_LLog_SetValue(llog, v);
    if (1!=sscanf(GWEN_DB_GetCharValue(dbCurr, "loaded", 0, "0"),
		  "%d", &v))
      v=0;
    LC_GeldKarte_LLog_SetLoaded(llog, v);

    LC_GeldKarte_LLog_SetCenterId(llog,
                                  GWEN_DB_GetCharValue(dbCurr,
                                                       "centerId",
                                                       0, 0));
    LC_GeldKarte_LLog_SetTerminalId(llog,
                                    GWEN_DB_GetCharValue(dbCurr,
                                                         "terminalId",
                                                         0, 0));
    LC_GeldKarte_LLog_SetTraceId(llog,
                                 GWEN_DB_GetCharValue(dbCurr,
                                                      "traceId",
                                                      0, 0));
    d=GWEN_DB_GetCharValue(dbCurr, "date", 0, 0);
    t=GWEN_DB_GetCharValue(dbCurr, "time", 0, 0);
    if (d && t) {
      if (strcmp(d, "00000000")!=0) {
	if (strcmp(t, "000000")==0) {
	  GWEN_BUFFER *dbuf;
	  GWEN_TIME *ti;

	  dbuf=GWEN_Buffer_new(0, 9, 0, 1);
	  GWEN_Buffer_AppendString(dbuf, d);
	  ti=GWEN_Time_fromString(GWEN_Buffer_GetStart(dbuf),
				  "YYYYMMDD");
	  if (ti) {
	    LC_GeldKarte_LLog_SetTime(llog, ti);
	    GWEN_Time_free(ti);
	  }
	  else {
	    DBG_INFO(LC_LOGDOMAIN, "No/bad date/time in EF_BLOG record");
	  }
	}
	else {
	  GWEN_BUFFER *dbuf;
	  GWEN_TIME *ti;

	  dbuf=GWEN_Buffer_new(0, 15, 0, 1);
	  GWEN_Buffer_AppendString(dbuf, d);
	  GWEN_Buffer_AppendString(dbuf, t);
	  ti=GWEN_Time_fromString(GWEN_Buffer_GetStart(dbuf),
				  "YYYYMMDDhhmmss");
	  if (ti) {
	    LC_GeldKarte_LLog_SetTime(llog, ti);
	    GWEN_Time_free(ti);
	  }
	  else {
	    DBG_INFO(LC_LOGDOMAIN, "No/bad date/time in EF_BLOG record");
	  }
	}
      }
    }
    if (bll) {
      LC_GeldKarte_LLog_List2_PushBack(bll, llog);
      DBG_INFO(LC_LOGDOMAIN, "Added BLOG entry to list");
    }
    else
      LC_GeldKarte_LLog_free(llog);
    count++;

    dbCurr=GWEN_DB_FindNextGroup(dbCurr, "llog");
  }

  if (!count) {
    return LC_Client_ResultNoData;
  }
  return LC_Client_ResultOk;
}







