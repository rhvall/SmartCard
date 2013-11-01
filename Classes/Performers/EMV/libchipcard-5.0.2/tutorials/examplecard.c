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


#include "examplecard_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <chipcard/chipcard.h>
#include <chipcard/cards/processorcard.h>


/* This must be at the top of the file to tell GWEN that we are to inherit
 * another type in this file. Internally GWEN stores the hash value of the
 * types involved to speed up the lookup of type specific data in a base type
 * object.
 */
GWEN_INHERIT(LC_CARD, EXAMPLE_CARD)


/* This function establishes the heritage and sets its own Open() and Close()
 * functions. This function must be called before LC_Card_Open() is.
 * Also, this function must be called before using any other function of the
 * inheriting type.
 */
int ExampleCard_ExtendCard(LC_CARD *card){
  EXAMPLE_CARD *xc;
  int rv;

  rv=LC_ProcessorCard_ExtendCard(card);
  if (rv) {
    DBG_ERROR(LC_LOGDOMAIN, "Could not extend card as processor card");
    return rv;
  }

  GWEN_NEW_OBJECT(EXAMPLE_CARD, xc);

  xc->openFn=LC_Card_GetOpenFn(card);
  xc->closeFn=LC_Card_GetCloseFn(card);
  LC_Card_SetOpenFn(card, ExampleCard_Open);
  LC_Card_SetCloseFn(card, ExampleCard_Close);

  GWEN_INHERIT_SETDATA(LC_CARD, EXAMPLE_CARD, card, xc,
                       ExampleCard_freeData);
  return 0;
}



/* This function can be used to resolve the heritage established via
 * ExampleCard_ExtendCard(). After this function has been called no other
 * ExampleCard function (except ExampleCard_ExtendCard()) may be called.
 * Please note that this function resets the cards' internal Open() and
 * Close() function pointers to the values found upon execution of
 * ExampleCard_ExtendCard(), so after that the function ExampleCard_Close()
 * will no longer be called internally by LC_Card_Close().
 */
int ExampleCard_UnextendCard(LC_CARD *card){
  EXAMPLE_CARD *xc;
  int rv;

  xc=GWEN_INHERIT_GETDATA(LC_CARD, EXAMPLE_CARD, card);
  assert(xc);
  LC_Card_SetOpenFn(card, xc->openFn);
  LC_Card_SetCloseFn(card, xc->closeFn);
  GWEN_INHERIT_UNLINK(LC_CARD, EXAMPLE_CARD, card);
  GWEN_FREE_OBJECT(xc);

  rv=LC_ProcessorCard_UnextendCard(card);
  if (rv) {
    DBG_INFO(LC_LOGDOMAIN, "here");
  }
  return rv;
}


/* This function is called internally by LC_Card_free() if the card is still
 * extended as an ExampleCard. It should free all data associated with the
 * LC_CARD object concerning ExampleCard data.
 * bp is a pointer to the lowest base type (in this case LC_CARD) and p is
 * a pointer to the derived type (in this case EXAMPLE_CARD). You need to cast
 * these pointers to their real types respectively.
 */
void GWENHYWFAR_CB ExampleCard_freeData(void *bp, void *p){
  EXAMPLE_CARD *xc;

  assert(bp);
  assert(p);
  xc=(EXAMPLE_CARD*)p;
  GWEN_FREE_OBJECT(xc);
}


/* This function is called internally by LC_Card_Open(). It is always a good
 * idea to call the Open() function found upon ExampleCard_ExtendCard(),
 * first. After that you can do whatever your card needs to be done.
 * Well, in the case of this example here we simply select the MasterFile
 * (otherwise called "root" in disk file systems).
 * Also, it is best to split the card specific stuff off into an extra
 * function called _Reopen() (in this case ExampleCard_Reopen), so that it
 * may later be used again, because the function LC_Card_Open() also does
 * the connection work and must therefore only be called *once*.
 */
LC_CLIENT_RESULT CHIPCARD_CB ExampleCard_Open(LC_CARD *card){
  LC_CLIENT_RESULT res;
  EXAMPLE_CARD *xc;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening card as ExampleCard");

  assert(card);
  xc=GWEN_INHERIT_GETDATA(LC_CARD, EXAMPLE_CARD, card);
  assert(xc);

  res=xc->openFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  res=ExampleCard_Reopen(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    xc->closeFn(card);
    return res;
  }

  return LC_Client_ResultOk;
}


/* As discussed above this is the card specific setup. */
LC_CLIENT_RESULT ExampleCard_Reopen(LC_CARD *card){
  LC_CLIENT_RESULT res;
  EXAMPLE_CARD *xc;

  DBG_DEBUG(LC_LOGDOMAIN, "Opening ExampleCard");

  assert(card);
  xc=GWEN_INHERIT_GETDATA(LC_CARD, EXAMPLE_CARD, card);
  assert(xc);

  DBG_DEBUG(LC_LOGDOMAIN, "Selecting Example card application");
  res=LC_Card_SelectApp(card, "ExampleCard");
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  DBG_DEBUG(LC_LOGDOMAIN, "Selecting MF...");
  res=LC_Card_SelectMf(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return LC_Client_ResultOk;
}


/* This function is called internally upon LC_Card_Close().
 * It *must* call the function found upon ExampleCard_ExtendCard() otherwise
 * the card is not released !
 * Additionally, you can do your own deinit stuff here before actually
 * releasing the card.
 */
LC_CLIENT_RESULT CHIPCARD_CB ExampleCard_Close(LC_CARD *card){
  LC_CLIENT_RESULT res;
  EXAMPLE_CARD *xc;

  assert(card);
  xc=GWEN_INHERIT_GETDATA(LC_CARD, EXAMPLE_CARD, card);
  assert(xc);

  res=xc->closeFn(card);
  if (res!=LC_Client_ResultOk) {
    DBG_INFO(LC_LOGDOMAIN, "here");
    return res;
  }

  return res;
}


/* This is just an example of how to access data stored in the card type
 * extension
 */
int ExampleCard_GetExampleData(const LC_CARD *card){
  EXAMPLE_CARD *xc;

  assert(card);
  xc=GWEN_INHERIT_GETDATA(LC_CARD, EXAMPLE_CARD, card);
  assert(xc);

  return xc->exampleData;
}



int test_type(TYPE_VISIBLE *v) {
  return test_type(v);
  return ((EXAMPLE_CARD*)v)->exampleData;
}



