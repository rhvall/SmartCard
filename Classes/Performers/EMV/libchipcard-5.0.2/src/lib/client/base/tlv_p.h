/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CLIENT_TLV_P_H
#define CHIPCARD_CLIENT_TLV_P_H


#include <chipcard/tlv.h>


struct LC_TLV {
  GWEN_LIST_ELEMENT(LC_TLV)
  int isBerTlv;
  unsigned int tagMode;
  unsigned int tagSize;
  unsigned int tagType;
  unsigned int tagLength;
  void *tagData;
};




#endif /* CHIPCARD_CLIENT_TLV_P_H */
