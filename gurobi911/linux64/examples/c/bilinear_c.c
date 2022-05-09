/* Copyright 2020, Gurobi Optimization, LLC */

/* This example formulates and solves the following simple bilinear model:

     maximize    x
     subject to  x + y + z <= 10
                 x * y <= 2          (bilinear inequality)
                 x * z + y * z == 1  (bilinear equality)
                 x, y, z non-negative (x integral in second version)
*/

#include <stdlib.h>
#include <stdio.h>
#include "gurobi_c.h"

int
main(int   argc,
     char *argv[])
{
  GRBenv   *env   = NULL;
  GRBmodel *model = NULL;
  int       error = 0;
  double    sol[3];
  int       ind[3];
  double    val[3];
  double    obj[] = {1, 0, 0};
  int       qrow[2];
  int       qcol[2];
  double    qval[2];
  int       optimstatus;
  double    objval;

  /* Create environment */

  error = GRBloadenv(&env, "bilinear.log");
  if (error) goto QUIT;

  /* Create an empty model */

  error = GRBnewmodel(env, &model, "bilinear", 0, NULL, NULL, NULL, NULL,
                      NULL);
  if (error) goto QUIT;


  /* Add variables */

  error = GRBaddvars(model, 3, 0, NULL, NULL, NULL, obj, NULL, NULL, NULL,
                     NULL);
  if (error) goto QUIT;

  /* Change sense to maximization */

  error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
  if (error) goto QUIT;

  /* Linear constraint: x + y + z <= 10 */

  ind[0] = 0; ind[1] = 1; ind[2] = 2;
  val[0] = 1; val[1] = 1; val[2] = 1;

  error = GRBaddconstr(model, 3, ind, val, GRB_LESS_EQUAL, 10.0, "c0");
  if (error) goto QUIT;

  /* Bilinear inequality: x * y <= 2 */

  qrow[0] = 0; qcol[0] = 1; qval[0] = 1.0;

  error = GRBaddqconstr(model, 0, NULL, NULL, 1, qrow, qcol, qval,
                        GRB_LESS_EQUAL, 2.0, "bilinear0");
  if (error) goto QUIT;

  /* Bilinear equality: x * z + y * z == 1 */

  qrow[0] = 0; qcol[0] = 2; qval[0] = 1.0;
  qrow[1] = 1; qcol[1] = 2; qval[1] = 1.0;

  error = GRBaddqconstr(model, 0, NULL, NULL, 2, qrow, qcol, qval,
                        GRB_EQUAL, 1, "bilinear1");
  if (error) goto QUIT;

  /* Optimize model - this will fail since we need to set NonConvex to 2 */

  error = GRBoptimize(model);
  if (!error) {
    printf("Should have failed!\n");
    goto QUIT;
  }

  /* Set parameter and solve again */

  error = GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_NONCONVEX, 2);
  if (error) goto QUIT;

  error = GRBoptimize(model);
  if (error) goto QUIT;

  /* Write model to 'bilinear.lp' */

  error = GRBwrite(model, "bilinear.lp");
  if (error) goto QUIT;

  /* Capture solution information */

  error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
  if (error) goto QUIT;

  error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
  if (error) goto QUIT;

  error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, 3, sol);
  if (error) goto QUIT;

  printf("\nOptimization complete\n");
  if (optimstatus == GRB_OPTIMAL) {
    printf("Optimal objective: %.4e\n", objval);

    printf("  x=%.2f, y=%.2f, z=%.2f\n", sol[0], sol[1], sol[2]);
  } else if (optimstatus == GRB_INF_OR_UNBD) {
    printf("Model is infeasible or unbounded\n");
  } else {
    printf("Optimization was stopped early\n");
  }

  /* Now constrain 'x' to be integral and solve again */

  error = GRBsetcharattrelement(model, GRB_CHAR_ATTR_VTYPE, 0, GRB_INTEGER);
  if (error) goto QUIT;

  error = GRBoptimize(model);
  if (error) goto QUIT;

  /* Capture solution information */

  error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
  if (error) goto QUIT;

  error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
  if (error) goto QUIT;

  error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, 3, sol);
  if (error) goto QUIT;

  printf("\nOptimization complete\n");
  if (optimstatus == GRB_OPTIMAL) {
    printf("Optimal objective: %.4e\n", objval);

    printf("  x=%.2f, y=%.2f, z=%.2f\n", sol[0], sol[1], sol[2]);
  } else if (optimstatus == GRB_INF_OR_UNBD) {
    printf("Model is infeasible or unbounded\n");
  } else {
    printf("Optimization was stopped early\n");
  }

QUIT:

  /* Error reporting */

  if (error) {
    printf("ERROR: %s\n", GRBgeterrormsg(env));
    exit(1);
  }

  /* Free model */

  GRBfreemodel(model);

  /* Free environment */

  GRBfreeenv(env);

  return 0;
}
