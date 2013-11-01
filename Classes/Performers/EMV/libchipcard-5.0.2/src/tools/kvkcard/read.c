

static int storeCardData(const char *fname,
			 LC_CARD *cd,
			 int dosMode,
                         const char *cardType,
			 const char *cardNumber,
			 const LC_HI_PERSONAL_DATA *pData,
			 const LC_HI_INSURANCE_DATA *iData) {
  FILE *f;
  GWEN_BUFFER *dbuf;
  GWEN_TIME *ti;
  const char *CRLF;
  const char *s;
  const GWEN_TIME *cti;

  ti=GWEN_CurrentTime();
  assert(ti);

  if (dosMode)
    CRLF="\r\n";
  else
    CRLF="\n";

  if (fname) {
    f=fopen(fname, "w+");
    if (f==0) {
      fprintf(stderr,
	      "Could not create file \"%s\", reason: %s\n",
	      fname,
	      strerror(errno));
      GWEN_Time_free(ti);
      return -1;
    }
  }
  else
    f=stdout;

  /* header */
  fprintf(f,"Version:libchipcard4-"CHIPCARD_VERSION_FULL_STRING"%s", CRLF);
  dbuf=GWEN_Buffer_new(0, 32, 0, 1);
  GWEN_Time_toString(ti, "DD.MM.YYYY", dbuf);
  fprintf(f, "Datum:%s%s", GWEN_Buffer_GetStart(dbuf), CRLF);
  GWEN_Buffer_Reset(dbuf);
  GWEN_Time_toString(ti, "hh:mm:ss", dbuf);
  fprintf(f, "Zeit:%s%s", GWEN_Buffer_GetStart(dbuf), CRLF);
  GWEN_Time_free(ti);
  GWEN_Buffer_free(dbuf); dbuf=0;
  fprintf(f, "Lesertyp:%s%s", LC_Card_GetReaderType(cd), CRLF);
  fprintf(f, "Kartentyp:%s%s", cardType?cardType:"", CRLF);

  /* insurance data */
  s=LC_HIInsuranceData_GetInstitutionName(iData);
  fprintf(f,"KK-Name:%s%s", s?s:"", CRLF);
  s=LC_HIInsuranceData_GetInstitutionId(iData);
  fprintf(f,"KK-Nummer:%s%s", s?s:"", CRLF);
  /* this is not the card number but the "Vertragskassennummer" */
  fprintf(f,"VKNR:%s%s", cardNumber?cardNumber:"", CRLF);
  s=LC_HIPersonalData_GetInsuranceId(pData);
  fprintf(f,"V-Nummer:%s%s", s?s:"", CRLF);
  s=LC_HIInsuranceData_GetStatus(iData);
  fprintf(f,"V-Status:%s%s", s?s:"", CRLF);
  s=LC_HIInsuranceData_GetGroup(iData);
  fprintf(f,"V-Statusergaenzung:%s%s", s?s:"", CRLF);
  if (s) {
    const char *x=0;

    switch(*s) {
    case '1': x="west"; break;
    case '9': x="ost"; break;
    case '6': x="BVG"; break;
    case '7': x="SVA, nach Aufwand, dt.-nl Grenzgaenger"; break;
    case '8': x="SVA, pauschal"; break;
    case 'M': x="DMP Diabetes mellitus Typ 2, west"; break;
    case 'X': x="DMP Diabetes mellitus Typ 2, ost"; break;
    case 'A': x="DMP Brustkrebs, west"; break;
    case 'C': x="DMP Brustkrebs, ost"; break;
    case 'K': x="DMP KHK, west"; break;
    case 'L': x="DMP KHK, ost"; break;
    case '4': x="nichtversicherter Sozialhilfe-Empfaenger"; break;
    case 'E': x="DMP Diabetes mellitus Typ 1, west"; break;
    case 'N': x="DMP Diabetes mellitus Typ 1, ost"; break;
    case 'D': x="DMP Asthma bronchiale, west"; break;
    case 'F': x="DMP Asthma bronchiale, ost"; break;
    case 'S': x="DMP COPD, west"; break;
    case 'P': x="DMP COPD, ost"; break;
    default:  x=0;
    }
    if (x)
      fprintf(f,"V-Status-Erlaeuterung:%s%s", x, CRLF);
  }
  s=LC_HIPersonalData_GetTitle(pData);
  fprintf(f,"Titel:%s%s", s?s:"", CRLF);
  s=LC_HIPersonalData_GetPrename(pData);
  fprintf(f,"Vorname:%s%s", s?s:"", CRLF);
  s=LC_HIPersonalData_GetNameSuffix(pData);
  fprintf(f,"Namenszusatz:%s%s", s?s:"", CRLF);
  s=LC_HIPersonalData_GetName(pData);
  fprintf(f,"Familienname:%s%s", s?s:"", CRLF);
  cti=LC_HIPersonalData_GetDateOfBirth(pData);
  if (cti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Time_toUtcString(cti, "DDMMYYYY", tbuf);
    fprintf(f,"Geburtsdatum:%s%s",
	    GWEN_Buffer_GetStart(tbuf), CRLF);
    GWEN_Buffer_free(tbuf);
  }

  s=LC_HIPersonalData_GetAddrStreet(pData);
  fprintf(f,"Strasse:%s%s", s?s:"", CRLF);
  s=LC_HIPersonalData_GetAddrState(pData);
  fprintf(f,"Laendercode:%s%s", s?s:"", CRLF);
  s=LC_HIPersonalData_GetAddrZipCode(pData);
  fprintf(f,"PLZ:%s%s", s?s:"", CRLF);
  s=LC_HIPersonalData_GetAddrCity(pData);
  fprintf(f,"Ort:%s%s", s?s:"", CRLF);

  cti=LC_HIInsuranceData_GetCoverBegin(iData);
  if (cti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Time_toUtcString(cti, "DDMMYY", tbuf);
    fprintf(f,"gueltig-seit:%s%s",
	    GWEN_Buffer_GetStart(tbuf), CRLF);
    GWEN_Buffer_free(tbuf);
  }

  cti=LC_HIInsuranceData_GetCoverEnd(iData);
  if (cti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Time_toUtcString(cti, "MMYY", tbuf);
    fprintf(f,"gueltig-bis:%s%s",
	    GWEN_Buffer_GetStart(tbuf), CRLF);
    GWEN_Buffer_free(tbuf);
  }

  fprintf(f,"Pruefsumme-gueltig:ja%s", CRLF);
  fprintf(f,"Kommentar:derzeit keiner%s", CRLF);

  if (fname) {
    if (fclose(f)) {
      DBG_ERROR(0, "Could not close file \"%s\", reason: \n %s",
		fname,
		strerror(errno));
      return -1;
    }
  }

  return 0;
}



