/* Copyright 2020, Gurobi Optimization, LLC */

/* We find alternative epsilon-optimal solutions to a given knapsack
 * problem by using PoolSearchMode */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gurobi_c.h"

#define MAXSTR 128


int main(void)
{
  GRBenv   *env   = NULL;
  GRBenv   *menv  = NULL;
  GRBmodel *model = NULL;
  int       error = 0;
  char      buffer[MAXSTR];
  int e, status, nSolutions, prlen;
  double objval, *cval = NULL;
  int *cind = NULL;

  /* Sample data */
  const int groundSetSize = 10;
  double objCoef[10] =
    {32, 32, 15, 15, 6, 6, 1, 1, 1, 1};
  double knapsackCoef[10] =
    {16, 16,  8,  8, 4, 4, 2, 2, 1, 1};
  double Budget = 33;

  /* Create environment */
  error = GRBloadenv(&env, "poolsearch_c.log");
  if (error) goto QUIT;

  /* Create initial model */
  error = GRBnewmodel(env, &model, "poolsearch_c", groundSetSize, NULL,
                      NULL, NULL, NULL, NULL);
  if (error) goto QUIT;

  /* get model environment */
  menv = GRBgetenv(model);
  if (!menv) {
    fprintf(stderr, "Error: could not get model environment\n");
    goto QUIT;
  }

  /* set objective function */
  error = GRBsetdblattrarray(model, "Obj", 0, groundSetSize, objCoef);
  if (error) goto QUIT;

  /* set variable types and names */
  for (e = 0; e < groundSetSize; e++) {
    sprintf(buffer, "El%d", e);
    error = GRBsetcharattrelement(model, "VType", e, GRB_BINARY);
    if (error) goto QUIT;

    error = GRBsetstrattrelement(model, "VarName", e, buffer);
    if (error) goto QUIT;
  }

  /* Make space for constraint data */
  cind = malloc(sizeof(int) * groundSetSize);
  if (!cind) goto QUIT;
  for (e = 0; e < groundSetSize; e++)
    cind[e] = e;

  /* Constraint: limit total number of elements to be picked to be at most
   * Budget */
  sprintf (buffer, "Budget");
  error = GRBaddconstr(model, groundSetSize, cind, knapsackCoef,
                       GRB_LESS_EQUAL, Budget, buffer);
  if (error) goto QUIT;

  /* set global sense for ALL objectives */
  error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
  if (error) goto QUIT;

  /* Limit how many solutions to collect */
  error = GRBsetintparam(menv, GRB_INT_PAR_POOLSOLUTIONS, 1024);
  if (error) goto QUIT;

  /* Limit the search space by setting a gap for the worst possible solution that will be accepted */
  error = GRBsetdblparam(menv, GRB_DBL_PAR_POOLGAP, 0.10);
  if (error) goto QUIT;

  /* do a systematic search for the k-best solutions */
  error = GRBsetintparam(menv, GRB_INT_PAR_POOLSEARCHMODE, 2);
  if (error) goto QUIT;

  /* save problem */
  error = GRBwrite(model, "poolsearch_c.lp");
  if (error) goto QUIT;
  error = GRBwrite(model, "poolsearch_c.mps");
  if (error) goto QUIT;

  /* Optimize */
  error = GRBoptimize(model);
  if (error) goto QUIT;

  /* Status checking */
  error = GRBgetintattr(model, "Status", &status);
  if (error) goto QUIT;

  if (status == GRB_INF_OR_UNBD ||
      status == GRB_INFEASIBLE  ||
      status == GRB_UNBOUNDED     ) {
    printf("The model cannot be solved "
           "because it is infeasible or unbounded\n");
    goto QUIT;
  }
  if (status != GRB_OPTIMAL) {
    printf("Optimization was stopped with status %d\n", status);
    goto QUIT;
  }

  /* make space for optimal solution */
  cval = malloc(sizeof(double) * groundSetSize);
  if (!cval) goto QUIT;

  /* Print best selected set */
  error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, groundSetSize, cval);
  if (error) goto QUIT;

  printf("Selected elements in best solution:\n\t");
  for (e = 0; e < groundSetSize; e++) {
    if (cval[e] < .9) continue;
    printf("El%d ", e);
  }

  /* print number of solutions stored */
  error = GRBgetintattr(model, GRB_INT_ATTR_SOLCOUNT, &nSolutions);
  if (error) goto QUIT;
  printf("\nNumber of solutions found: %d\nValues:", nSolutions);

  /* print objective values of alternative solutions */
  prlen = 0;
  for (e = 0; e < nSolutions; e++) {
    error = GRBsetintparam(menv, GRB_INT_PAR_SOLUTIONNUMBER, e);
    if (error) goto QUIT;

    error = GRBgetdblattr(model, GRB_DBL_ATTR_POOLOBJVAL, &objval);
    if (error) goto QUIT;

    prlen += printf(" %g", objval);
    if (prlen >= 75 && e+1 < nSolutions) {
      prlen = printf("\n    ");
    }
  }
  printf("\n");

  /* print fourth best set if available */
  if (nSolutions >= 4) {
    error = GRBsetintparam(menv, GRB_INT_PAR_SOLUTIONNUMBER, 3);
    if (error) goto QUIT;

    /* get the solution vector */
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_XN, 0, groundSetSize, cval);
    if (error) goto QUIT;

    printf("Selected elements in fourth best solution:\n\t");
    for (e = 0; e < groundSetSize; e++) {
      if (cval[e] < .9) continue;
      printf("El%d ", e);
    }
    printf("\n");
  }

QUIT:
  if (model != NULL) GRBfreemodel(model);
  if (env != NULL)   GRBfreeenv(env);
  if (cind != NULL)  free(cind);
  if (cval != NULL)  free(cval);
  return error;
}
