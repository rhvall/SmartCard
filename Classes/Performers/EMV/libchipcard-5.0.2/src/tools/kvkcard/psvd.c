

int psvd(LC_CLIENT *cl, GWEN_DB_NODE *dbArgs){
  LC_CLIENT_RESULT res;
  int v;
  int rv;
  const char *outFile;
  const char *inFile;
  LC_HI_INSURANCE_DATA *data=NULL;
  GWEN_XMLNODE *root;
  GWEN_DB_NODE *db;

  v=GWEN_DB_GetIntValue(dbArgs, "verbosity", 0, 0);
  outFile=GWEN_DB_GetCharValue(dbArgs, "filename", 0, 0);
  inFile=GWEN_DB_GetCharValue(dbArgs, "infilename", 0, 0);

  if (inFile==NULL) {
    fprintf(stderr, "This command needs \"--infilename\"\n");
    return RETURNVALUE_PARAM;
  }

  if (outFile==NULL) {
    fprintf(stderr, "This command needs \"--filename\"\n");
    return RETURNVALUE_PARAM;
  }

  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(root,
                       inFile,
		       GWEN_XML_FLAGS_HANDLE_HEADERS |
		       GWEN_XML_FLAGS_HANDLE_NAMESPACES);
  if (rv<0) {
    GWEN_XMLNode_free(root);
    DBG_ERROR(0, "Error: %d", rv);
    return RETURNVALUE_WORK;
  }
  GWEN_XMLNode_StripNamespaces(root);

  res=LC_EgkCard_ParseInsuranceData(root, &data);
  if (res!=LC_Client_ResultOk) {
    GWEN_XMLNode_free(root);
    DBG_ERROR(0, "Error: %d", res);
    return RETURNVALUE_WORK;
  }

  db=GWEN_DB_Group_new("vd");
  rv=LC_HIInsuranceData_toDb(data, db);
  if (rv<0) {
    GWEN_DB_Group_free(db);
    GWEN_XMLNode_free(root);
    DBG_ERROR(0, "Error: %d", rv);
    return RETURNVALUE_WORK;
  }


  rv=GWEN_DB_WriteFile(db, outFile, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    GWEN_DB_Group_free(db);
    GWEN_XMLNode_free(root);
    DBG_ERROR(0, "Error: %d", rv);
    return RETURNVALUE_WORK;
  }

  GWEN_DB_Group_free(db);
  GWEN_XMLNode_free(root);

  /* finished */
  if (v>1)
    fprintf(stderr, "Finished.\n");
  return rv;
}



