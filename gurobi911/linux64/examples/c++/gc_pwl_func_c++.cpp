/* Copyright 2020, Gurobi Optimization, LLC

This example considers the following nonconvex nonlinear problem

 maximize    2 x    + y
 subject to  exp(x) + 4 sqrt(y) <= 9
             x, y >= 0

 We show you two approaches to solve this:

 1) Use a piecewise-linear approach to handle general function
    constraints (such as exp and sqrt).
    a) Add two variables
       u = exp(x)
       v = sqrt(y)
    b) Compute points (x, u) of u = exp(x) for some step length (e.g., x
       = 0, 1e-3, 2e-3, ..., xmax) and points (y, v) of v = sqrt(y) for
       some step length (e.g., y = 0, 1e-3, 2e-3, ..., ymax). We need to
       compute xmax and ymax (which is easy for this example, but this
       does not hold in general).
    c) Use the points to add two general constraints of type
       piecewise-linear.

 2) Use the Gurobis built-in general function constraints directly (EXP
    and POW). Here, we do not need to compute the points and the maximal
    possible values, which will be done internally by Gurobi.  In this
    approach, we show how to "zoom in" on the optimal solution and
    tighten tolerances to improve the solution quality.

*/
#if defined (WIN32) || defined (WIN64)
#include <Windows.h>
#endif

#include "gurobi_c++.h"
#include <cmath>
using namespace std;

static double f(double u) { return exp(u); }
static double g(double u) { return sqrt(u); }

static void
printsol(GRBModel& m, GRBVar& x, GRBVar& y, GRBVar& u, GRBVar& v)
{
  cout << "x = " << x.get(GRB_DoubleAttr_X) << ", u = " << u.get(GRB_DoubleAttr_X) << endl;
  cout << "y = " << y.get(GRB_DoubleAttr_X) << ", v = " << v.get(GRB_DoubleAttr_X) << endl;
  cout << "Obj = " << m.get(GRB_DoubleAttr_ObjVal) << endl;

  // Calculate violation of exp(x) + 4 sqrt(y) <= 9
  double vio = f(x.get(GRB_DoubleAttr_X)) + 4 * g(y.get(GRB_DoubleAttr_X)) - 9;
  if (vio < 0.0) vio = 0.0;
  cout << "Vio = " << vio << endl;
}

int
main(int argc, char* argv[])
{
  double* xpts = NULL;
  double* ypts = NULL;
  double* vpts = NULL;
  double* upts = NULL;

  try {

    // Create environment

    GRBEnv env = GRBEnv();

    // Create a new model

    GRBModel m = GRBModel(env);

    // Create variables

    double lb = 0.0, ub = GRB_INFINITY;

    GRBVar x = m.addVar(lb, ub, 0.0, GRB_CONTINUOUS, "x");
    GRBVar y = m.addVar(lb, ub, 0.0, GRB_CONTINUOUS, "y");
    GRBVar u = m.addVar(lb, ub, 0.0, GRB_CONTINUOUS, "u");
    GRBVar v = m.addVar(lb, ub, 0.0, GRB_CONTINUOUS, "v");

    // Set objective

    m.setObjective(2*x + y, GRB_MAXIMIZE);

    // Add linear constraint

    m.addConstr(u + 4*v <= 9, "l1");

  // Approach 1) PWL constraint approach

    double intv = 1e-3;
    double xmax = log(9.0);
    int len = (int) ceil(xmax/intv) + 1;
    xpts = new double[len];
    upts = new double[len];
    for (int i = 0; i < len; i++) {
      xpts[i] = i*intv;
      upts[i] = f(i*intv);
    }
    GRBGenConstr gc1 = m.addGenConstrPWL(x, u, len, xpts, upts, "gc1");

    double ymax = (9.0/4.0)*(9.0/4.0);
    len = (int) ceil(ymax/intv) + 1;
    ypts = new double[len];
    vpts = new double[len];
    for (int i = 0; i < len; i++) {
      ypts[i] = i*intv;
      vpts[i] = g(i*intv);
    }
    GRBGenConstr gc2  = m.addGenConstrPWL(y, v, len, ypts, vpts, "gc2");

    // Optimize the model and print solution

    m.optimize();
    printsol(m, x, y, u, v);

  // Approach 2) General function constraint approach with auto PWL
  //             translation by Gurobi

    // restore unsolved state and get rid of PWL constraints
    m.reset();
    m.remove(gc1);
    m.remove(gc2);
    m.update();

    m.addGenConstrExp(x, u, "gcf1");
    m.addGenConstrPow(y, v, 0.5, "gcf2");

    m.set(GRB_DoubleParam_FuncPieceLength, 1e-3);

    // Optimize the model and print solution

    m.optimize();
    printsol(m, x, y, u, v);

    // Zoom in, use optimal solution to reduce the ranges and use a smaller
    // pclen=1e-5 to solve it

    double xval = x.get(GRB_DoubleAttr_X);
    double yval = y.get(GRB_DoubleAttr_X);

    x.set(GRB_DoubleAttr_LB, max(x.get(GRB_DoubleAttr_LB), xval-0.01));
    x.set(GRB_DoubleAttr_UB, min(x.get(GRB_DoubleAttr_UB), xval+0.01));
    y.set(GRB_DoubleAttr_LB, max(y.get(GRB_DoubleAttr_LB), yval-0.01));
    y.set(GRB_DoubleAttr_UB, min(y.get(GRB_DoubleAttr_UB), yval+0.01));
    m.update();
    m.reset();

    m.set(GRB_DoubleParam_FuncPieceLength, 1e-5);

    // Optimize the model and print solution

    m.optimize();
    printsol(m, x, y, u, v);

  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }

  if (xpts) delete[] xpts;
  if (ypts) delete[] ypts;
  if (upts) delete[] upts;
  if (vpts) delete[] vpts;

  return 0;
}
