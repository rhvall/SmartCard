/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef CHIPCARD_COMMON_DRIVERINFO_P_H
#define CHIPCARD_COMMON_DRIVERINFO_P_H

#include "driverinfo.h"

#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/db.h>

/**
 * Search for a file in a list of folders.
 * @param slDirs list of folders to scan
 * @param slNames list of names for the file to be found
 * @param nbuf buffer to receive the name of the file found (if any)
 */
static
int LC_DriverInfo_FindFile(GWEN_STRINGLIST *slDirs,
                           GWEN_STRINGLIST *slNames,
                           GWEN_BUFFER *nbuf);


static
GWEN_DB_NODE *LC_DriverInfo_DriverDbFromXml(GWEN_XMLNODE *node,
					    int dontSearchDrivers);

static
GWEN_DB_NODE *LC_DriverInfo_ReaderDbFromXml(GWEN_XMLNODE *node);

static
int LC_DriverInfo_ReadDriverFile(const char *fname,
                                 GWEN_DB_NODE *dbDrivers,
				 int availOnly,
				 int dontSearchDrivers);


#endif
