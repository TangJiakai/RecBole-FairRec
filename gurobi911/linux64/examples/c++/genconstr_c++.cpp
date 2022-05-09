/* Copyright 2020, Gurobi Optimization, LLC */

/* In this example we show the use of general constraints for modeling
 * some common expressions. We use as an example a SAT-problem where we
 * want to see if it is possible to satisfy at least four (or all) clauses
 * of the logical for
 *
 * L = (x0 or ~x1 or x2)  and (x1 or ~x2 or x3)  and
 *     (x2 or ~x3 or x0)  and (x3 or ~x0 or x1)  and
 *     (~x0 or ~x1 or x2) and (~x1 or ~x2 or x3) and
 *     (~x2 or ~x3 or x0) and (~x3 or ~x0 or x1)
 *
 * We do this by introducing two variables for each literal (itself and its
 * negated value), a variable for each clause, and then two
 * variables for indicating if we can satisfy four, and another to identify
 * the minimum of the clauses (so if it one, we can satisfy all clauses)
 * and put these two variables in the objective.
 * i.e. the Objective function will be
 *
 * maximize Obj0 + Obj1
 *
 *  Obj0 = MIN(Clause1, ... , Clause8)
 *  Obj1 = 1 -> Clause1 + ... + Clause8 >= 4
 *
 * thus, the objective value will be two if and only if we can satisfy all
 * clauses; one if and only if at least four clauses can be satisfied, and
 * zero otherwise.
 */


#include "gurobi_c++.h"
#include <sstream>
#include <iomanip>
using namespace std;

#define n         4
#define NLITERALS 4  // same as n
#define NCLAUSES  8
#define NOBJ      2

int
main(void)
{
  GRBEnv *env = 0;

  try{
    // Example data
    //   e.g. {0, n+1, 2} means clause (x0 or ~x1 or x2)
    const int Clauses[][3] = {{  0, n+1, 2}, {  1, n+2, 3},
                              {  2, n+3, 0}, {  3, n+0, 1},
                              {n+0, n+1, 2}, {n+1, n+2, 3},
                              {n+2, n+3, 0}, {n+3, n+0, 1}};

    int i, status;

    // Create environment
    env = new GRBEnv("genconstr_c++.log");

    // Create initial model
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "genconstr_c++");

    // Initialize decision variables and objective

    GRBVar Lit[NLITERALS];
    GRBVar NotLit[NLITERALS];
    for (i = 0; i < NLITERALS; i++) {
      ostringstream vname;
      vname << "X" << i;
      Lit[i]    = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());

      vname.str("");
      vname << "notX" << i;
      NotLit[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());
    }

    GRBVar Cla[NCLAUSES];
    for (i = 0; i < NCLAUSES; i++) {
      ostringstream vname;
      vname << "Clause" << i;
      Cla[i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, vname.str());
    }

    GRBVar Obj[NOBJ];
    for (i = 0; i < NOBJ; i++) {
      ostringstream vname;
      vname << "Obj" << i;
      Obj[i] = model.addVar(0.0, 1.0, 1.0, GRB_BINARY, vname.str());
    }

    // Link Xi and notXi
    GRBLinExpr lhs;
    for (i = 0; i < NLITERALS; i++) {
      ostringstream cname;
      cname << "CNSTR_X" << i;
      lhs = 0;
      lhs += Lit[i];
      lhs += NotLit[i];
      model.addConstr(lhs == 1.0, cname.str());
    }

    // Link clauses and literals
    GRBVar clause[3];
    for (i = 0; i < NCLAUSES; i++) {
      for (int j = 0; j < 3; j++) {
        if (Clauses[i][j] >= n) clause[j] = NotLit[Clauses[i][j]-n];
        else                    clause[j] = Lit[Clauses[i][j]];
      }
      ostringstream cname;
      cname << "CNSTR_Clause" << i;
      model.addGenConstrOr(Cla[i], clause, 3, cname.str());
    }

    // Link objs with clauses
    model.addGenConstrMin(Obj[0], Cla, NCLAUSES,
                          GRB_INFINITY, "CNSTR_Obj0");
    lhs = 0;
    for (i = 0; i < NCLAUSES; i++) {
      lhs += Cla[i];
    }
    model.addGenConstrIndicator(Obj[1], 1, lhs >= 4.0, "CNSTR_Obj1");

    // Set global objective sense
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // Save problem
    model.write("genconstr_c++.mps");
    model.write("genconstr_c++.lp");

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

    // Print result
    double objval = model.get(GRB_DoubleAttr_ObjVal);

    if (objval > 1.9)
      cout << "Logical expression is satisfiable" << endl;
    else if (objval > 0.9)
      cout << "At least four clauses can be satisfied" << endl;
    else
      cout << "Not even three clauses can be satisfied" << endl;

  } catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  }
  catch (...) {
    cout << "Exception during optimization" << endl;
  }

  // Free environment
  delete env;

  return 0;
}
