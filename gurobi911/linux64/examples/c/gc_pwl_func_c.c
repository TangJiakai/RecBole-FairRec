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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gurobi_c.h"

static double f(double u) { return exp(u); }
static double g(double u) { return sqrt(u); }

static int
printsol(GRBmodel *m)
{
  double x[4];
  double vio;
  int    error = 0;

  error = GRBgetdblattrarray(m, "X", 0, 4, x);
  if (error) goto QUIT;

  printf("x = %g, u = %g\n", x[0], x[2]);
  printf("y = %g, v = %g\n", x[1], x[3]);

  /* Calculate violation of exp(x) + 4 sqrt(y) <= 9 */
  vio = f(x[0]) + 4*g(x[1]) - 9;
  if (vio < 0.0) vio = 0.0;
  printf("Vio = %g\n", vio);

QUIT:

  return error;
}

int
main(int   argc,
     char *argv[])
{
  GRBenv   *env   = NULL;
  GRBmodel *model = NULL;
  int       error = 0;
  double    lb, ub;
  int       i, len;
  double    intv  = 1e-3;
  double    xmax, ymax, t;
  int       ind[2];
  double    val[2];
  double    x[4];
  double   *xpts  = NULL;
  double   *ypts  = NULL;
  double   *vpts  = NULL;
  double   *upts  = NULL;

  /* Create environment */

  error = GRBloadenv(&env, NULL);
  if (error) goto QUIT;

  /* Create a new model */

  error = GRBnewmodel(env, &model, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  if (error) goto QUIT;

  /* Add variables */

  lb = 0.0; ub = GRB_INFINITY;

  error = GRBaddvar(model, 0, NULL, NULL, 2.0, lb, ub, GRB_CONTINUOUS, "x");
  if (error) goto QUIT;
  error = GRBaddvar(model, 0, NULL, NULL, 1.0, lb, ub, GRB_CONTINUOUS, "y");
  if (error) goto QUIT;
  error = GRBaddvar(model, 0, NULL, NULL, 0.0, lb, ub, GRB_CONTINUOUS, "u");
  if (error) goto QUIT;
  error = GRBaddvar(model, 0, NULL, NULL, 0.0, lb, ub, GRB_CONTINUOUS, "v");
  if (error) goto QUIT;

  /* Change objective sense to maximization */
  error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
  if (error) goto QUIT;

  /* Add linear constraint: u + 4*v <= 9 */
  ind[0] = 2; ind[1] = 3;
  val[0] = 1; val[1] = 4;

  error = GRBaddconstr(model, 2, ind, val, GRB_LESS_EQUAL, 9.0, "c1");
  if (error) goto QUIT;

/* Approach 1) PWL constraint approach */

  xmax = log(9.0);
  len = (int) ceil(xmax/intv) + 1;
  xpts = (double *) malloc(len*sizeof(double));
  upts = (double *) malloc(len*sizeof(double));
  for (i = 0; i < len; i++) {
    xpts[i] = i*intv;
    upts[i] = f(i*intv);
  }

  error = GRBaddgenconstrPWL(model, "gc1", 0, 2, len, xpts, upts);
  if (error) goto QUIT;

  ymax = (9.0/4.0)*(9.0/4.0);
  len = (int) ceil(ymax/intv) + 1;
  ypts = (double *) malloc(len*sizeof(double));
  vpts = (double *) malloc(len*sizeof(double));
  for (i = 0; i < len; i++) {
    ypts[i] = i*intv;
    vpts[i] = g(i*intv);
  }

  error = GRBaddgenconstrPWL(model, "gc2", 1, 3, len, ypts, vpts);
  if (error) goto QUIT;

  /* Optimize the model and print solution */

  error = GRBoptimize(model);
  if (error) goto QUIT;

  error = printsol(model);
  if (error) goto QUIT;

/* Approach 2) General function constraint approach with auto PWL
 *             translation by Gurobi
 */

  /* restore unsolved state and get rid of PWL constraints */
  error = GRBresetmodel(model);
  if (error) goto QUIT;

  ind[0] = 0; ind[1] = 1;
  error = GRBdelgenconstrs(model, 2, ind);
  if (error) goto QUIT;

  error = GRBupdatemodel(model);
  if (error) goto QUIT;

  error = GRBaddgenconstrExp(model, "gcf1", 0, 2, NULL);
  if (error) goto QUIT;

  error = GRBaddgenconstrPow(model, "gcf2", 1, 3, 0.5, NULL);
  if (error) goto QUIT;

  error = GRBsetdblparam(GRBgetenv(model), "FuncPieceLength", 1e-3);
  if (error) goto QUIT;

  /* Optimize the model and print solution */

  error = GRBoptimize(model);
  if (error) goto QUIT;

  error = printsol(model);
  if (error) goto QUIT;

  /* Zoom in, use optimal solution to reduce the ranges and use a smaller
   * pclen=1e-5 to solve it
   */
  error = GRBgetdblattrarray(model, "X", 0, 4, x);
  if (error) goto QUIT;

  t = x[0] - 0.01;
  if (t < 0.0) t = 0.0;
  error = GRBsetdblattrelement(model, "LB", 0, t);
  if (error) goto QUIT;

  t = x[1] - 0.01;
  if (t < 0.0) t = 0.0;
  error = GRBsetdblattrelement(model, "LB", 1, t);
  if (error) goto QUIT;

  error = GRBsetdblattrelement(model, "UB", 0, x[0]+0.01);
  if (error) goto QUIT;

  error = GRBsetdblattrelement(model, "UB", 1, x[1]+0.01);
  if (error) goto QUIT;

  error = GRBupdatemodel(model);
  if (error) goto QUIT;

  error = GRBresetmodel(model);
  if (error) goto QUIT;

  error = GRBsetdblparam(GRBgetenv(model), "FuncPieceLength", 1e-5);
  if (error) goto QUIT;

  /* Optimize the model and print solution */

  error = GRBoptimize(model);
  if (error) goto QUIT;

  error = printsol(model);
  if (error) goto QUIT;

QUIT:

  if (error) {
    printf("ERROR: %s\n", GRBgeterrormsg(env));
    exit(1);
  }

  /* Free data */

  if (xpts) free(xpts);
  if (ypts) free(ypts);
  if (upts) free(upts);
  if (vpts) free(vpts);

  /* Free model */

  GRBfreemodel(model);

  /* Free environment */

  GRBfreeenv(env);

  return 0;
}
