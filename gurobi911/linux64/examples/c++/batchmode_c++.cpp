/* Copyright 2020, Gurobi Optimization, LLC */

// This example reads a MIP model from a file, solves it in batch mode,
// and prints the JSON solution string.
//
// You will need a Cluster Manager license for this example to work.

#include <ctime>
#if defined (WIN32) || defined (WIN64) || defined(_WIN32) || defined (_WIN64)
#include <Windows.h>
#define sleep(n) Sleep(1000*n)
#else
#include <unistd.h>
#endif
#include "gurobi_c++.h"
using namespace std;

// Set-up the environment for batch mode optimization.
//
// The function configures and start an environment to be used for batch
// optimization.
void
setupbatchenv(GRBEnv* env)
{
  env->set(GRB_StringParam_LogFile,        "batchmode.log");
  env->set(GRB_StringParam_CSManager,      "http://localhost:61080");
  env->set(GRB_StringParam_UserName,       "gurobi");
  env->set(GRB_StringParam_ServerPassword, "pass");
  env->set(GRB_IntParam_CSBatchMode, 1);

  // No network communication happened up to this point. This will happen
  // now that we call the start() method.
  env->start();
}

// Print batch job error information, if any
void
printbatcherrorinfo(GRBBatch &batch)
{
  if (batch.get(GRB_IntAttr_BatchErrorCode) == 0)
    return;

  cerr << "Batch ID " << batch.get(GRB_StringAttr_BatchID)
       << ": Error code " << batch.get(GRB_IntAttr_BatchErrorCode)
       << " (" << batch.get(GRB_StringAttr_BatchErrorMessage)
       << ")" << endl;
}

// Create a batch request for given problem file
string
newbatchrequest(char* filename)
{
  GRBEnv*   env   = NULL;
  GRBModel* model = NULL;
  GRBVar*   v     = NULL;
  string batchID;

  try {
    // Start environment, create Model object from file
    env = new GRBEnv(true);
    setupbatchenv(env);
    model = new GRBModel(*env, filename);

    // Set some parameters; switch on detailed JSON information
    model->set(GRB_DoubleParam_MIPGap, 0.01);
    model->set(GRB_IntParam_JSONSolDetail, 1);

    // Define tags for some variables in order to access their values later
    int numvars = model->get(GRB_IntAttr_NumVars);
    v = model->getVars();
    if (numvars > 10) numvars = 10;
    for (int j = 0; j < numvars; j++) {
      char vtag[64];
      sprintf(vtag, "Variable %d", j);
      v[j].set(GRB_StringAttr_VTag, string(vtag));
    }

    // submit batch request
    batchID = model->optimizeBatch();

  } catch (...) {
    // Free local resources
    delete[] v;
    delete model;
    delete env;
    // Let the exception propagate
    throw;
  }

  // Free local resources
  delete[] v;
  delete model;
  delete env;

  return batchID;
}

// Wait for the final status of the batch.
// Initially the status of a batch is "submitted"; the status will change
// once the batch has been processed (by a compute server).
void
waitforfinalstatus(string batchID)
{
  // Wait no longer than one hour
  time_t maxwaittime = 3600;
  GRBEnv*   env   = NULL;
  GRBBatch* batch = NULL;


  try {
    // Setup and start environment, create local Batch handle object
    env = new GRBEnv(true);
    setupbatchenv(env);
    batch = new GRBBatch(*env, batchID);
    time_t starttime = time(NULL);
    int BatchStatus = batch->get(GRB_IntAttr_BatchStatus);

    while (BatchStatus == GRB_BATCH_SUBMITTED) {

      // Abort this batch if it is taking too long
      time_t curtime = time(NULL);
      if (curtime - starttime > maxwaittime) {
        batch->abort();
        break;
      }

      // Wait for two seconds
      sleep(2);

      // Update the resident attribute cache of the Batch object with the
      // latest values from the cluster manager.
      batch->update();
      BatchStatus = batch->get(GRB_IntAttr_BatchStatus);

      // If the batch failed, we try again
      if (BatchStatus == GRB_BATCH_FAILED)
        batch->retry();
    }
  } catch (...) {
    // Print information about error status of the job that
    // processed the batch
    printbatcherrorinfo(*batch);
    // Free local resources
    delete batch;
    delete env;
    // let the exception propagate
    throw;
  }

  // Free local resources
  delete batch;
  delete env;
}

void
printfinalreport(string batchID)
{
  GRBEnv*   env   = NULL;
  GRBBatch* batch = NULL;

  try {
    // Setup and starts environment, create local Batch handle object
    env = new GRBEnv(true);
    setupbatchenv(env);
    batch = new GRBBatch(*env, batchID);

    int BatchStatus = batch->get(GRB_IntAttr_BatchStatus);
    if (BatchStatus == GRB_BATCH_CREATED)
      cout << "Batch status is 'CREATED'" << endl;
    else if (BatchStatus == GRB_BATCH_SUBMITTED)
      cout << "Batch is 'SUBMITTED" << endl;
    else if (BatchStatus == GRB_BATCH_ABORTED)
      cout << "Batch is 'ABORTED'" << endl;
    else if (BatchStatus == GRB_BATCH_FAILED)
      cout << "Batch is 'FAILED'" << endl;
    else if (BatchStatus == GRB_BATCH_COMPLETED) {
      cout << "Batch is 'COMPLETED'" << endl;
      // Pretty printing the general solution information
      cout << "JSON solution:" << batch->getJSONSolution() << endl;

      // Write the full JSON solution string to a file
      batch->writeJSONSolution("batch-sol.json.gz");
    } else {
      // Should not happen
      cout << "Batch has unknown BatchStatus" << endl;
    }
  } catch (...) {
    // Free local resources
    delete batch;
    delete env;
    // let the exception propagate
    throw;
  }

  // Free local resources
  delete batch;
  delete env;
}

// Instruct cluster manager to remove all data relating to this BatchID
void
batchdiscard(string batchID)
{
  GRBEnv*   env   = NULL;
  GRBBatch* batch = NULL;

  try {
    // Setup and start environment, create local Batch handle object
    env = new GRBEnv(true);
    setupbatchenv(env);
    batch = new GRBBatch(*env, batchID);

    // Remove batch request from manager
    batch->discard();
  } catch (...) {
    // Free local resources even
    delete batch;
    delete env;
    // let the exception propagate
    throw;
  }

  // Free local resources
  delete batch;
  delete env;
}

// Solve a given model using batch optimization
int
main(int   argc,
    char** argv)
{
  // Ensure we have an input file
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " filename" << endl;
    return 0;
  }

  try {
    // Submit new batch request
    string batchID = newbatchrequest(argv[1]);

    // Wait for final status
    waitforfinalstatus(batchID);

    // Report final status info
    printfinalreport(batchID);

    // Remove batch request from manager
    batchdiscard(batchID);

    cout << "Batch optimization OK" << endl;
  } catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch (...) {
    cout << "Exception during optimization" << endl;
  }
  return 0;
}