int handleKvkCard(LC_CARD *card, GWEN_DB_NODE *dbArgs) {
  int v;
  const char *fname;
  int dobeep;
  int dosMode;
  LC_CLIENT_RESULT res;
  LC_HI_PERSONAL_DATA *pdata=NULL;
  LC_HI_INSURANCE_DATA *idata=NULL;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  dobeep=GWEN_DB_GetIntValue(dbArgs, "beep", 0, 0);
  dosMode=GWEN_DB_GetIntValue(dbArgs, "dosMode", 0, 0);

  res=LC_KvkCard_ReadCardData(card, &pdata, &idata);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "LC_KvkCard_ReadCardData");
    if (dobeep)
      errorBeep();
    return RETURNVALUE_WORK;
  }

  /* open file */
  if (v>0)
    fprintf(stderr, "Writing data to file\n");
  fname=GWEN_DB_GetCharValue(dbArgs, "fileName", 0, 0);
  if (storeCardData(fname, card, dosMode,
                    "Krankenversichertenkarte",
		    LC_KvkCard_GetCardNumber(card),
		    pdata, idata)) {
    fprintf(stderr, "ERROR: Could not write to file.\n");
    if (dobeep)
      errorBeep();
    return RETURNVALUE_WORK;
  }

  if (v>1)
    fprintf(stderr, "Data written.\n");

  if (dobeep)
    okBeep();

  return 0;
}



int handleEgkCard(LC_CARD *card, GWEN_DB_NODE *dbArgs) {
  int v;
  const char *fname;
  int dobeep;
  int dosMode;
  LC_CLIENT_RESULT res;
  LC_HI_PERSONAL_DATA *pdata=NULL;
  LC_HI_INSURANCE_DATA *idata=NULL;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  dobeep=GWEN_DB_GetIntValue(dbArgs, "beep", 0, 0);
  dosMode=GWEN_DB_GetIntValue(dbArgs, "dosMode", 0, 0);

  res=LC_EgkCard_ReadPersonalData(card, &pdata);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "LC_EgkCard_ReadPersonalData");
    if (dobeep)
      errorBeep();
    return RETURNVALUE_WORK;
  }

  if (pdata==NULL)
    pdata=LC_HIPersonalData_new();

  res=LC_EgkCard_ReadInsuranceData(card, &idata);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "LC_EgkCard_ReadInsuranceData");
    if (dobeep)
      errorBeep();
    LC_HIPersonalData_free(pdata);
    return RETURNVALUE_WORK;
  }

  if (idata==NULL)
    idata=LC_HIInsuranceData_new();

  if (v>0)
    fprintf(stderr, "Writing data to file\n");
  fname=GWEN_DB_GetCharValue(dbArgs, "fileName", 0, 0);
  if (storeCardData(fname, card, dosMode,
                    "Elektronische Gesundheitskarte",
		    NULL,
		    pdata, idata)) {
    fprintf(stderr, "ERROR: Could not write to file.\n");
    if (dobeep)
      errorBeep();
    LC_HIInsuranceData_free(idata);
    LC_HIPersonalData_free(pdata);
    return RETURNVALUE_WORK;
  }

  if (v>1)
    fprintf(stderr, "Data written.\n");

  if (dobeep)
    okBeep();

  LC_HIInsuranceData_free(idata);
  LC_HIPersonalData_free(pdata);
  return 0;
}



