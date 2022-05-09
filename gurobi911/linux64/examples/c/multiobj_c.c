/* Copyright 2020, Gurobi Optimization, LLC */

/* Want to cover three different sets but subject to a common budget of
 * elements allowed to be used. However, the sets have different priorities to
 * be covered; and we tackle this by using multi-objective optimization. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gurobi_c.h"

#define MAXSTR 128

int
main(void)
{
  GRBenv   *env   = NULL;
  GRBenv   *menv  = NULL;
  GRBmodel *model = NULL;
  int       error = 0;
  int      *cind  = NULL;
  double   *cval  = NULL;
  char      buffer[MAXSTR];
  int e, i, status, nSolutions;
  double objn;

  /* Sample data */
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

  /* Create environment */
  error = GRBloadenv(&env, "multiobj_c.log");
  if (error) goto QUIT;

  /* Create initial model */
  error = GRBnewmodel(env, &model, "multiobj_c", groundSetSize, NULL,
                      NULL, NULL, NULL, NULL);
  if (error) goto QUIT;

  /* get model environment */
  menv = GRBgetenv(model);
  if (!menv) {
    fprintf(stderr, "Error: could not get model environment\n");
    goto QUIT;
  }

  /* Initialize decision variables for ground set:
   * x[e] == 1 if element e is chosen for the covering. */
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
  cval = malloc(sizeof(double) * groundSetSize);
  if (!cval) goto QUIT;

  /* Constraint: limit total number of elements to be picked to be at most
   * Budget */
  for (e = 0; e < groundSetSize; e++) {
    cind[e] = e;
    cval[e] = 1.0;
  }
  sprintf (buffer, "Budget");
  error = GRBaddconstr(model, groundSetSize, cind, cval, GRB_LESS_EQUAL,
                       (double)Budget, buffer);
  if (error) goto QUIT;

  /* Set global sense for ALL objectives */
  error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
  if (error) goto QUIT;

  /* Limit how many solutions to collect */
  error = GRBsetintparam(menv, GRB_INT_PAR_POOLSOLUTIONS, 100);
  if (error) goto QUIT;

  /* Set and configure i-th objective */
  for (i = 0; i < nSubsets; i++) {
    sprintf(buffer, "Set%d", i+1);

    error = GRBsetobjectiven(model, i, SetObjPriority[i], SetObjWeight[i],
                             1.0 + i, 0.01, buffer, 0.0, groundSetSize,
                             cind, Set[i]);
    if (error) goto QUIT;
  }

  /* Save problem */
  error = GRBwrite(model, "multiobj_c.lp");
  if (error) goto QUIT;
  error = GRBwrite(model, "multiobj_c.mps");
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
    printf("Optimization was stopped with status %i\n", status);
    goto QUIT;
  }

  /* Print best selected set */
  error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, groundSetSize, cval);
  if (error) goto QUIT;

  printf("Selected elements in best solution:\n\t");
  for (e = 0; e < groundSetSize; e++) {
    if (cval[e] < .9) continue;
    printf("El%d ", e);
  }

  /* Print number of solutions stored */
  error = GRBgetintattr(model, GRB_INT_ATTR_SOLCOUNT, &nSolutions);
  if (error) goto QUIT;
  printf("\nNumber of solutions found: %d\n", nSolutions);

  /* Print objective values of solutions */

  if (nSolutions > 10) nSolutions = 10;
  printf("Objective values for first %d solutions:\n", nSolutions);
  for (i = 0; i < nSubsets; i++) {
    error = GRBsetintparam(menv, GRB_INT_PAR_OBJNUMBER, i);
    if (error) goto QUIT;

    printf("\tSet %d:", i);
    for (e = 0; e < nSolutions; e++) {
      error = GRBsetintparam(menv, GRB_INT_PAR_SOLUTIONNUMBER, e);
      if (error) goto QUIT;

      error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJNVAL, &objn);
      if (error) goto QUIT;

      printf(" %6g", objn);
    }
    printf("\n");
  }

QUIT:

  if (cind != NULL)  free(cind);
  if (cval != NULL)  free(cval);
  if (model != NULL) GRBfreemodel(model);
  if (env != NULL)   GRBfreeenv(env);

  return error;
}
