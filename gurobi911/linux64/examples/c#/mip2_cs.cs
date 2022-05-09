/* Copyright 2020, Gurobi Optimization, LLC */

/* This example reads a MIP model from a file, solves it and
   prints the objective values from all feasible solutions
   generated while solving the MIP. Then it creates the fixed
   model and solves that model. */

using System;
using Gurobi;

class mip2_cs
{
  static void Main(string[] args)
  {
    if (args.Length < 1) {
      Console.Out.WriteLine("Usage: mip2_cs filename");
      return;
    }

    try {
      GRBEnv    env   = new GRBEnv();
      GRBModel  model = new GRBModel(env, args[0]);
      if (model.IsMIP == 0) {
        Console.WriteLine("Model is not a MIP");
        return;
      }

      model.Optimize();

      int optimstatus = model.Status;
      double objval = 0;
      if (optimstatus == GRB.Status.OPTIMAL) {
        objval = model.ObjVal;
        Console.WriteLine("Optimal objective: " + objval);
      } else if (optimstatus == GRB.Status.INF_OR_UNBD) {
        Console.WriteLine("Model is infeasible or unbounded");
        return;
      } else if (optimstatus == GRB.Status.INFEASIBLE) {
        Console.WriteLine("Model is infeasible");
        return;
      } else if (optimstatus == GRB.Status.UNBOUNDED) {
        Console.WriteLine("Model is unbounded");
        return;
      } else {
        Console.WriteLine("Optimization was stopped with status = "
                           + optimstatus);
        return;
      }

      /* Iterate over the solutions and compute the objectives */

      model.Parameters.OutputFlag = 0;

      Console.WriteLine();
      for (int k = 0; k < model.SolCount; ++k) {
        model.Parameters.SolutionNumber = k;
        double objn = model.PoolObjVal;

        Console.WriteLine("Solution " + k + " has objective: " + objn);
      }
      Console.WriteLine();
      model.Parameters.OutputFlag = 1;

      /* Create a fixed model, turn off presolve and solve */

      GRBModel fixedmodel = model.FixedModel();

      fixedmodel.Parameters.Presolve = 0;

      fixedmodel.Optimize();

      int foptimstatus = fixedmodel.Status;

      if (foptimstatus != GRB.Status.OPTIMAL) {
        Console.WriteLine("Error: fixed model isn't optimal");
        return;
      }

      double fobjval = fixedmodel.ObjVal;

      if (Math.Abs(fobjval - objval) > 1.0e-6 * (1.0 + Math.Abs(objval))) {
        Console.WriteLine("Error: objective values are different");
        return;
      }

      GRBVar[] fvars  = fixedmodel.GetVars();
      double[] x      = fixedmodel.Get(GRB.DoubleAttr.X, fvars);
      string[] vnames = fixedmodel.Get(GRB.StringAttr.VarName, fvars);

      for (int j = 0; j < fvars.Length; j++) {
        if (x[j] != 0.0) Console.WriteLine(vnames[j] + " " + x[j]);
      }

      // Dispose of models and env
      fixedmodel.Dispose();
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " + e.Message);
    }
  }
}
