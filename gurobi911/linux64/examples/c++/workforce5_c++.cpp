/* Copyright 2020, Gurobi Optimization, LLC */

/* Assign workers to shifts; each worker may or may not be available on a
   particular day. We use multi-objective optimization to solve the model.
   The highest-priority objective minimizes the sum of the slacks
   (i.e., the total number of uncovered shifts). The secondary objective
   minimizes the difference between the maximum and minimum number of
   shifts worked among all workers.  The second optimization is allowed
   to degrade the first objective by up to the smaller value of 10% and 2 */

#include "gurobi_c++.h"
#include <sstream>
using namespace std;

int solveAndPrint(GRBModel& model, GRBVar& totSlack,
                  int nWorkers, string* Workers,
                  GRBVar* totShifts);

int
main(int   argc,
     char *argv[])
{
  GRBEnv  *env       = 0;
  GRBVar **x         = 0;
  GRBVar  *slacks    = 0;
  GRBVar  *totShifts = 0;
  int      xCt       = 0;
  int      s, w;

  try {
    // Sample data
    const int nShifts = 14;
    const int nWorkers = 8;

    // Sets of days and workers
    string Shifts[] =
    { "Mon1", "Tue2", "Wed3", "Thu4", "Fri5", "Sat6",
      "Sun7", "Mon8", "Tue9", "Wed10", "Thu11", "Fri12", "Sat13",
      "Sun14" };
    string Workers[] =
    { "Amy", "Bob", "Cathy", "Dan", "Ed", "Fred", "Gu", "Tobi" };

    // Number of workers required for each shift
    double shiftRequirements[] =
    { 3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5 };

    // Worker availability: 0 if the worker is unavailable for a shift
    double availability[][14] =
    { { 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0 },
      { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1 },
      { 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1 },
      { 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1 },
      { 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };

    // Create environment
    env = new GRBEnv("workforce5_c++.log");

    // Create initial model
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "workforce5_c++");

    // Initialize assignment decision variables:
    //  x[w][s] == 1 if worker w is assigned to shift s.
    // This is no longer a pure assignment model, so we must
    // use binary variables.
    x = new GRBVar*[nWorkers];
    for (w = 0; w < nWorkers; w++) {
      x[w] = model.addVars(nShifts, GRB_BINARY);
      xCt++;
      for (s = 0; s < nShifts; s++) {
        ostringstream vname;

        vname << Workers[w] << "." << Shifts[s];
        x[w][s].set(GRB_DoubleAttr_UB, availability[w][s]);
        x[w][s].set(GRB_StringAttr_VarName, vname.str());
      }
    }

    // Initialize slack decision variables
    slacks = model.addVars(nShifts);
    for (s = 0; s < nShifts; s++) {
      ostringstream vname;

      vname << Shifts[s] << "Slack";
      slacks[s].set(GRB_StringAttr_VarName, vname.str());
    }

    // Variable to represent the total slack
    GRBVar totSlack = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS,
                                   "totSlack");

    // Initialize variables to count the total shifts worked by each worker
    totShifts = model.addVars(nWorkers);
    for (w = 0; w < nWorkers; w++) {
      ostringstream vname;

      vname << Workers[w] << "TotShifts";
      totShifts[w].set(GRB_StringAttr_VarName, vname.str());
    }

    GRBLinExpr lhs;

    // Constraint: assign exactly shiftRequirements[s] workers
    // to each shift s, plus the slack
    for (s = 0; s < nShifts; s++) {
      lhs = 0;
      lhs += slacks[s];
      for (w = 0; w < nWorkers; w++) {
        lhs += x[w][s];
      }
      model.addConstr(lhs == shiftRequirements[s], Shifts[s]);
    }

    // Constraint: set totSlack column equal to the total slack
    lhs = 0;
    for (s = 0; s < nShifts; s++) {
      lhs += slacks[s];
    }
    model.addConstr(lhs == totSlack, "totSlack");

    // Constraint: compute the total number of shifts for each worker
    for (w = 0; w < nWorkers; w++) {
      lhs = 0;

      for (s = 0; s < nShifts; s++) {
        lhs += x[w][s];
      }

      ostringstream vname;
      vname << "totShifts" << Workers[w];
      model.addConstr(lhs == totShifts[w], vname.str());
    }

    // Constraint: set minShift/maxShift variable to less <=/>= to the
    // number of shifts among all workers
    GRBVar minShift = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS,
                                   "minShift");
    GRBVar maxShift = model.addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS,
                                   "maxShift");
    model.addGenConstrMin(minShift, totShifts, nWorkers, GRB_INFINITY, "minShift");
    model.addGenConstrMax(maxShift, totShifts, nWorkers, -GRB_INFINITY, "maxShift");

    // Set global sense for ALL objectives
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    // Set primary objective
    model.setObjectiveN(totSlack, 0, 2, 1.0, 2.0, 0.1, "TotalSlack");

    // Set secondary objective
    model.setObjectiveN(maxShift - minShift, 1, 1, 1.0, 0, 0, "Fairness");

    // Save problem
    model.write("workforce5_c++.lp");

    // Optimize
    int status = solveAndPrint(model, totSlack, nWorkers, Workers, totShifts);

    // Delete local variables
    if (status != GRB_OPTIMAL)
      return 1;
  }
  catch (GRBException e){
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  }
  catch (...) {
    cout << "Exception during optimization" << endl;
  }

  for (s = 0; s < xCt; s++)
    delete[] x[s];
  delete[] x;
  delete[] slacks;
  delete[] totShifts;
  delete env;
  return 0;
}

int solveAndPrint(GRBModel& model,
                  GRBVar&   totSlack,
                  int       nWorkers,
                  string*   Workers,
                  GRBVar*   totShifts)
{
  model.optimize();
  int status = model.get(GRB_IntAttr_Status);

  if ((status == GRB_INF_OR_UNBD) ||
      (status == GRB_INFEASIBLE)  ||
      (status == GRB_UNBOUNDED)     ) {
    cout << "The model cannot be solved " <<
    "because it is infeasible or unbounded" << endl;
    return status;
  }
  if (status != GRB_OPTIMAL) {
    cout << "Optimization was stopped with status " << status << endl;
    return status;
  }

  // Print total slack and the number of shifts worked for each worker
  cout << endl << "Total slack required: " <<
    totSlack.get(GRB_DoubleAttr_X) << endl;
  for (int w = 0; w < nWorkers; ++w) {
    cout << Workers[w] << " worked " <<
    totShifts[w].get(GRB_DoubleAttr_X) << " shifts" << endl;
  }
  cout << endl;

  return status;
}
