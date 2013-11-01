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


/*
 * This is a small tutorial on how to use the basic functions of
 * libchipcard. It just waits for a card to be inserted and prints some
 * card's information.
 * This is the most basic type of application using a chipcard, no error
 * checking is performed.
 *
 * This tutorial is intended to show the basics only.
 * After studying this tutorial you should advance to the next one, which
 * will add some error checking.
 *
 * Usage:
 *   tutorial1b
 */


int main(int argc, char **argv) {
  /* The basic object of Libchipcard itself is LC_CLIENT.
   * You must create and initialize such an object before doing anything
   * with Libchipcard.
   */
  LC_CLIENT *cl;

  /* The other central object is LC_CARD. This is the object most card
   * commands operate on.
   */
  LC_CARD *card;

  /* Create an instance of Libchipcard.
   * Libchipcard wants to know what application is requesting its service to
   * improve server-side logging. It also makes it easier to debug
   * Libchipcard.
   * The last parameter is the path to the data folder of Libchipcard.
   * We don't want any special handling here so we provide a NULL to make
   * Libchipcard use its default path.
   */
  cl=LC_Client_new("tutorial1b", "1.0");

  /* Initialize Libchipcard by reading its configuration file.
   */
  LC_Client_Init(cl);

  /* We now need to tell Libchipcard that we are interested in chipcards.
   * After sending this command the chipcard server will notify us about
   * available cards.
   * Only now the server will be connected, and if we are the only client
   * then the server now starts acquiring card readers.
   */
  LC_Client_Start(cl);

  /* It's always nice to tell the user what we expect of him. */
  fprintf(stderr, "Please insert a chip card.\n");

  /* Now that the server is informed about us being interested in chipcards
   * we can just wait for the server to notify us about inserted cards.
   * We will wait for about 30 seconds.
   * The card retrieved via this call is automatically assigned exclusively
   * to the caller, so we must release the card using LC_Client_ReleaseCard()
   * as soon as we are finished with the card.
   */
  LC_Client_GetNextCard(cl, &card, 30);

  /* Now that we have found a card we can tell the server that we don't want
   * to be informed about other cards.
   */
  LC_Client_Stop(cl);

  /* This performs some internal card type specific functions.
   */
  LC_Card_Open(card);

  /* Show the generic information available for this card
   */
  LC_Card_Dump(card, 0);

  /* Performs some internal card type specific functions.
   */
  LC_Card_Close(card);

  /* After working with the card we should always release it so that other
   * clients may access it. If no other client is interested in this card
   * the reader is shutdown after a grace period, so releasing a card after
   * using it is always a good idea.
   */
  LC_Client_ReleaseCard(cl, card);

  /* Release all ressources assiciated with the given card.
   */
  LC_Card_free(card);

  /* Release all ressources associated with Libchipcard.
   * You should always do this at the end of your program to prevent
   * memory leaks.
   */
  LC_Client_free(cl);

  /* Aaaand that's it ;-)
   */
  return 0;
}



