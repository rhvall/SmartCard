


void LC_Client__SampleXmlFiles(const char *where,
                               GWEN_STRINGLIST *sl) {
  GWEN_BUFFER *buf;
  GWEN_DIRECTORY *d;
  unsigned int dpos;

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  d=GWEN_Directory_new();
  GWEN_Buffer_AppendString(buf, where);
  DBG_DEBUG(LC_LOGDOMAIN, "Sampling files in \"%s\"",
            GWEN_Buffer_GetStart(buf));
  dpos=GWEN_Buffer_GetPos(buf);
  if (!GWEN_Directory_Open(d, GWEN_Buffer_GetStart(buf))) {
    char buffer[256];
    unsigned int i;
    GWEN_STRINGLIST *slDirs;
    GWEN_STRINGLISTENTRY *se;

    slDirs=GWEN_StringList_new();
    while (!GWEN_Directory_Read(d, buffer, sizeof(buffer))){
      if (strcmp(buffer, ".")!=0 &&
          strcmp(buffer, "..")!=0) {
        struct stat st;

        GWEN_Buffer_Crop(buf, 0, dpos);
        GWEN_Buffer_SetPos(buf, dpos);
        GWEN_Buffer_AppendByte(buf, '/');
        GWEN_Buffer_AppendString(buf, buffer);
        DBG_DEBUG(LC_LOGDOMAIN, "Checking file \"%s\"",
                  GWEN_Buffer_GetStart(buf));
  
        if (stat(GWEN_Buffer_GetStart(buf), &st)) {
          DBG_ERROR(LC_LOGDOMAIN, "stat(%s): %s",
                    GWEN_Buffer_GetStart(buf),
                    strerror(errno));
        }
        else {
          if (S_ISDIR(st.st_mode)) {
            /* it is a folder, dive into it later */
            GWEN_StringList_AppendString(slDirs,
                                         GWEN_Buffer_GetStart(buf),
                                         0, 1);
          }
          else {
            i=strlen(buffer);
            if (i>3) {
              if (strcasecmp(buffer+i-4, ".xml")==0) {
                DBG_INFO(LC_LOGDOMAIN, "Adding xml file \"%s\"",
                         GWEN_Buffer_GetStart(buf));
                GWEN_StringList_AppendString(sl,
                                             GWEN_Buffer_GetStart(buf),
                                             0, 1);
              } /* if name ends in ".xml" */
            } /* if name longer than 3 chars */
          } /* if it is not a folder */
        } /* if stat succeeded */
      } /* if not a special file/folder */
    } /* while */
    GWEN_Directory_Close(d);

    /* now read subfolders */
    se=GWEN_StringList_FirstEntry(slDirs);
    while(se) {
      LC_Client__SampleXmlFiles(GWEN_StringListEntry_Data(se), sl);
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(slDirs);

  } /* if open succeeded */
  else {
    DBG_DEBUG(LC_LOGDOMAIN, "Could not open dir \"%s\"",
	      GWEN_Buffer_GetStart(buf));
  }
  GWEN_Directory_free(d);
  GWEN_Buffer_free(buf);
}



int LC_Client_MergeXMLDefs(GWEN_XMLNODE *destNode,
                           GWEN_XMLNODE *node) {
  GWEN_XMLNODE *nsrc, *ndst;

  assert(node);

  nsrc=GWEN_XMLNode_GetChild(node);
  while(nsrc) {
    /* merge 1st level */
    if (GWEN_XMLNode_GetType(nsrc)==GWEN_XMLNodeTypeTag) {
      ndst=GWEN_XMLNode_FindFirstTag(destNode,
                                     GWEN_XMLNode_GetData(nsrc),
                                     "name",
                                     GWEN_XMLNode_GetProperty(nsrc,
                                                              "name",
                                                              ""));
      if (ndst) {
        GWEN_XMLNODE *nsrc2, *ndst2;

        /* merge 2nd level */
        DBG_VERBOUS(LC_LOGDOMAIN, "Merging tags from \"%s\" into \"%s\"",
                    GWEN_XMLNode_GetData(nsrc),
                    GWEN_XMLNode_GetData(ndst));
        nsrc2=GWEN_XMLNode_GetChild(nsrc);
        while(nsrc2) {
          if (GWEN_XMLNode_GetType(nsrc2)==GWEN_XMLNodeTypeTag) {
            ndst2=GWEN_XMLNode_FindNode(ndst,
                                        GWEN_XMLNodeTypeTag,
                                        GWEN_XMLNode_GetData(nsrc2));
            if (ndst2) {
              GWEN_XMLNODE *n;

              DBG_VERBOUS(LC_LOGDOMAIN,
                          "Level2: Merging tags from "
                          "\"%s\" into \"%s\"",
                          GWEN_XMLNode_GetData(nsrc2),
                          GWEN_XMLNode_GetData(ndst2));
              /* node found, copy branch */
              n=GWEN_XMLNode_GetChild(nsrc2);
              while (n) {
                GWEN_XMLNODE *newNode;

                DBG_VERBOUS(LC_LOGDOMAIN, "Adding node \"%s\"",
                            GWEN_XMLNode_GetData(n));
                newNode=GWEN_XMLNode_dup(n);
                GWEN_XMLNode_AddChild(ndst2, newNode);
                n=GWEN_XMLNode_Next(n);
              } /* while n */
            }
            else {
              GWEN_XMLNODE *newNode;

              DBG_VERBOUS(LC_LOGDOMAIN, "Adding branch \"%s\"",
                          GWEN_XMLNode_GetData(nsrc2));
              newNode=GWEN_XMLNode_dup(nsrc2);
              GWEN_XMLNode_AddChild(ndst, newNode);
            }
          } /* if TAG */
          nsrc2=GWEN_XMLNode_Next(nsrc2);
        } /* while there are 2nd level source tags */
      }
      else {
	GWEN_XMLNODE *newNode;

        DBG_VERBOUS(LC_LOGDOMAIN, "Adding branch \"%s\"",
                    GWEN_XMLNode_GetData(nsrc));
        newNode=GWEN_XMLNode_dup(nsrc);
        GWEN_XMLNode_AddChild(destNode, newNode);
      }
    } /* if TAG */
    nsrc=GWEN_XMLNode_Next(nsrc);
  } /* while */

  return 0;
}



int LC_Client_ReadXmlFiles(GWEN_XMLNODE *root,
			   const char *basedir,
                           const char *tPlural,
                           const char *tSingular) {
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *se;
  GWEN_BUFFER *buf;
  int filesLoaded=0;

  /* prepare path */
  sl=GWEN_StringList_new();
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf, basedir);
  GWEN_Buffer_AppendString(buf, DIRSEP);
  GWEN_Buffer_AppendString(buf, tPlural);

  DBG_DEBUG(0, "Reading XML file (%s) from here: %s",
            tPlural,
            GWEN_Buffer_GetStart(buf));

  /* sample all XML files from that path */
  LC_Client__SampleXmlFiles(GWEN_Buffer_GetStart(buf), sl);

  /* load all files from the list */
  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    GWEN_XMLNODE *n;
  
    n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, tSingular);
    if (GWEN_XML_ReadFile(n,
                          GWEN_StringListEntry_Data(se),
                          GWEN_XML_FLAGS_DEFAULT)) {
      DBG_ERROR(LC_LOGDOMAIN, "Could not read XML file \"%s\"",
                GWEN_StringListEntry_Data(se));
    }
    else {
      GWEN_XMLNODE *nn;
  
      nn=GWEN_XMLNode_FindNode(n, GWEN_XMLNodeTypeTag, tPlural);
      if (!nn) {
        DBG_WARN(LC_LOGDOMAIN, "File \"%s\" does not contain <%s>",
                 GWEN_StringListEntry_Data(se), tPlural);
      }
      else {
        if (LC_Client_MergeXMLDefs(root, nn)) {
          DBG_ERROR(LC_LOGDOMAIN, "Could not merge file \"%s\"",
                    GWEN_StringListEntry_Data(se));
        }
        else {
          filesLoaded++;
        }
      }
    }
    GWEN_XMLNode_free(n);

    se=GWEN_StringListEntry_Next(se);
  }

  /* cleanup */
  GWEN_StringList_free(sl);

  /* done */
  if (filesLoaded==0) {
    DBG_ERROR(LC_LOGDOMAIN, "No %s files loaded", tSingular);
    return -1;
  }

  return 0;
}



GWEN_XMLNODE *LC_Client_GetAppNode(LC_CLIENT *cl, const char *appName) {
  GWEN_XMLNODE *node;

  assert(cl);
  node=GWEN_XMLNode_FindFirstTag(cl->appNodes,
                                 "app",
                                 "name",
                                 appName);
  if (node==0) {
    DBG_ERROR(LC_LOGDOMAIN, "App \"%s\" not found", appName);
    return 0;
  }

  return node;
}



GWEN_XMLNODE *LC_Client_GetCardNode(LC_CLIENT *cl, const char *cardName) {
  GWEN_XMLNODE *node;

  assert(cl);
  node=GWEN_XMLNode_FindFirstTag(cl->cardNodes,
                                 "card",
                                 "name",
                                 cardName);
  if (node==0) {
    DBG_ERROR(LC_LOGDOMAIN, "Card \"%s\" not found", cardName);
    return 0;
  }

  return node;
}













