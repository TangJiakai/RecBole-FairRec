/* Copyright 2020, Gurobi Optimization, LLC */

/* This example reads a MIP model from a file, solves it in batch mode,
 * and prints the JSON solution string. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#if defined (WIN32) || defined (WIN64)
#include <Windows.h>
#define sleep(n) Sleep(1000*n)
#else
#include <unistd.h>
#endif
#include "gurobi_c.h"

/* setup gurobi environment */
int setupbatchconnection(GRBenv **envP)
{
  int     error = 0;
  GRBenv *env   = NULL;

  /* setup a batch environment */
  error = GRBemptyenv(envP);
  if (error) goto QUIT;
  env = *envP;
  error = GRBsetintparam(env, "CSBatchMode", 1);
  if (error) goto QUIT;
  error = GRBsetstrparam(env, "LogFile", "batchmode.log");
  if (error) goto QUIT;
  error = GRBsetstrparam(env, "CSManager", "http://localhost:61080");
  if (error) goto QUIT;
  error = GRBsetstrparam(env, "UserName", "gurobi");
  if (error) goto QUIT;
  error = GRBsetstrparam(env, "ServerPassword", "pass");
  if (error) goto QUIT;
  error = GRBstartenv(env);
  if (error) goto QUIT;

QUIT:

  if (error) {
    printf("Failed to setup environment, error code %d\n", error);
  } else {
    printf("Successfully created environment\n");
  }
  return error;
}

/* display batch-error code if any */
void batcherrorinfo(GRBbatch *batch)
{
  int error = 0;
  int errorCode;
  char *errorMsg;
  char *BatchID;

  if (!batch) goto QUIT;

  /* query the last error code */
  error = GRBgetbatchintattr(batch, "BatchErrorCode", &errorCode);
  if (error || !errorCode) goto QUIT;

  /* query the last error message */
  error = GRBgetbatchstrattr(batch, "BatchErrorMessage", &errorMsg);
  if (error) goto QUIT;

  error = GRBgetbatchstrattr(batch, "BatchID", &BatchID);
  if (error) goto QUIT;

  printf("Batch ID %s Error Code %d (%s)\n", BatchID, errorCode, errorMsg);

QUIT:

  return;
}

/* create a batch request for given problem file */
int newbatchrequest(const char *filename,
		    char       *BatchID)
{
  int       error = 0;
  GRBenv   *env   = NULL;
  GRBenv   *menv  = NULL;
  GRBmodel *model = NULL;
  char tag[128];
  int cols, j;

  /* setup a batch connection */
  error = setupbatchconnection(&env);
  if (error) goto QUIT;

  /* read a model */
  error = GRBreadmodel(env, filename, &model);
  if (error) goto QUIT;

  /* set some params */
  menv = GRBgetenv(model);
  error = GRBsetdblparam(menv, "MIPGap", 0.01);
  if (error) goto QUIT;
  /* for extra detailed information on JSON solution string */
  error = GRBsetintparam(menv, "JSONSolDetail", 1);
  if (error) goto QUIT;

  /* setup some tags, we need tags to be able to query results later on */
  error = GRBgetintattr(model, "NumVars", &cols);
  if (error) goto QUIT;

  if (cols > 10) cols = 10;
  for (j = 0; j < cols; j++) {
    sprintf(tag, "MyUniqueVariableID%d",j);
    error = GRBsetstrattrelement(model, "VTag", j, tag);
  }

  /* submit batch request to the Manager */
  error = GRBoptimizebatch(model, BatchID);
  if (error) goto QUIT;

QUIT:

  if (error) {
    printf("Failed to submit a new batch request, error code %d\n", error);
  } else {
    printf("Successfully submitted new batch request %s\n", BatchID);
  }
  GRBfreemodel(model);
  GRBfreeenv(env);
  return error;
}

