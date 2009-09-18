/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: getsysid.c 1288 2007-08-11 16:53:57Z martin $
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

#include <aqhbci/user.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int setMaxTransfers(AB_BANKING *ab,
		    GWEN_DB_NODE *dbArgs,
		    int argc,
		    char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  AB_USER_LIST2 *ul;
  AB_USER *u=0;
  int rv;
  const char *bankId;
  const char *userId;
  const char *customerId;
  int maxTransfers;
  int maxDebitNotes;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
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
    GWEN_ArgsType_Char,           /* type */
    "userId",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "u",                          /* short option */
    "user",                       /* long option */
    "Specify the user id (Benutzerkennung)",    /* short description */
    "Specify the user id (Benutzerkennung)"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "customerId",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "customer",                   /* long option */
    "Specify the customer id (Kundennummer)",    /* short description */
    "Specify the customer id (Kundennummer)"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,            /* type */
    "maxTransfers",               /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "t",                          /* short option */
    "transfers",                  /* long option */
    "Specify the maximum number of transfers per job",    /* short description */
    "Specify the maximum number of transfers per job"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,            /* type */
    "maxDebitNotes",              /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "d",                          /* short option */
    "debitnotes",                 /* long option */
    "Specify the maximum number of debit notes per job",    /* short description */
    "Specify the maximum number of debit notes per job"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,            /* type */
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
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab, 0);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, "*");
  userId=GWEN_DB_GetCharValue(db, "userId", 0, "*");
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, "*");

  maxTransfers=GWEN_DB_GetIntValue(db, "maxTransfers", 0, -1);
  maxDebitNotes=GWEN_DB_GetIntValue(db, "maxDebitNotes", 0, -1);

  ul=AB_Banking_FindUsers(ab, AH_PROVIDER_NAME, "de",
                          bankId, userId, customerId);
  if (ul) {
    if (AB_User_List2_GetSize(ul)!=1) {
      DBG_ERROR(0, "Ambiguous customer specification");
      return 3;
    }
    else {
      AB_USER_LIST2_ITERATOR *uit;

      uit=AB_User_List2_First(ul);
      assert(uit);
      u=AB_User_List2Iterator_Data(uit);
      AB_User_List2Iterator_free(uit);
    }
    AB_User_List2_free(ul);
  }
  if (!u) {
    DBG_ERROR(0, "No matching customer");
    return 3;
  }
  else {
    /* lock user */
    rv=AB_Banking_BeginExclUseUser(ab, u, 0);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR: Could not lock user, maybe it is used in another application? (%d)\n",
	      rv);
      AB_Banking_OnlineFini(ab, 0);
      AB_Banking_Fini(ab);
      return 4;
    }

    /* modify user */
    if (maxTransfers>0) {
      fprintf(stderr, "Setting maximum number of transfers per job to %d\n",
	      maxTransfers);
      AH_User_SetMaxTransfersPerJob(u, maxTransfers);
    }

    if (maxDebitNotes>0) {
      fprintf(stderr, "Setting maximum number of debit notes per job to %d\n",
	      maxDebitNotes);
      AH_User_SetMaxDebitNotesPerJob(u, maxDebitNotes);
    }

    /* unlock user */
    rv=AB_Banking_EndExclUseUser(ab, u, 0, 0);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR: Could not unlock user (%d)\n",
	      rv);
      AB_Banking_OnlineFini(ab, 0);
      AB_Banking_Fini(ab);
      return 4;
    }
  }

  rv=AB_Banking_OnlineFini(ab, 0);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}



