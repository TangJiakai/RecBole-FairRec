/* Copyright 2020, Gurobi Optimization, LLC */

/* We find alternative epsilon-optimal solutions to a given knapsack
   problem by using PoolSearchMode */

import gurobi.*;

public class Poolsearch {

  public static void main(String[] args) {

    try{
      // Sample data
      int groundSetSize = 10;
      double objCoef[] = new double[] {32, 32, 15, 15, 6, 6, 1, 1, 1, 1};
      double knapsackCoef[] = new double[] {16, 16,  8,  8, 4, 4, 2, 2, 1, 1};
      double Budget = 33;
      int e, status, nSolutions;

      // Create environment
      GRBEnv env = new GRBEnv("Poolsearch.log");

      // Create initial model
      GRBModel model = new GRBModel(env);
      model.set(GRB.StringAttr.ModelName, "Poolsearch");

      // Initialize decision variables for ground set:
      // x[e] == 1 if element e is chosen
      GRBVar[] Elem = model.addVars(groundSetSize, GRB.BINARY);
      model.set(GRB.DoubleAttr.Obj, Elem, objCoef, 0, groundSetSize);

      for (e = 0; e < groundSetSize; e++) {
        Elem[e].set(GRB.StringAttr.VarName, "El" + String.valueOf(e));
      }

      // Constraint: limit total number of elements to be picked to be at most
      // Budget
      GRBLinExpr lhs = new GRBLinExpr();
      for (e = 0; e < groundSetSize; e++) {
        lhs.addTerm(knapsackCoef[e], Elem[e]);
      }
      model.addConstr(lhs, GRB.LESS_EQUAL, Budget, "Budget");

      // set global sense for ALL objectives
      model.set(GRB.IntAttr.ModelSense, GRB.MAXIMIZE);

      // Limit how many solutions to collect
      model.set(GRB.IntParam.PoolSolutions, 1024);

      // Limit the search space by setting a gap for the worst possible solution that will be accepted
      model.set(GRB.DoubleParam.PoolGap, 0.10);

      // do a systematic search for the k-best solutions
      model.set(GRB.IntParam.PoolSearchMode, 2);

      // save problem
      model.write("Poolsearch.lp");

      // Optimize
      model.optimize();

      // Status checking
      status = model.get(GRB.IntAttr.Status);

      if (status == GRB.INF_OR_UNBD ||
          status == GRB.INFEASIBLE  ||
          status == GRB.UNBOUNDED     ) {
        System.out.println("The model cannot be solved " +
               "because it is infeasible or unbounded");
        System.exit(1);
      }
      if (status != GRB.OPTIMAL) {
        System.out.println("Optimization was stopped with status " + status);
        System.exit(1);
      }

      // Print best selected set
      System.out.println("Selected elements in best solution:");
      System.out.print("\t");
      for (e = 0; e < groundSetSize; e++) {
        if (Elem[e].get(GRB.DoubleAttr.X) < .9) continue;
        System.out.print(" El" + e);
      }
      System.out.println();

      // Print number of solutions stored
      nSolutions = model.get(GRB.IntAttr.SolCount);
      System.out.println("Number of solutions found: " + nSolutions);

      // Print objective values of solutions
      for (e = 0; e < nSolutions; e++) {
        model.set(GRB.IntParam.SolutionNumber, e);
        System.out.print(model.get(GRB.DoubleAttr.PoolObjVal) +  " ");
        if (e%15 == 14) System.out.println();
      }
      System.out.println();

      // print fourth best set if available
      if (nSolutions >= 4) {
        model.set(GRB.IntParam.SolutionNumber, 3);

        System.out.println("Selected elements in fourth best solution:");
        System.out.print("\t");
        for (e = 0; e < groundSetSize; e++) {
          if (Elem[e].get(GRB.DoubleAttr.Xn) < .9) continue;
          System.out.print(" El" + e);
        }
        System.out.println();
      }

      model.dispose();
      env.dispose();
    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode() + ". " +
          e.getMessage());
    }
  }

}
