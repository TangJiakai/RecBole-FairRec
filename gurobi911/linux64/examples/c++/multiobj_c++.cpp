/* Copyright 2020, Gurobi Optimization, LLC */

/* Want to cover three different sets but subject to a common budget of
 * elements allowed to be used. However, the sets have different priorities to
 * be covered; and we tackle this by using multi-objective optimization. */

#include "gurobi_c++.h"
#include <sstream>
#include <iomanip>
using namespace std;

int
main(void)
{
  GRBEnv *env  = 0;
  GRBVar *Elem = 0;
  int e, i, status, nSolutions;

  try{
    // Sample data
    const int groundSetSize = 20;
    const int nSubsets      = 4;
    const int Budget        = 12;
    double Set[][20] =
    { { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
      { 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0 } };
    int    SetObjPriority[] = {3, 2, 2, 1};
    double SetObjWeight[]   = {1.0, 0.25, 1.25, 1.0};

    // Create environment
    env = new GRBEnv("multiobj_c++.log");

    // Create initial model
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "multiobj_c++");

    // Initialize decision variables for ground set:
    // x[e] == 1 if element e is chosen for the covering.
    Elem = model.addVars(groundSetSize, GRB_BINARY);
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
      lhs += Elem[e];
    }
    model.addConstr(lhs <= Budget, "Budget");

    // Set global sense for ALL objectives
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // Limit how many solutions to collect
    model.set(GRB_IntParam_PoolSolutions, 100);

    // Set and configure i-th objective
    for (i = 0; i < nSubsets; i++) {
      GRBLinExpr objn = 0;
      for (e = 0; e < groundSetSize; e++)
        objn += Set[i][e]*Elem[e];
      ostringstream vname;
      vname << "Set" << i;

      model.setObjectiveN(objn, i, SetObjPriority[i], SetObjWeight[i],
                          1.0 + i, 0.01, vname.str());
    }

    // Save problem
    model.write("multiobj_c++.lp");

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
    if (nSolutions > 10) nSolutions = 10;
    cout << "Objective values for first " << nSolutions;
    cout << " solutions:" << endl;
    for (i = 0; i < nSubsets; i++) {
      model.set(GRB_IntParam_ObjNumber, i);

      cout << "\tSet" << i;
      for (e = 0; e < nSolutions; e++) {
        cout << " ";
        model.set(GRB_IntParam_SolutionNumber, e);
        double val = model.get(GRB_DoubleAttr_ObjNVal);
        cout << std::setw(6) << val;
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
