/* Copyright 2020, Gurobi Optimization, LLC

 This example formulates and solves the following simple model
 with PWL constraints:

  maximize
        sum c[j] * x[j]
  subject to
        sum A[i,j] * x[j] <= 0,  for i = 0, ..., m-1
        sum y[j] <= 3
        y[j] = pwl(x[j]),        for j = 0, ..., n-1
        x[j] free, y[j] >= 0,    for j = 0, ..., n-1
  where pwl(x) = 0,     if x  = 0
               = 1+|x|, if x != 0

  Note
   1. sum pwl(x[j]) <= b is to bound x vector and also to favor sparse x vector.
      Here b = 3 means that at most two x[j] can be nonzero and if two, then
      sum x[j] <= 1
   2. pwl(x) jumps from 1 to 0 and from 0 to 1, if x moves from negative 0 to 0,
      then to positive 0, so we need three points at x = 0. x has infinite bounds
      on both sides, the piece defined with two points (-1, 2) and (0, 1) can
      extend x to -infinite. Overall we can use five points (-1, 2), (0, 1),
      (0, 0), (0, 1) and (1, 2) to define y = pwl(x)
*/

#include "gurobi_c++.h"
#include <sstream>
using namespace std;

int
main(int argc,
     char *argv[])
{
  int n = 5;
  int m = 5;
  double c[] = { 0.5, 0.8, 0.5, 0.1, -1 };
  double A[][5] = { {0, 0, 0, 1, -1},
                    {0, 0, 1, 1, -1},
                    {1, 1, 0, 0, -1},
                    {1, 0, 1, 0, -1},
                    {1, 0, 0, 1, -1} };
  int npts = 5;
  double xpts[] = {-1, 0, 0, 0, 1};
  double ypts[] = {2, 1, 0, 1, 2};

  GRBEnv* env = 0;
  GRBVar* x = 0;
  GRBVar* y = 0;

  try {
    // Env and model
    env = new GRBEnv();
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "gc_pwl_c++");

    // Add variables, set bounds and obj coefficients
    x = model.addVars(n);
    for (int i = 0; i < n; i++) {
      x[i].set(GRB_DoubleAttr_LB, -GRB_INFINITY);
      x[i].set(GRB_DoubleAttr_Obj, c[i]);
    }

    y = model.addVars(n);

    // Set objective to maximize
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

    // Add linear constraints
    for (int i = 0; i < m; i++) {
      GRBLinExpr le = 0;
      for (int j = 0; j < n; j++) {
        le += A[i][j] * x[j];
      }
      model.addConstr(le <= 0);
    }

    GRBLinExpr le1 = 0;
    for (int j = 0; j < n; j++) {
      le1 += y[j];
    }
    model.addConstr(le1 <= 3);

    // Add piecewise constraints
    for (int j = 0; j < n; j++) {
      model.addGenConstrPWL(x[j], y[j], npts, xpts, ypts);
    }

    // Optimize model
    model.optimize();

    for (int j = 0; j < n; j++) {
      cout << "x[" << j << "] = " << x[j].get(GRB_DoubleAttr_X) << endl;
    }

    cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

  }
  catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  }
  catch (...) {
    cout << "Exception during optimization" << endl;
  }

  delete[] x;
  delete[] y;
  delete env;
  return 0;
}
