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


#define LC_CT_STARCOS_NUM_CONTEXT 5
#define LC_CT_STARCOS_NUM_KEY     20


typedef struct LC_CT_PLUGIN_STARCOS LC_CT_PLUGIN_STARCOS;

struct LC_CT_PLUGIN_STARCOS {
  LC_CLIENT *client;
};

static void GWENHYWFAR_CB LC_Crypt_TokenStarcos_Plugin_FreeData(void *bp, void *p);




typedef struct LC_CT_STARCOS LC_CT_STARCOS;

struct LC_CT_STARCOS {
  GWEN_PLUGIN_MANAGER *pluginManager;
  GWEN_CRYPT_TOKEN_KEYINFO *keyInfos[LC_CT_STARCOS_NUM_KEY];
  GWEN_CRYPT_TOKEN_CONTEXT *contexts[LC_CT_STARCOS_NUM_CONTEXT];
  LC_CLIENT *client;
  LC_CARD *card;
  int haveAccessPin;
  int haveAdminPin;
};

static GWEN_CRYPT_TOKEN *LC_Crypt_TokenStarcos_new(GWEN_PLUGIN_MANAGER *pm,
						   LC_CLIENT *lc,
						   const char *name);

static void GWENHYWFAR_CB LC_Crypt_TokenStarcos_FreeData(void *bp, void *p);

static int LC_Crypt_TokenStarcos__GetCard(GWEN_CRYPT_TOKEN *ct,
					  uint32_t guiid);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_GetKeyIdList(GWEN_CRYPT_TOKEN *ct,
				     uint32_t *pIdList,
				     uint32_t *pCount,
				     uint32_t gid);

static const GWEN_CRYPT_TOKEN_KEYINFO* GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_GetKeyInfo(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   uint32_t flags,
				   uint32_t gid);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_SetKeyInfo(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				   uint32_t gid);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_GetContextIdList(GWEN_CRYPT_TOKEN *ct,
					 uint32_t *pIdList,
					 uint32_t *pCount,
					 uint32_t gid);

static const GWEN_CRYPT_TOKEN_CONTEXT* GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_GetContext(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   uint32_t gid);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_SetContext(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
				   uint32_t gid);


static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Sign(GWEN_CRYPT_TOKEN *ct,
			     uint32_t keyId,
			     GWEN_CRYPT_PADDALGO *a,
			     const uint8_t *pInData,
			     uint32_t inLen,
			     uint8_t *pSignatureData,
			     uint32_t *pSignatureLen,
			     uint32_t *pSeqCounter,
			     uint32_t gid);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Verify(GWEN_CRYPT_TOKEN *ct,
			       uint32_t keyId,
			       GWEN_CRYPT_PADDALGO *a,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       const uint8_t *pSignatureData,
			       uint32_t signatureLen,
			       uint32_t seqCounter,
			       uint32_t gid);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Encipher(GWEN_CRYPT_TOKEN *ct,
				 uint32_t keyId,
				 GWEN_CRYPT_PADDALGO *a,
				 const uint8_t *pInData,
				 uint32_t inLen,
				 uint8_t *pOutData,
				 uint32_t *pOutLen,
				 uint32_t gid);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Decipher(GWEN_CRYPT_TOKEN *ct,
				 uint32_t keyId,
				 GWEN_CRYPT_PADDALGO *a,
				 const uint8_t *pInData,
				 uint32_t inLen,
				 uint8_t *pOutData,
				 uint32_t *pOutLen,
				 uint32_t gid);


static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Open(GWEN_CRYPT_TOKEN *ct, int admin, uint32_t guiid);
static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t guiid);




static GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Plugin_CreateToken(GWEN_PLUGIN *pl,
					   const char *name);

static GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Plugin_CreateToken(GWEN_PLUGIN *pl,
					   const char *name);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_Plugin_CheckToken(GWEN_PLUGIN *pl,
					  GWEN_BUFFER *name);

static GWEN_PLUGIN *LC_Crypt_TokenStarcos_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
						     const char *modName,
						     const char *fileName);

static int GWENHYWFAR_CB
  LC_Crypt_TokenStarcos_GenerateKey(GWEN_CRYPT_TOKEN *ct,
				    uint32_t kid,
				    const GWEN_CRYPT_CRYPTALGO *a,
				    uint32_t gid);

static int LC_Crypt_TokenStarcos__ReadKeyInfo(GWEN_CRYPT_TOKEN *ct,
					      uint32_t kid,
					      uint32_t gid);


#endif

