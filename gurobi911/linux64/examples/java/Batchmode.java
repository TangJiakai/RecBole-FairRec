/* Copyright 2020, Gurobi Optimization, LLC */

/* This example reads a model from a file, solves it in batch mode
 * and prints the JSON solution string. */

import gurobi.*;

public class Batchmode {

  // Set-up a batch-mode environment
  private static GRBEnv setupbatchconnection() throws GRBException {
    GRBEnv env = new GRBEnv(true);
    env.set(GRB.IntParam.CSBatchMode,       1);
    env.set(GRB.StringParam.LogFile,        "batchmode.log");
    env.set(GRB.StringParam.CSManager,      "http://localhost:61080");
    env.set(GRB.StringParam.UserName,       "gurobi");
    env.set(GRB.StringParam.ServerPassword, "pass");
    env.start();
    return env;
  }

  // Display batch-error if any
  private static void batcherrorinfo(GRBBatch batch) throws GRBException {
    // Get last error code
    int error = batch.get(GRB.IntAttr.BatchErrorCode);
    if (error == 0) return;

    // Query last error message
    String errMsg = batch.get(GRB.StringAttr.BatchErrorMessage);

    // Query batchID
    String batchID = batch.get(GRB.StringAttr.BatchID);

    System.out.println("Batch ID " + batchID + "Error Code " +
                       error + "(" + errMsg + ")");
  }

  // Create a batch request from the given problem file
  private static String newbatchrequest(String filename) throws GRBException {
    // Setup a batch connection
    GRBEnv env = setupbatchconnection();

    // Read a model
    GRBModel model = new GRBModel(env, filename);

    // Set some parameters
    model.set(GRB.DoubleParam.MIPGap,     0.01);
    model.set(GRB.IntParam.JSONSolDetail, 1);

    // Set-up some tags, we need tags to be able to query results
    int count = 0;
    for (GRBVar v: model.getVars()) {
      v.set(GRB.StringAttr.VTag, "UniqueVariableIdentifier" + count);
      count += 1;
      if (count >= 10) break;
    }

    // Batch-mode optimization
    String batchid = model.optimizeBatch();

    // no need to keep the model around
    model.dispose();

    // no need to keep environment
    env.dispose();

    return batchid;
  }

  // Wait for final status
  private static void waitforfinalstatus(String batchid) throws Exception {
    // Setup a batch connection
    GRBEnv env = setupbatchconnection();

    // Create Batch-object
    GRBBatch batch = new GRBBatch(env, batchid);

    try {
      // Query status, and wait for completed
      int status = batch.get(GRB.IntAttr.BatchStatus);
      long timestart = System.currentTimeMillis();

      while(status == GRB.BatchStatus.SUBMITTED) {

        // Abort if taking too long
        long curtime = System.currentTimeMillis();
        if (curtime - timestart > 3600 * 1000) {
          // Request to abort the batch
          batch.abort();
          break;
        }

        // Do not bombard the server
        Thread.sleep(2000);

        // Update local attributes
        batch.update();

        // Query current status
        status = batch.get(GRB.IntAttr.BatchStatus);

        // Deal with failed status
        if (status == GRB.BatchStatus.FAILED ||
            status == GRB.BatchStatus.ABORTED  ) {
          // Retry the batch job
          batch.retry();
        }
      }

    } catch (Exception e) {
      // Display batch-error if any
      batcherrorinfo(batch);
      throw e;
    } finally {
      // Dispose resources
      batch.dispose();
      env.dispose();
    }
  }

  // Final report on batch request
  private static void finalreport(String batchid) throws GRBException {
    
    // Setup a batch connection
    GRBEnv env = setupbatchconnection();

    // Create batch object
    GRBBatch batch = new GRBBatch(env, batchid);

    try {
      int status = batch.get(GRB.IntAttr.BatchStatus);
      // Display depending on batch status
      switch(status) {
        case GRB.BatchStatus.CREATED:
          System.out.println("Batch is 'CREATED'");
          System.out.println("maybe batch-creation process was killed?");
          break;
        case GRB.BatchStatus.SUBMITTED:
          System.out.println("Batch is 'SUBMITTED'");
          System.out.println("Some other user re-submitted this Batch object?");
          break;
        case GRB.BatchStatus.ABORTED:
          System.out.println("Batch is 'ABORTED'");
          break;
        case GRB.BatchStatus.FAILED:
          System.out.println("Batch is 'FAILED'");
          break;
        case GRB.BatchStatus.COMPLETED:

          // print JSON solution into string
          System.out.println("JSON solution:" + batch.getJSONSolution());

          // save solution into a file
          batch.writeJSONSolution("batch-sol.json.gz");
          break;
        default:
          System.out.println("This should not happen, probably points to a user-memory corruption problem");
          System.exit(1);
          break;

      }
    } catch (GRBException e) {
      // Display batch-error if any
      batcherrorinfo(batch);
      throw e;
    } finally {
      // Dispose resources
      batch.dispose();
      env.dispose();
    }
  }

  // Discard batch data from the Cluster Manager
  private static void discardbatch(String batchid) throws GRBException {
    // Setup a batch connection
    GRBEnv env = setupbatchconnection();

    // Create batch object
    GRBBatch batch = new GRBBatch(env, batchid);

    try {
      // Request to erase input and output data related to this batch
      batch.discard();
    } catch (GRBException e) {
      // Display batch-error if any
      batcherrorinfo(batch);
      throw e;
    } finally {
      // Dispose resources
      batch.dispose();
      env.dispose();
    }

  }

  // Main public function
  public static void main(String[] args) {

    // Ensure enough parameters 
    if (args.length < 1) {
      System.out.println("Usage: java Batch filename");
      System.exit(1);
    }

    try {

      // Create a new batch request
      String batchid = newbatchrequest(args[0]);

      // Wait for final status
      waitforfinalstatus(batchid);

      // Query final status, and if completed, print JSON solution
      finalreport(batchid);
      
      // once the user is done, discard all remote information
      discardbatch(batchid);

      // Signal success
      System.out.println("OK");
    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode() + ". "
          + e.getMessage());
    } catch (Exception e) {
      System.out.println("Error");
    }
  }
}
