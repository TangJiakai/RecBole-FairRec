/* Copyright 2020, Gurobi Optimization, LLC */

/* Want to cover three different sets but subject to a common budget of
   elements allowed to be used. However, the sets have different priorities to
   be covered; and we tackle this by using multi-objective optimization. */

using System;
using Gurobi;

class multiobj_cs {
  static void Main() {

    try {
      // Sample data
      int groundSetSize = 20;
      int nSubsets      = 4;
      int Budget        = 12;
      double[,] Set = new double[,]
      { { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
        { 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0 } };
      int[]    SetObjPriority = new int[] {3, 2, 2, 1};
      double[] SetObjWeight   = new double[] {1.0, 0.25, 1.25, 1.0};
      int e, i, status, nSolutions;

      // Create environment
      GRBEnv env = new GRBEnv("multiobj_cs.log");

      // Create initial model
      GRBModel model = new GRBModel(env);
      model.ModelName = "multiobj_cs";

      // Initialize decision variables for ground set:
      // x[e] == 1 if element e is chosen for the covering.
      GRBVar[] Elem = model.AddVars(groundSetSize, GRB.BINARY);
      for (e = 0; e < groundSetSize; e++) {
        string vname = string.Format("El{0}", e);
        Elem[e].VarName = vname;
      }

      // Constraint: limit total number of elements to be picked to be at most
      // Budget
      GRBLinExpr lhs = new GRBLinExpr();
      for (e = 0; e < groundSetSize; e++) {
        lhs.AddTerm(1.0, Elem[e]);
      }
      model.AddConstr(lhs, GRB.LESS_EQUAL, Budget, "Budget");

      // Set global sense for ALL objectives
      model.ModelSense = GRB.MAXIMIZE;

      // Limit how many solutions to collect
      model.Parameters.PoolSolutions = 100;

      // Set and configure i-th objective
      for (i = 0; i < nSubsets; i++) {
        string vname = string.Format("Set{0}", i);
        GRBLinExpr objn = new GRBLinExpr();
        for (e = 0; e < groundSetSize; e++) {
          objn.AddTerm(Set[i,e], Elem[e]);
        }

        model.SetObjectiveN(objn, i, SetObjPriority[i], SetObjWeight[i],
                            1.0 + i, 0.01, vname);
      }

      // Save problem
      model.Write("multiobj_cs.lp");

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
      if (nSolutions > 10) nSolutions = 10;
      Console.WriteLine("Objective values for first {0} solutions:", nSolutions);
      for (i = 0; i < nSubsets; i++) {
        model.Parameters.ObjNumber = i;

        Console.Write("\tSet" + i);
        for (e = 0; e < nSolutions; e++) {
          model.Parameters.SolutionNumber = e;
          Console.Write("{0,8}", model.ObjNVal);
        }
        Console.WriteLine();
      }
      model.Dispose();
      env.Dispose();
    } catch (GRBException e) {
      Console.WriteLine("Error code = {0}", e);
      Console.WriteLine(e.Message);
    }
  }
}
