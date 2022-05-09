/* Copyright 2020, Gurobi Optimization, LLC */

/* This example reads a MIP model from a file, solves it in batch mode,
   and prints the JSON solution string.

   You will need a Cluster Manager license for this example to work. */

using System;
using Gurobi;

class batchmode_cs
{
  /// <summary>Set-up the environment for batch mode optimization.
  /// </summary>
  /// <remarks>
  /// The function creates an empty environment, sets all neccessary
  /// parameters, and returns the ready-to-be-started Env object to
  /// caller.
  /// </remarks>
  static void setupbatchenv(ref GRBEnv env)
  {
    env.CSBatchMode    = 1;
    env.CSManager      = "http://localhost:61080";
    env.LogFile        = "batchmode.log";
    env.ServerPassword = "pass";
    env.UserName       = "gurobi";

    // No network communication happened up to this point. This will happen
    // now that we call start().
    env.Start();
  }

  ///<summary>Print batch job error information, if any</summary>
  static void printbatcherrorinfo(ref GRBBatch batch)
  {
    if (batch.BatchErrorCode == 0)
      return;
    Console.WriteLine("Batch ID: " + batch.BatchID + ", Error code: " +
                      batch.BatchErrorCode + "(" +
                      batch.BatchErrorMessage + ")");
  }

  ///<summary>Create a batch request for given problem file</summary>
  static string newbatchrequest(string filename)
  {
    string batchID= "";
    // Create an empty environment
    GRBEnv env = new GRBEnv(true);

    // set environment and build model
    setupbatchenv(ref env);
    GRBModel model = new GRBModel(env, filename);

    try {

      // Set some parameters
      model.Set(GRB.DoubleParam.MIPGap, 0.01);
      model.Set(GRB.IntParam.JSONSolDetail, 1);

      // Define tags for some variables to access their values later
      int count = 0;
      foreach (GRBVar v in model.GetVars()) {
        v.VTag = "Variable" + count;
        count += 1;
        if (count >= 10) break;
      }

      // Submit batch request
      batchID = model.OptimizeBatch();

    } finally {
      // Dispose of model and env
      model.Dispose();
      env.Dispose();
    }
    return batchID;
  }

  ///<summary>Wait for the final status of the batch. Initially the
  /// status of a batch is <see cref="GRB.BatchStatus.SUBMITTED"/>;
  /// the status will change once the batch has been processed
  /// (by a compute server).</summary>
  static void waitforfinalstatus(string batchID)
  {
    // Wait no longer than one hour
    double maxwaittime = 3600;
    DateTime start = DateTime.Now;

    // Setup and start environment, create local Batch handle object
    GRBEnv env = new GRBEnv(true);
    setupbatchenv(ref env);
    GRBBatch batch = new GRBBatch(env, batchID);
    try {

      while (batch.BatchStatus == GRB.BatchStatus.SUBMITTED) {
        // Abort this batch if it is taking too long
        TimeSpan interval = DateTime.Now - start;
        if (interval.TotalSeconds > maxwaittime) {
          batch.Abort();
          break;
        }
        // Wait for two seconds
        System.Threading.Thread.Sleep(2000);

        // Update the resident attribute cache of the Batch object
        // with the latest values from the cluster manager.
        batch.Update();

        // If the batch failed, we retry it
        if (batch.BatchStatus == GRB.BatchStatus.FAILED) {
          batch.Retry();
          System.Threading.Thread.Sleep(2000);
          batch.Update();
        }
      }
    } finally {
      // Print information about error status of the job
      // that processed the batch
      printbatcherrorinfo(ref batch);
      batch.Dispose();
      env.Dispose();
    }
  }

  ///<summary>Final Report for Batch Request</summary>
  static void printfinalreport(string batchID)
  {
    // Setup and start environment, create local Batch handle object
    GRBEnv env = new GRBEnv(true);
    setupbatchenv(ref env);
    GRBBatch batch = new GRBBatch(env, batchID);

    switch(batch.BatchStatus) {
      case GRB.BatchStatus.CREATED:
        Console.WriteLine("Batch status is 'CREATED'\n");
        break;
      case GRB.BatchStatus.SUBMITTED:
        Console.WriteLine("Batch is 'SUBMITTED\n");
        break;
      case GRB.BatchStatus.ABORTED:
        Console.WriteLine("Batch is 'ABORTED'\n");
        break;
      case GRB.BatchStatus.FAILED:
        Console.WriteLine("Batch is 'FAILED'\n");
        break;
      case GRB.BatchStatus.COMPLETED:
        Console.WriteLine("Batch is 'COMPLETED'\n");
        // Get JSON solution as string
        Console.WriteLine("JSON solution:" + batch.GetJSONSolution());

        // Write the full JSON solution string to a file
        batch.WriteJSONSolution("batch-sol.json.gz");
        break;
      default:
        // Should not happen
        Console.WriteLine("Unknown BatchStatus" + batch.BatchStatus);
        Environment.Exit(1);
        break;
    }
    // Cleanup
    batch.Dispose();
    env.Dispose();
  }

  ///<summary>Instruct the cluster manager to discard all data relating
  /// to this BatchID</summary>
  static void batchdiscard(string batchID)
  {
    // Setup and start environment, create local Batch handle object
    GRBEnv env = new GRBEnv(true);
    setupbatchenv(ref env);
    GRBBatch batch = new GRBBatch(env, batchID);

    // Remove batch request from manager
    batch.Discard();

    // Cleanup
    batch.Dispose();
    env.Dispose();
  }

  ///<summary>Solve a given model using batch optimization</summary>
  static void Main(string[] args)
  {
    if (args.Length < 1) {
      Console.Out.WriteLine("Usage: batchmode_cs filename");
      return;
    }

    try {
      // Submit new batch request
      string batchID = newbatchrequest(args[0]);

      // Wait for final status
      waitforfinalstatus(batchID);

      // Report final status info
      printfinalreport(batchID);

      // Remove batch request from manager
      batchdiscard(batchID);

      Console.WriteLine("Batch optimization OK");
    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " +
          e.Message);
    }
  }
}
