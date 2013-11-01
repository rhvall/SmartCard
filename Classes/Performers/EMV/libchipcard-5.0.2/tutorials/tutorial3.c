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


#include <chipcard/chipcard.h>
#include <chipcard/client.h>
#include "examplecard.h"


/*
 * While tutorial1 only works with base classes and tutorial2 only *uses*
 * derived cards we are now to understand how more complex card classes can be
 * created.
 *
 * Now we define a card ourselves. The new card type is called "ExampleCard"
 * and it is a processor card. Its definition can be found in the files
 * examplecard.h, examplecard_p.h, examplecard.c and examplecard.xml.
 *
 * The file "examplecard.xml" defines which "files" (called "DF" and "EF" on
 * chip cards) exist on such a card. It may also define data formats used
 * by the card which can be used with the functions LC_Card_ParseData() and
 * LC_Card_CreateData(). This file is not needed for card types which are
 * derived from other types (via an _ExtendCard() function) and which do not
 * have additional files/formats.
 * But for this tutorial we show a complete setup.
 *
 * Please note that you have to do "make install" before executing this
 * tutorial because the XML file needs to be installed in Libchipcard2's
 * data folder to be found.
 *
 * Usage:
 *   tutorial3
 */


/* This function explains an error */
void showError(LC_CARD *card, LC_CLIENT_RESULT res,
               const char *failedCommand) {
  const char *s;

  switch(res) {
  case LC_Client_ResultOk:
    s="Ok.";
    break;
  case LC_Client_ResultWait:
    s="Timeout.";
    break;
  case LC_Client_ResultIpcError:
    s="IPC error.";
    break;
  case LC_Client_ResultCmdError:
    s="Command error.";
    break;
  case LC_Client_ResultDataError:
    s="Data error.";
    break;
  case LC_Client_ResultAborted:
    s="Aborted.";
    break;
  case LC_Client_ResultInvalid:
    s="Invalid argument to command.";
    break;
  case LC_Client_ResultInternal:
    s="Internal error.";
    break;
  case LC_Client_ResultGeneric:
    s="Generic error.";
    break;
  default:
    s="Unknown error.";
    break;
  }

  fprintf(stderr, "Error in \"%s\": %s\n", failedCommand, s);
  if (res==LC_Client_ResultCmdError) {
    int sw1;
    int sw2;

    sw1=LC_Card_GetLastSW1(card);
    sw2=LC_Card_GetLastSW2(card);
    fprintf(stderr, "  Last card command result:\n");
    if (sw1!=-1 && sw2!=-1)
      fprintf(stderr, "   SW1=%02x, SW2=%02x\n", sw1, sw2);
    s=LC_Card_GetLastResult(card);
    if (s)
      fprintf(stderr, "   Result: %s\n", s);
    s=LC_Card_GetLastText(card);
    if (s)
      fprintf(stderr, "   Text  : %s\n", s);
  }
}



int main(int argc, char **argv) {
  LC_CLIENT *cl;
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  int rv;
  int i;

  cl=LC_Client_new("tutorial3", "1.0");
  res=LC_Client_Init(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Init");
    LC_Client_free(cl);
    return 1;
  }

  fprintf(stderr, "INFO: Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "StartWait");
    LC_Client_free(cl);
    return 2;
  }

  fprintf(stderr, "Please insert an EC card or a GeldKarte.\n");
  res=LC_Client_GetNextCard(cl, &card, 30);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "GetNextCard");
    LC_Client_Stop(cl);
    LC_Client_free(cl);
    return 2;
  }

  /* stop waiting */
  fprintf(stderr, "INFO: Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    LC_Client_ReleaseCard(cl, card);
    LC_Card_free(card);
    LC_Client_free(cl);
    return 2;
  }

  /* ======================================================================
   * Until now we only handled basic card functions as the other tutorials
   * did.
   * The inserted card is supposed to be an EC card or a GeldKarte, so we need
   * to tell Libchipcard2 that we want to use it as such. This makes sure that
   * the correct card commands for the reader/card combination is used
   * internally.
   * The following function also sets more specific functions for the complex
   * card type to be called internally upon LC_Card_Open() and
   * LC_Card_Close(), so we need to call the _ExtendCard() function before
   * the function LC_Card_Open() !
   *
   * Please always remember to unextend an extended card before extending it
   * as a different type.
   *
   * This is a fine example of the heritage model used in Libchipcard2:
   * You could also create your own card type by extending an existing one.
   */
  rv=ExampleCard_ExtendCard(card);
  if (rv) {
    fprintf(stderr, "Could not extend card as ExampleCard\n");
    return 2;
  }

  /* open card */
  fprintf(stderr, "INFO: Opening card.\n");

  /* This now internally calls the ExampleCard_Open() function */
  fprintf(stderr, "INFO: Opening card.\n");
  res=LC_Card_Open(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardOpen");
    LC_Client_ReleaseCard(cl, card);
    LC_Card_free(card);
    LC_Client_free(cl);
    return 2;
  }

  /* Since the card has been extended as an ExampleCard we can now
   * use the functions of that module.
   * In this case we retrieve the user information stored on the card.
   */
  i=ExampleCard_GetExampleData(card);
  fprintf(stderr, "Example Data: %d\n", i);

  /* ====================================================================== */

  /* close card */
  fprintf(stderr, "INFO: Closing card.\n");
  /* This now internally calls the ExampleCard_Close() function */
  res=LC_Card_Close(card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardClose");
    LC_Client_ReleaseCard(cl, card);
    LC_Card_free(card);
    LC_Client_free(cl);
    return 2;
  }
  fprintf(stderr, "INFO: Card closed.\n");

  /* release card */
  res=LC_Client_ReleaseCard(cl, card);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "CardRelease");
    LC_Card_free(card);
    LC_Client_free(cl);
    return 2;
  }

  /* cleanup */
  LC_Card_free(card);
  LC_Client_free(cl);
  return 0;
}


