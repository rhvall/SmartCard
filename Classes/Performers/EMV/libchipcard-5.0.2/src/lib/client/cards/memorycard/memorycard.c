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


#include "memorycard_p.h"
#include <chipcard/chipcard.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>


GWEN_INHERIT(LC_CARD, LC_MEMORYCARD)



int LC_MemoryCard_ExtendCard(LC_CARD *card){
  LC_MEMORYCARD *mc;

  GWEN_NEW_OBJECT(LC_MEMORYCARD, mc);

  mc->openFn=LC_Card_GetOpenFn(card);
  mc->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, LC_MemoryCard_Open);
  LC_Card_SetCloseFn(card, LC_MemoryCard_Close);

  GWEN_INHERIT_SETDATA(LC_CARD, LC_MEMORYCARD, card, mc,
                       LC_MemoryCard_freeData);

  LC_MemoryCard__CalculateCapacity(card);

  return 0;
}



int LC_MemoryCard_UnextendCard(LC_CARD *card){
  LC_MEMORYCARD *mc;

  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);
  LC_Card_SetOpenFn(card, mc->openFn);
  LC_Card_SetCloseFn(card, mc->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, LC_MEMORYCARD, card);
  return 0;
}



void GWENHYWFAR_CB LC_MemoryCard_freeData(void *bp, void *p){
  LC_MEMORYCARD *mc;

  assert(bp);
  assert(p);
  mc=(LC_MEMORYCARD*)p;
  GWEN_FREE_OBJECT(mc);
}



LC_CLIENT_RESULT CHIPCARD_CB LC_MemoryCard_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_MEMORYCARD *mc;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening card as memory card");

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);

  res=mc->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=LC_MemoryCard_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    mc->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_MemoryCard_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_MEMORYCARD *mc;
  int i;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening memory card");

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);

  DBG_DEBUG(LC_LOGDOMAIN, "Selecting memory card and app");
  res=LC_Card_SelectCard(card, "MemoryCard");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }
  res=LC_Card_SelectApp(card, "MemoryCard");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  i=LC_MEMORYCARD_DEFAULT_WRITEBOUNDARY;
  if (LC_Card_GetReaderFlags(card) & LC_READER_FLAGS_LOW_WRITE_BOUNDARY)
    i=32;
  mc->writeBoundary=i;

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT CHIPCARD_CB LC_MemoryCard_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  LC_MEMORYCARD *mc;

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);

  res=mc->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}



LC_CLIENT_RESULT LC_MemoryCard_ReadBinary(LC_CARD *card,
                                          int offset,
                                          int size,
                                          GWEN_BUFFER *buf){
  int t;
  LC_MEMORYCARD *mc;
  int bytesRead=0;
  LC_CLIENT_RESULT res;

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);

  while(size>0) {
    if (size>252)
      t=252;
    else
      t=size;
    res=LC_Card_IsoReadBinary(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                              offset, t, buf);
    if (res!=LC_Client_ResultOk) {
      if (res==LC_Client_ResultNoData && bytesRead)
        return LC_Client_ResultOk;
      return res;
    }

    size-=t;
    offset+=t;
    bytesRead+=t;
  } /* while still data to read */

  return LC_Client_ResultOk;
}



LC_CLIENT_RESULT LC_MemoryCard_WriteBinary(LC_CARD *card,
                                           int offset,
                                           const char *ptr,
                                           unsigned int size) {
  LC_MEMORYCARD *mc;
  LC_CLIENT_RESULT res;

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);

  while(size>0) {
    int t;
    int j;
  
    /* calculate the position at the next boundary */
    j=(((offset)/mc->writeBoundary)+1)*mc->writeBoundary;
    /* read the distance from the current offset to that next boundary */
    t=j-(offset);
    if (t>size)
      t=size;

    res=LC_Card_IsoUpdateBinary(card, LC_CARD_ISO_FLAGS_RECSEL_GIVEN,
                                offset, ptr, t);
    if (res!=LC_Client_ResultOk)
      return res;

    size-=t;
    offset+=t;
    ptr+=t;
  } /* while still data to write */

  return LC_Client_ResultOk;
}



void LC_MemoryCard__CalculateCapacity(LC_CARD *card){
  LC_MEMORYCARD *mc;
  int i1, i2;
  int j1, j2;
  const unsigned char *p;
  unsigned int atrLen;

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);

  mc->capacity=0;
  atrLen=LC_Card_GetAtr(card, &p);
  if (atrLen==0 || p==0) {
    DBG_WARN(LC_LOGDOMAIN, "No ATR");
    return;
  }

  if (atrLen<2) {
    DBG_WARN(LC_LOGDOMAIN, "ATR too small");
    return;
  }

  i1=( ( (unsigned char)p[1] ) >> 3 ) & 0x0f; /* count of elements */
  i2=( (unsigned char)p[1] ) & 0x07;      /* size of element */

  /* check element number */
  if (i1==0)
    j1=1;
  else
    j1=1<<i1;
  j1*=64;

  /* check element size */
  if (i2==0)
    j2=1;
  else
    j2=1<<i2;

  /* calculate memory size */
  if (j1 && j2)
    mc->capacity=j1*j2/8;
  DBG_DEBUG(LC_LOGDOMAIN, "Capacity is: %d", mc->capacity);
}



unsigned int LC_MemoryCard_GetCapacity(const LC_CARD *card){
  LC_MEMORYCARD *mc;

  assert(card);
  mc=GWEN_INHERIT_GETDATA(LC_CARD, LC_MEMORYCARD, card);
  assert(mc);

  return mc->capacity;
}







