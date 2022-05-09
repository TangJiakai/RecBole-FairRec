/* Copyright 2020, Gurobi Optimization, LLC */

/* We find alternative epsilon-optimal solutions to a given knapsack
 * problem by using PoolSearchMode */

#include "gurobi_c++.h"
#include <sstream>
#include <iomanip>
using namespace std;


int main(void)
{
  GRBEnv *env  = 0;
  GRBVar *Elem = 0;
  int e, status, nSolutions;

  try {
    // Sample data
    const int groundSetSize = 10;
    double objCoef[10] =
    {32, 32, 15, 15, 6, 6, 1, 1, 1, 1};
    double knapsackCoef[10] =
    {16, 16,  8,  8, 4, 4, 2, 2, 1, 1};
    double Budget = 33;

    // Create environment
    env = new GRBEnv("poolsearch_c++.log");

    // Create initial model
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "poolsearch_c++");

    // Initialize decision variables for ground set:
    // x[e] == 1 if element e is chosen
    Elem = model.addVars(groundSetSize, GRB_BINARY);
    model.set(GRB_DoubleAttr_Obj, Elem, objCoef, groundSetSize);

    for (e = 0; e < groundSetSize; e++) {
      ostringstream vname;
      vname << "El" << e;
      Elem[e].set(GRB_StringAttr_VarName, vname.str());
    }

    // Constraint: limit total number of elements to be picked to be at most
    // Budget
    GRBLinExpr lhs;
    lhs = 0;
    for (e = 0; e < groundSetSize; e++) {
      lhs += Elem[e] * knapsackCoef[e];
    }
    model.addConstr(lhs <= Budget, "Budget");

    // set global sense for ALL objectives
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // Limit how many solutions to collect
    model.set(GRB_IntParam_PoolSolutions, 1024);

    // Limit the search space by setting a gap for the worst possible solution that will be accepted
    model.set(GRB_DoubleParam_PoolGap, 0.10);

    // do a systematic search for the k-best solutions
    model.set(GRB_IntParam_PoolSearchMode, 2);

    // save problem
    model.write("poolsearch_c++.lp");

    // Optimize
    model.optimize();

    // Status checking
    status = model.get(GRB_IntAttr_Status);

    if (status == GRB_INF_OR_UNBD ||
        status == GRB_INFEASIBLE  ||
        status == GRB_UNBOUNDED     ) {
      cout << "The model cannot be solved " <<
             "because it is infeasible or unbounded" << endl;
      return 1;
    }
    if (status != GRB_OPTIMAL) {
      cout << "Optimization was stopped with status " << status << endl;
      return 1;
    }

    // Print best selected set
    cout << "Selected elements in best solution:" << endl << "\t";
    for (e = 0; e < groundSetSize; e++) {
      if (Elem[e].get(GRB_DoubleAttr_X) < .9) continue;
      cout << " El" << e;
    }
    cout << endl;

    // Print number of solutions stored
    nSolutions = model.get(GRB_IntAttr_SolCount);
    cout << "Number of solutions found: " << nSolutions << endl;

    // Print objective values of solutions
    for (e = 0; e < nSolutions; e++) {
      model.set(GRB_IntParam_SolutionNumber, e);
      cout << model.get(GRB_DoubleAttr_PoolObjVal) << " ";
      if (e%15 == 14) cout << endl;
    }
    cout << endl;

    // print fourth best set if available
    if (nSolutions >= 4) {
      model.set(GRB_IntParam_SolutionNumber, 3);

      cout << "Selected elements in fourth best solution:" << endl << "\t";
      for (e = 0; e < groundSetSize; e++) {
        if (Elem[e].get(GRB_DoubleAttr_Xn) < .9) continue;
        cout << " El" << e;
      }
      cout << endl;
    }
  }
  catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  }
  catch (...) {
    cout << "Exception during optimization" << endl;
  }

  // Free environment/vars
  delete[] Elem;
  delete env;
  return 0;
}
