/* Copyright 2020, Gurobi Optimization, LLC */

/* Solve a model with different values of the Method parameter;
   show which value gives the shortest solve time. */

using System;
using Gurobi;

class lpmethod_cs
{
  static void Main(string[] args)
  {
    if (args.Length < 1) {
      Console.Out.WriteLine("Usage: lpmethod_cs filename");
      return;
    }

    try {
      // Read model
      GRBEnv env = new GRBEnv();
      GRBModel model = new GRBModel(env, args[0]);

      // Solve the model with different values of Method
      int bestMethod = -1;
      double bestTime = model.Parameters.TimeLimit;
      for (int i = 0; i <= 2; ++i)
      {
        model.Reset();
        model.Parameters.Method = i;
        model.Optimize();
        if (model.Status == GRB.Status.OPTIMAL)
        {
          bestTime = model.Runtime;
          bestMethod = i;
          // Reduce the TimeLimit parameter to save time
          // with other methods
          model.Parameters.TimeLimit = bestTime;
        }
      }

      // Report which method was fastest
      if (bestMethod == -1) {
        Console.WriteLine("Unable to solve this model");
      } else {
        Console.WriteLine("Solved in " + bestTime
          + " seconds with Method: " + bestMethod);
      }

      // Dispose of model and env
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " + e.Message);
    }
  }
}
