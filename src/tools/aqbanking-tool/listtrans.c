/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/bio_file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



int listTrans(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  const char *outFile;
  const char *exporterName;
  const char *profileName;
  const char *profileFile;
  AB_IMEXPORTER *exporter;
  GWEN_DB_NODE *dbProfiles;
  GWEN_DB_NODE *dbProfile;
  AB_IMEXPORTER_CONTEXT *ctx=0;
  AB_IMEXPORTER_CONTEXT *nctx=0;
  AB_IMEXPORTER_ACCOUNTINFO *iea=0;
  const char *bankId;
  const char *accountId;
  const char *bankName;
  const char *accountName;
  int fd;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "bankId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "b",                          /* short option */
    "bank",                       /* long option */
    "Specify the bank code",      /* short description */
    "Specify the bank code"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "accountId",                  /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "a",                          /* short option */
    "account",                    /* long option */
    "Specify the account number",     /* short description */
    "Specify the account number"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "bankName",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "N",                          /* short option */
    "bankname",                   /* long option */
    "Specify the bank name",      /* short description */
    "Specify the bank name"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "accountName",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "n",                          /* short option */
    "accountname",                    /* long option */
    "Specify the account name",     /* short description */
    "Specify the account name"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "ctxFile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "ctxfile",                    /* long option */
    "Specify the file to store the context in",   /* short description */
    "Specify the file to store the context in"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "outFile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "o",                          /* short option */
    "outfile",                    /* long option */
    "Specify the file to store the data in",   /* short description */
    "Specify the file to store the data in"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "exporterName",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "exporter",                    /* long option */
    "Specify the exporter to use",   /* short description */
    "Specify the exporter to use"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "profileName",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "profile",                    /* long option */
    "Specify the export profile to use",   /* short description */
    "Specify the export profile to use"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "profileFile",                /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "profile-file",               /* long option */
    "Specify the file to load the export profile from",/* short description */
    "Specify the file to load the export profile from" /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  exporterName=GWEN_DB_GetCharValue(db, "exporterName", 0, "csv");
  profileName=GWEN_DB_GetCharValue(db, "profileName", 0, "default");
  profileFile=GWEN_DB_GetCharValue(db, "profileFile", 0, 0);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  bankName=GWEN_DB_GetCharValue(db, "bankName", 0, 0);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0, 0);
  accountName=GWEN_DB_GetCharValue(db, "accountName", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error on init (%d)", rv);
    return 2;
  }

  /* get export module */
  exporter=AB_Banking_GetImExporter(ab, exporterName);
  if (!exporter) {
    DBG_ERROR(AQT_LOGDOMAIN, "Export module \"%s\" not found", exporterName);
    return 3;
  }

  /* get profiles */
  if (profileFile) {
    dbProfiles=GWEN_DB_Group_new("profiles");
    if (GWEN_DB_ReadFile(dbProfiles, profileFile,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(AQT_LOGDOMAIN, "Error reading profiles from \"%s\"",
                profileFile);
      return 3;
    }
  }
  else {
    dbProfiles=AB_Banking_GetImExporterProfiles(ab, exporterName);
  }

  /* select profile */
  dbProfile=GWEN_DB_GetFirstGroup(dbProfiles);
  while(dbProfile) {
    const char *name;

    name=GWEN_DB_GetCharValue(dbProfile, "name", 0, 0);
    assert(name);
    if (strcasecmp(name, profileName)==0)
      break;
    dbProfile=GWEN_DB_GetNextGroup(dbProfile);
  }
  if (!dbProfile) {
    DBG_ERROR(AQT_LOGDOMAIN, "Profile \"%s\" for exporter \"%s\" not found",
              profileName, exporterName);
    return 3;
  }

  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  if (ctxFile==0)
    fd=fileno(stdin);
  else
    fd=open(ctxFile, O_RDONLY);
  if (fd<0) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error selecting output file: %s",
              strerror(errno));
    return 4;
  }
  else {
    GWEN_BUFFEREDIO *bio;
    GWEN_ERRORCODE err;
    GWEN_DB_NODE *dbCtx;

    dbCtx=GWEN_DB_Group_new("context");
    bio=GWEN_BufferedIO_File_new(fd);
    if (!ctxFile)
      GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
    if (GWEN_DB_ReadFromStream(dbCtx, bio,
                               GWEN_DB_FLAGS_DEFAULT |
                               GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(AQT_LOGDOMAIN, "Error reading context");
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(AQT_LOGDOMAIN, err);
      GWEN_DB_Group_free(dbCtx);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 4;
    }
    GWEN_BufferedIO_free(bio);

    ctx=AB_ImExporterContext_fromDb(dbCtx);
    if (!ctx) {
      DBG_ERROR(AQT_LOGDOMAIN, "No context in input data");
      GWEN_DB_Group_free(dbCtx);
      return 4;
    }
    GWEN_DB_Group_free(dbCtx);
  }

  nctx=AB_ImExporterContext_new();
  iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(iea) {
    int matches=1;
    AB_ACCOUNT *a;
    const char *s;
    const char *s1, *s2;

    s1=AB_ImExporterAccountInfo_GetBankCode(iea);
    s2=AB_ImExporterAccountInfo_GetAccountNumber(iea);
    a=AB_Banking_GetAccountByCodeAndNumber(ab, s1, s2);
    if (!a) {
      DBG_ERROR(AQT_LOGDOMAIN, "Account %s/%s not found, ignoring",
                s1, s2);
      matches=0;
    }
    else {
      if (matches && bankId) {
        s=AB_Account_GetBankCode(a);
        if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankId, 0))
          matches=0;
      }

      if (matches && bankName) {
        s=AB_Account_GetBankName(a);
        if (!s || !*s)
          s=AB_ImExporterAccountInfo_GetBankName(iea);
        if (!s || !*s || -1==GWEN_Text_ComparePattern(s, bankName, 0))
          matches=0;
      }

      if (matches && accountId) {
        s=AB_Account_GetAccountNumber(a);
        if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountId, 0))
          matches=0;
      }
      if (matches && accountName) {
        s=AB_Account_GetAccountName(a);
        if (!s || !*s)
          s=AB_ImExporterAccountInfo_GetAccountName(iea);
        if (!s || !*s || -1==GWEN_Text_ComparePattern(s, accountName, 0))
          matches=0;
      }
    }
    if (matches) {
      AB_IMEXPORTER_ACCOUNTINFO *nai;

      nai=AB_ImExporterAccountInfo_dup(iea);
      AB_ImExporterContext_AddAccountInfo(nctx, nai);
    } /* if matches */
    iea=AB_ImExporterContext_GetNextAccountInfo(ctx);
  } /* while */

  /* export new context */
  outFile=GWEN_DB_GetCharValue(db, "outFile", 0, 0);
  if (outFile==0)
    fd=fileno(stdout);
  else
    fd=open(outFile, O_RDWR | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR
#ifdef S_IRGRP
            | S_IRGRP
#endif
#ifdef S_IWGRP
            | S_IWGRP
#endif
           );
  if (fd<0) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error selecting output file: %s",
              strerror(errno));
    AB_ImExporterContext_free(nctx);
    GWEN_DB_Group_free(dbProfiles);
    return 4;
  }
  else {
    GWEN_BUFFEREDIO *bio;
    GWEN_ERRORCODE err;

    bio=GWEN_BufferedIO_File_new(fd);
    if (!outFile)
      GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
    rv=AB_ImExporter_Export(exporter,
                            nctx,
                            bio,
                            dbProfile);
    if (rv) {
      DBG_ERROR(AQT_LOGDOMAIN, "Error exporting data (%d)", rv);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      AB_ImExporterContext_free(nctx);
      GWEN_DB_Group_free(dbProfiles);
      return 4;
    }
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(AQT_LOGDOMAIN, err);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      AB_ImExporterContext_free(nctx);
      GWEN_DB_Group_free(dbProfiles);
      return 4;
    }
    GWEN_BufferedIO_free(bio);
  }
  AB_ImExporterContext_free(nctx);
  GWEN_DB_Group_free(dbProfiles);

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}