/* wait for final bstatus */
int waitforfinalstatus(const char *BatchID)
{
  int       error = 0;
  GRBenv   *env   = NULL;
  GRBbatch *batch = NULL;
  time_t start, current;
  int bstatus;

  /* setup a batch connection */
  error = setupbatchconnection(&env);
  if (error) goto QUIT;

  /* create batch-object */
  error = GRBgetbatch(env, BatchID, &batch);
  if (error) goto QUIT;

  /* query bstatus, and wait for completed */
  error = GRBgetbatchintattr(batch, "BatchStatus",  &bstatus);
  if (error) goto QUIT;
  start = time(NULL);

  while (bstatus == GRB_BATCH_SUBMITTED) {

    /* abort if taking too long */
    current = time(NULL);
    if (current - start >= 3600) {
      /* request to abort the batch */
      error = GRBabortbatch(batch);
      if (error) goto QUIT;
    }

    /* do not bombard the server */
    sleep(1u);

    /* update local attributes */
    error = GRBupdatebatch(batch);
    if (error) goto QUIT;

    /* query bstatus */
    error = GRBgetbatchintattr(batch, "BatchStatus",  &bstatus);
    if (error) goto QUIT;

    /* deal with failed bstatus */
    if (bstatus == GRB_BATCH_FAILED) {
      /* retry the batch request */
      error = GRBretrybatch(batch);
      if (error) goto QUIT;
      bstatus = GRB_BATCH_SUBMITTED;
    }
  }


QUIT:

  if (error) {
    printf("Failed to wait for final bstatus, error code %d\n", error);
  } else {
    printf("Final Batch Status %d\n", bstatus);
  }
  batcherrorinfo(batch);
  /* release local resources */
  GRBfreebatch(batch);
  GRBfreeenv(env);
  return error;
}

/* final report on batch request */
int finalreport(const char *BatchID)
{
  int       error   = 0;
  GRBenv   *env     = NULL;
  GRBbatch *batch   = NULL;
  char     *jsonsol = NULL;
  int bstatus;

  /* setup a batch connection */
  error = setupbatchconnection(&env);
  if (error) goto QUIT;

  /* create batch object */
  error = GRBgetbatch(env, BatchID, &batch);
  if (error) goto QUIT;

  /* query bstatus, and wait for completed */
  error = GRBgetbatchintattr(batch, "BatchStatus",  &bstatus);
  if (error) goto QUIT;

  /* display depending on batch bstatus */
  switch (bstatus) {
    case GRB_BATCH_CREATED:
      printf("Batch is 'CREATED'\n");
      printf("maybe batch-creation process was killed?\n");
      break;
    case GRB_BATCH_SUBMITTED:
      printf("Batch is 'SUBMITTED'\n");
      printf("Some other user re-submitted this Batch object?\n");
      break;
    case GRB_BATCH_ABORTED:
      printf("Batch is 'ABORTED'\n");
      break;
    case GRB_BATCH_FAILED:
      printf("Batch is 'FAILED'\n");
      break;
    case GRB_BATCH_COMPLETED:

      /* print JSON solution into string */
      error = GRBgetbatchjsonsolution(batch, &jsonsol);
      if (error) goto QUIT;
      printf("JSON solution: %s\n", jsonsol);

      /* save solution into a file */
      error = GRBwritebatchjsonsolution(batch, "batch-sol.json.gz");
      if (error) goto QUIT;
      break;
    default:
      printf("This should not happen, probably points to a"
             " user-memory corruption problem\n");
      exit(EXIT_FAILURE);
      break;
  }

QUIT:

  if (error) {
    printf("Failed to perform final report, error code %d\n", error);
  } else {
    printf("Reporting done\n");
  }
  batcherrorinfo(batch);

  if (jsonsol)
    GRBfree(jsonsol);

  GRBfreebatch(batch);
  GRBfreeenv(env);
  return error;
}

/* remove batch ID from manager */
int discardbatch(const char *BatchID)
{
  int       error   = 0;
  GRBenv   *env     = NULL;
  GRBbatch *batch   = NULL;

  /* setup a batch connection */
  error = setupbatchconnection(&env);
  if (error) goto QUIT;

  /* create batch object */
  error = GRBgetbatch(env, BatchID, &batch);
  if (error) goto QUIT;

  /* discard the batch object in the manager */
  error = GRBdiscardbatch(batch);
  if (error) goto QUIT;

QUIT:

  batcherrorinfo(batch);
  GRBfreebatch(batch);
  GRBfreeenv(env);
  return error;
}

int
main(int    argc,
     char **argv)
{
  int error = 0;
  char BatchID[GRB_MAX_STRLEN+1];

  /* ensure enough parameters */
  if (argc < 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    goto QUIT;
  }

  /* create a new batch request */
  error = newbatchrequest(argv[1], BatchID);
  if (error) goto QUIT;

  /* wait for final bstatus */
  error = waitforfinalstatus(BatchID);
  if (error) goto QUIT;

  /* query final bstatus, and if completed, print JSON solution */
  error = finalreport(BatchID);
  if (error) goto QUIT;

  /* eliminate batch from the manager */
  error = discardbatch(BatchID);
  if (error) goto QUIT;

QUIT:

  return error;
}
