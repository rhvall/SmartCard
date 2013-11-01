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


/* You always need to include the header files of Libchipcard2 to work with
 * it ;-)
 */
#include <chipcard/chipcard.h>
#include <chipcard/client.h>


/**
 * Please go to the source of this for a crosslinked view (see link below).
 * @callgraph
 */
int main(int argc, char **argv) {
  LC_CLIENT *cl;
  LC_CARD *card;

  cl=LC_Client_new("tutorial1a", "1.0");
  LC_Client_Init(cl);

  LC_Client_Start(cl);

  fprintf(stderr, "Please insert a chip card.\n");
  LC_Client_GetNextCard(cl, &card, 30);

  LC_Client_Stop(cl);

  LC_Card_Open(card);

  LC_Card_Dump(card, 0);

  LC_Card_Close(card);
  LC_Client_ReleaseCard(cl, card);
  LC_Card_free(card);

  LC_Client_free(cl);
  return 0;
}




