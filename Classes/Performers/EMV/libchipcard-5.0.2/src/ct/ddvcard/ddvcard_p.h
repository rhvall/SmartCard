/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHIPCARD_CT_DDV_P_H
#define CHIPCARD_CT_DDV_P_H


#include <gwenhywfar/ct_be.h>
#include <gwenhywfar/ctplugin.h>
#include <chipcard/card.h>


#define LC_CT_DDV_NUM_CONTEXT 5
#define LC_CT_DDV_NUM_KEY     2


typedef struct LC_CT_PLUGIN_DDV LC_CT_PLUGIN_DDV;

struct LC_CT_PLUGIN_DDV {
  LC_CLIENT *client;
};

GWEN_PLUGIN *LC_Crypt_TokenDDV_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					  const char *modName,
					  const char *fileName);

void GWENHYWFAR_CB LC_Crypt_TokenDDV_Plugin_FreeData(void *bp, void *p);




typedef struct LC_CT_DDV LC_CT_DDV;

struct LC_CT_DDV {
  GWEN_PLUGIN_MANAGER *pluginManager;
  GWEN_CRYPT_TOKEN_KEYINFO *keyInfos[LC_CT_DDV_NUM_KEY];
  GWEN_CRYPT_TOKEN_CONTEXT *contexts[LC_CT_DDV_NUM_CONTEXT];
  LC_CLIENT *client;
  LC_CARD *card;
  int havePin;
};

GWEN_CRYPT_TOKEN *LC_Crypt_TokenDDV_new(GWEN_PLUGIN_MANAGER *pm,
				       LC_CLIENT *lc,
				       const char *name);

void GWENHYWFAR_CB LC_Crypt_TokenDDV_FreeData(void *bp, void *p);

int LC_Crypt_TokenDDV__GetCard(GWEN_CRYPT_TOKEN *ct,
			       uint32_t guiid);

int LC_Crypt_TokenDDV__EnterPin(GWEN_CRYPT_TOKEN *ct,
				LC_CARD *hcard,
				GWEN_CRYPT_PINTYPE pt);



int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_GetKeyIdList(GWEN_CRYPT_TOKEN *ct,
				 uint32_t *pIdList,
				 uint32_t *pCount,
				 uint32_t gid);

const GWEN_CRYPT_TOKEN_KEYINFO* GWENHYWFAR_CB
  LC_Crypt_TokenDDV_GetKeyInfo(GWEN_CRYPT_TOKEN *ct,
			       uint32_t id,
			       uint32_t flags,
			       uint32_t gid);

int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_SetKeyInfo(GWEN_CRYPT_TOKEN *ct,
			       uint32_t id,
			       const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			       uint32_t gid);

int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_GetContextIdList(GWEN_CRYPT_TOKEN *ct,
				     uint32_t *pIdList,
				     uint32_t *pCount,
				     uint32_t gid);

const GWEN_CRYPT_TOKEN_CONTEXT* GWENHYWFAR_CB
  LC_Crypt_TokenDDV_GetContext(GWEN_CRYPT_TOKEN *ct,
			       uint32_t id,
			       uint32_t gid);

int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_SetContext(GWEN_CRYPT_TOKEN *ct,
			       uint32_t id,
			       const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
			       uint32_t gid);


int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Sign(GWEN_CRYPT_TOKEN *ct,
			 uint32_t keyId,
			 GWEN_CRYPT_PADDALGO *a,
			 const uint8_t *pInData,
			 uint32_t inLen,
			 uint8_t *pSignatureData,
			 uint32_t *pSignatureLen,
			 uint32_t *pSeqCounter,
			 uint32_t gid);

int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Verify(GWEN_CRYPT_TOKEN *ct,
			   uint32_t keyId,
			   GWEN_CRYPT_PADDALGO *a,
			   const uint8_t *pInData,
			   uint32_t inLen,
			   const uint8_t *pSignatureData,
			   uint32_t signatureLen,
			   uint32_t seqCounter,
			   uint32_t gid);

int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Encipher(GWEN_CRYPT_TOKEN *ct,
			     uint32_t keyId,
			     GWEN_CRYPT_PADDALGO *a,
			     const uint8_t *pInData,
			     uint32_t inLen,
			     uint8_t *pOutData,
			     uint32_t *pOutLen,
			     uint32_t gid);

int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Decipher(GWEN_CRYPT_TOKEN *ct,
			     uint32_t keyId,
			     GWEN_CRYPT_PADDALGO *a,
			     const uint8_t *pInData,
			     uint32_t inLen,
			     uint8_t *pOutData,
			     uint32_t *pOutLen,
			     uint32_t gid);


int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Open(GWEN_CRYPT_TOKEN *ct, int admin, uint32_t guiid);
int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t guiid);




GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Plugin_CreateToken(GWEN_PLUGIN *pl,
				       const char *name);

GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
LC_Crypt_TokenDDV_Plugin_CreateToken(GWEN_PLUGIN *pl,
						       const char *name);

int GWENHYWFAR_CB
  LC_Crypt_TokenDDV_Plugin_CheckToken(GWEN_PLUGIN *pl,
				      GWEN_BUFFER *name);

GWEN_PLUGIN *LC_Crypt_TokenDDV_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					  const char *modName,
					  const char *fileName);

int LC_Crypt_TokenDDV__ReadSignSeq(GWEN_CRYPT_TOKEN *ct,
				   uint32_t kid,
				   uint32_t *pSigCounter);

int LC_Crypt_TokenDDV__WriteSignSeq(GWEN_CRYPT_TOKEN *ct,
				    uint32_t kid,
				    uint32_t sigCounter);


int LC_Crypt_TokenDDV__IncSignSeq(GWEN_CRYPT_TOKEN *ct,
				  uint32_t kid,
				  uint32_t *pSigCount);


#endif

