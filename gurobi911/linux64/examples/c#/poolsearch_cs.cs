/* Copyright 2020, Gurobi Optimization, LLC */

/* We find alternative epsilon-optimal solutions to a given knapsack
   problem by using PoolSearchMode */

using System;
using Gurobi;

class poolsearch_cs {

  static void Main() {

    try{
      // Sample data
      int groundSetSize = 10;
      double[] objCoef = new double[] {32, 32, 15, 15, 6, 6, 1, 1, 1, 1};
      double[] knapsackCoef = new double[] {16, 16,  8,  8, 4, 4, 2, 2, 1, 1};
      double Budget = 33;
      int e, status, nSolutions;

      // Create environment
      GRBEnv env = new GRBEnv("poolsearch_cs.log");

      // Create initial model
      GRBModel model = new GRBModel(env);
      model.ModelName = "poolsearch_cs";

      // Initialize decision variables for ground set:
      // x[e] == 1 if element e is chosen
      GRBVar[] Elem = model.AddVars(groundSetSize, GRB.BINARY);
      model.Set(GRB.DoubleAttr.Obj, Elem, objCoef, 0, groundSetSize);

      for (e = 0; e < groundSetSize; e++) {
        Elem[e].VarName = string.Format("El{0}", e);
      }

      // Constraint: limit total number of elements to be picked to be at most
      // Budget
      GRBLinExpr lhs = new GRBLinExpr();
      for (e = 0; e < groundSetSize; e++) {
        lhs.AddTerm(knapsackCoef[e], Elem[e]);
      }
      model.AddConstr(lhs, GRB.LESS_EQUAL, Budget, "Budget");

      // set global sense for ALL objectives
      model.ModelSense = GRB.MAXIMIZE;

      // Limit how many solutions to collect
      model.Parameters.PoolSolutions = 1024;

      // Limit the search space by setting a gap for the worst possible solution that will be accepted
      model.Parameters.PoolGap = 0.10;

      // do a systematic search for the k-best solutions
      model.Parameters.PoolSearchMode = 2;

      // save problem
      model.Write("poolsearch_cs.lp");

      // Optimize
      model.Optimize();

      // Status checking
      status = model.Status;

      if (status == GRB.Status.INF_OR_UNBD ||
          status == GRB.Status.INFEASIBLE  ||
          status == GRB.Status.UNBOUNDED     ) {
        Console.WriteLine("The model cannot be solved " +
               "because it is infeasible or unbounded");
        return;
      }
      if (status != GRB.Status.OPTIMAL) {
        Console.WriteLine("Optimization was stopped with status {0}", status);
        return;
      }

      // Print best selected set
      Console.WriteLine("Selected elements in best solution:");
      Console.Write("\t");
      for (e = 0; e < groundSetSize; e++) {
        if (Elem[e].X < .9) continue;
        Console.Write("El{0} ", e);
      }
      Console.WriteLine();

      // Print number of solutions stored
      nSolutions = model.SolCount;
      Console.WriteLine("Number of solutions found: {0}", nSolutions);

      // Print objective values of solutions
      for (e = 0; e < nSolutions; e++) {
        model.Parameters.SolutionNumber = e;
        Console.Write("{0} ", model.PoolObjVal);
        if (e%15 == 14) Console.WriteLine();
      }
      Console.WriteLine();

      // Print fourth best set if available
      if (nSolutions >= 4) {
        model.Parameters.SolutionNumber = 3;

        Console.WriteLine("Selected elements in fourth best solution:");
        Console.Write("\t");
        for (e = 0; e < groundSetSize; e++) {
          if (Elem[e].Xn < .9) continue;
          Console.Write("El{0} ", e);
        }
        Console.WriteLine();
      }

      model.Dispose();
      env.Dispose();
    } catch (GRBException e) {
      Console.WriteLine("Error code: {0}. {1}", e.ErrorCode, e.Message);
    }
  }

}