int handleCard(LC_CARD *card, GWEN_DB_NODE *dbArgs) {
  int rv;
  LC_CLIENT_RESULT res;
  int v;
  int dobeep;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  dobeep=GWEN_DB_GetIntValue(dbArgs, "beep", 0, 0);

  /* try to open as KVK */
  rv=LC_KVKCard_ExtendCard(card);
  if (rv) {
    if (dobeep)
      errorBeep();
    return rv;
  }

  if (v>0)
    fprintf(stderr, "Opening card as KVK card.\n");
  res=LC_Card_Open(card);
  if (res==LC_Client_ResultOk) {
    if (v>0)
      fprintf(stderr, "Card is a KVK card, handling it.\n");
    rv=handleKvkCard(card, dbArgs);

    /* close card */
    if (v>0)
      fprintf(stderr, "Closing card.\n");
    res=LC_Card_Close(card);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "CardClose");
      return RETURNVALUE_WORK;
    }
    if (v>1)
      fprintf(stderr, "Card closed.\n");

    return rv;
  }
  else {
    if (v>0)
      fprintf(stderr, "Card is not a KVK card.\n");
  }
  LC_KVKCard_UnextendCard(card);

  /* try to open as EGK */
  rv=LC_EgkCard_ExtendCard(card);
  if (rv) {
    if (dobeep)
      errorBeep();
    return rv;
  }

  if (v>0)
    fprintf(stderr, "Opening card as EGK card.\n");
  res=LC_Card_Open(card);
  if (res==LC_Client_ResultOk) {
    if (v>0)
      fprintf(stderr, "Card is a EGK card, handling it.\n");
    rv=handleEgkCard(card, dbArgs);

    /* close card */
    if (v>0)
      fprintf(stderr, "Closing card.\n");
    res=LC_Card_Close(card);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "CardClose");
      return RETURNVALUE_WORK;
    }

    if (v>1)
      fprintf(stderr, "Card closed.\n");
    return rv;
  }
  else {
    if (v>0)
      fprintf(stderr, "Card is not a EGK card.\n");
    showError(card, res, "CardOpen");
  }

  /* not supported */
  if (dobeep)
    errorBeep();
  return RETURNVALUE_CARD_NOT_SUPP;
}



int kvkRead(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CARD *card=0;
  LC_CLIENT_RESULT res;
  int v;
  int i;
  int dobeep;
  int rv=0;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  dobeep=GWEN_DB_GetIntValue(dbArgs, "beep", 0, 0);

  if (v>1)
    fprintf(stderr, "Connecting to server.\n");
  res=LC_Client_Start(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "StartWait");
    if (dobeep)
      errorBeep();
    return RETURNVALUE_WORK;
  }
  if (v>1)
    fprintf(stderr, "Connected.\n");

  for (i=0;;i++) {
    if (v>0)
      fprintf(stderr, "Waiting for card...\n");
    res=LC_Client_GetNextCard(cl, &card, 20);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "GetNextCard");
      if (dobeep)
	errorBeep();
      return RETURNVALUE_WORK;
    }
    if (v>0)
      fprintf(stderr, "Found a card.\n");

    rv=handleCard(card, dbArgs);

    if (v>0)
      fprintf(stderr, "Releasing card.\n");
    res=LC_Client_ReleaseCard(cl, card);
    if (res!=LC_Client_ResultOk) {
      showError(card, res, "ReleaseCard");
      if (dobeep)
	errorBeep();
      LC_Card_free(card);
      return RETURNVALUE_WORK;
    }
    LC_Card_free(card);

#if 1
    break;
#else
    if (rv!=RETURNVALUE_CARD_NOT_SUPP)
      break;

    if (i>15) {
      fprintf(stderr, "ERROR: No card found.\n");
      if (dobeep)
	errorBeep();
      return RETURNVALUE_WORK;
    }
#endif
  } /* for */

  /* stop waiting */
  if (v>1)
    fprintf(stderr, "Telling the server that we need no more cards.\n");
  res=LC_Client_Stop(cl);
  if (res!=LC_Client_ResultOk) {
    showError(card, res, "Stop");
    if (dobeep)
      errorBeep();
    return RETURNVALUE_WORK;
  }

  /* finished */
  if (v>1)
    fprintf(stderr, "Finished.\n");
  return rv;
}



