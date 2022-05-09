/* Copyright 2020, Gurobi Optimization, LLC */

/* Assign workers to shifts; each worker may or may not be available on a
   particular day. We use multi-objective optimization to solve the model.
   The highest-priority objective minimizes the sum of the slacks
   (i.e., the total number of uncovered shifts). The secondary objective
   minimizes the difference between the maximum and minimum number of
   shifts worked among all workers.  The second optimization is allowed
   to degrade the first objective by up to the smaller value of 10% and 2 */

using System;
using Gurobi;

class workforce5_cs
{
  static void Main()
  {
    try {

      // Sample data
      // Sets of days and workers
      string[] Shifts =
          new string[]  { "Mon1", "Tue2", "Wed3", "Thu4", "Fri5", "Sat6",
              "Sun7", "Mon8", "Tue9", "Wed10", "Thu11", "Fri12", "Sat13",
              "Sun14" };
      string[] Workers =
          new string[] { "Amy", "Bob", "Cathy", "Dan", "Ed", "Fred", "Gu", "Tobi" };

      int nShifts = Shifts.Length;
      int nWorkers = Workers.Length;

      // Number of workers required for each shift
      double[] shiftRequirements =
          new double[] { 3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5 };

      // Worker availability: 0 if the worker is unavailable for a shift
      double[,] availability =
          new double[,] { { 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1 },
              { 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0 },
              { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
              { 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
              { 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1 },
              { 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1 },
              { 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1 },
              { 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };

      // Create environment
      GRBEnv env = new GRBEnv();

      // Create initial model
      GRBModel model = new GRBModel(env);
      model.ModelName = "workforce5_cs";

      // Initialize assignment decision variables:
      // x[w][s] == 1 if worker w is assigned to shift s.
      // This is no longer a pure assignment model, so we must
      // use binary variables.
      GRBVar[,] x = new GRBVar[nWorkers, nShifts];
      for (int w = 0; w < nWorkers; ++w) {
        for (int s = 0; s < nShifts; ++s) {
          x[w,s] =
              model.AddVar(0, availability[w,s], 0, GRB.BINARY,
                           string.Format("{0}.{1}", Workers[w], Shifts[s]));
        }
      }

      // Slack variables for each shift constraint so that the shifts can
      // be satisfied
      GRBVar[] slacks = new GRBVar[nShifts];
      for (int s = 0; s < nShifts; ++s) {
        slacks[s] =
            model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                         string.Format("{0}Slack", Shifts[s]));
      }

      // Variable to represent the total slack
      GRBVar totSlack = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                                     "totSlack");

      // Variables to count the total shifts worked by each worker
      GRBVar[] totShifts = new GRBVar[nWorkers];
      for (int w = 0; w < nWorkers; ++w) {
        totShifts[w] = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                                    string.Format("{0}TotShifts", Workers[w]));
      }

      GRBLinExpr lhs;

      // Constraint: assign exactly shiftRequirements[s] workers
      // to each shift s, plus the slack
      for (int s = 0; s < nShifts; ++s) {
        lhs = new GRBLinExpr();
        lhs.AddTerm(1.0, slacks[s]);
        for (int w = 0; w < nWorkers; ++w) {
          lhs.AddTerm(1.0, x[w,s]);
        }
        model.AddConstr(lhs, GRB.EQUAL, shiftRequirements[s], Shifts[s]);
      }

      // Constraint: set totSlack equal to the total slack
      lhs = new GRBLinExpr();
      lhs.AddTerm(-1.0, totSlack);
      for (int s = 0; s < nShifts; ++s) {
        lhs.AddTerm(1.0, slacks[s]);
      }
      model.AddConstr(lhs, GRB.EQUAL, 0, "totSlack");

      // Constraint: compute the total number of shifts for each worker
      for (int w = 0; w < nWorkers; ++w) {
        lhs = new GRBLinExpr();
        lhs.AddTerm(-1.0, totShifts[w]);
        for (int s = 0; s < nShifts; ++s) {
          lhs.AddTerm(1.0, x[w,s]);
        }
        model.AddConstr(lhs, GRB.EQUAL, 0, string.Format("totShifts{0}", Workers[w]));
      }

      // Constraint: set minShift/maxShift variable to less <=/>= to the
      // number of shifts among all workers
      GRBVar minShift = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                                     "minShift");
      GRBVar maxShift = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                                     "maxShift");
      model.AddGenConstrMin(minShift, totShifts, GRB.INFINITY, "minShift");
      model.AddGenConstrMax(maxShift, totShifts, -GRB.INFINITY, "maxShift");

      // Set global sense for ALL objectives
      model.ModelSense = GRB.MINIMIZE;

      // Set primary objective
      model.SetObjectiveN(totSlack, 0, 2, 1.0, 2.0, 0.1, "TotalSlack");

      // Set secondary objective
      model.SetObjectiveN(maxShift - minShift, 1, 1, 1.0, 0, 0, "Fairness");

      // Save problem
      model.Write("workforce5_cs.lp");

      // Optimize
      int status = solveAndPrint(model, totSlack, nWorkers, Workers, totShifts);

      if (status != GRB.Status.OPTIMAL)
        return;

      // Dispose of model and environment
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: {0}. {1}", e.ErrorCode, e.Message);
    }
  }

  private static int solveAndPrint(GRBModel model, GRBVar totSlack,
                                   int nWorkers, String[] Workers,
                                   GRBVar[] totShifts)
  {

    model.Optimize();
    int status = model.Status;
    if (status == GRB.Status.INF_OR_UNBD ||
        status == GRB.Status.INFEASIBLE  ||
        status == GRB.Status.UNBOUNDED     ) {
      Console.WriteLine("The model cannot be solved "
          + "because it is infeasible or unbounded");
      return status;
    }
    if (status != GRB.Status.OPTIMAL ) {
      Console.WriteLine("Optimization was stopped with status {0}", status);
      return status;
    }

    // Print total slack and the number of shifts worked for each worker
    Console.WriteLine("\nTotal slack required: {0}", totSlack.X);
    for (int w = 0; w < nWorkers; ++w) {
      Console.WriteLine("{0} worked {1} shifts", Workers[w], totShifts[w].X);
    }
    Console.WriteLine("\n");
    return status;
  }

}
