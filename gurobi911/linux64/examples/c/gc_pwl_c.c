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

#include <stdlib.h>
#include <stdio.h>
#include "gurobi_c.h"

int
main(int argc,
     char *argv[])
{
  GRBenv   *env   = NULL;
  GRBmodel *model = NULL;
  int      *cbeg  = NULL;
  int      *clen  = NULL;
  int      *cind  = NULL;
  double   *cval  = NULL;
  double   *rhs   = NULL;
  char     *sense = NULL;
  double   *lb    = NULL;
  double   *obj   = NULL;
  int       nz, i, j;
  int       status;
  double    objval;
  int       error = 0;

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

  /* Create environment */
  error = GRBloadenv(&env, NULL);
  if (error) goto QUIT;

  /* Allocate memory and build the model */
  nz = n; /* count nonzeros for n y variables */
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      if (A[i][j] != 0.0) nz++;
    }
  }

  cbeg  = (int *) malloc(2*n*sizeof(int));
  clen  = (int *) malloc(2*n*sizeof(int));
  cind  = (int *) malloc(nz*sizeof(int));
  cval  = (double *) malloc(nz*sizeof(double));
  rhs   = (double *) malloc((m+1)*sizeof(double));
  sense = (char *) malloc((m+1)*sizeof(char));
  lb    = (double *) malloc(2*n*sizeof(double));
  obj   = (double *) malloc(2*n*sizeof(double));

  for (j = 0; j < n; j++) {
    /* for x variables */
    lb[j]  = -GRB_INFINITY;
    obj[j] = c[j];
    /* for y variables */
    lb[j+n] = 0.0;
    obj[j+n] = 0.0;
  }

  for (i = 0; i < m; i++) {
    rhs[i] = 0.0;
    sense[i] = GRB_LESS_EQUAL;
  }
  sense[m] = GRB_LESS_EQUAL;
  rhs[m] = 3;

  nz = 0;
  for (j = 0; j < n; j++) {
    cbeg[j] = nz;
    for (i = 0; i < m; i++) {
      if (A[i][j] != 0.0 ) {
        cind[nz] = i;
        cval[nz] = A[i][j];
        nz++;
      }
    }
    clen[j] = nz - cbeg[j];
  }

  for (j = 0; j < n; j++) {
    cbeg[n+j] = nz;
    clen[n+j] = 1;
    cind[nz] = m;
    cval[nz] = 1.0;
    nz++;
  }

  error = GRBloadmodel(env, &model, "gc_pwl_c", 2*n, m+1,
                       GRB_MAXIMIZE, 0.0, obj, sense, rhs,
                       cbeg, clen, cind, cval, lb, NULL,
                       NULL, NULL, NULL);
  if (error) goto QUIT;

  /* Add piecewise constraints */
  for (j = 0; j < n; j++) {
    error = GRBaddgenconstrPWL(model, NULL, j, n+j, npts, xpts, ypts);
    if (error) goto QUIT;
  }

  /* Optimize model */
  error = GRBoptimize(model);
  if (error) goto QUIT;

  for (j = 0; j < n; j++) {
    double x;
    error = GRBgetdblattrelement(model, "X", j, &x);
    if (error) goto QUIT;
    printf("x[%d] = %g\n", j, x);
  }

  /* Report the result */
  error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &status);
  if (error) goto QUIT;

  if (status != GRB_OPTIMAL) {
    fprintf(stderr, "Error: it isn't optimal\n");
    goto QUIT;
  }

  error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
  if (error) goto QUIT;
  printf("Obj: %g\n", objval);

QUIT:

  /* Error reporting */
  if (error) {
    printf("ERROR: %s\n", GRBgeterrormsg(env));
    exit(1);
  }

  /* Free data */
  if (cbeg)  free(cbeg);
  if (clen)  free(clen);
  if (cind)  free(cind);
  if (cval)  free(cval);
  if (rhs)   free(rhs);
  if (sense) free(sense);
  if (lb)    free(lb);
  if (obj)   free(obj);

  /* Free model */
  GRBfreemodel(model);

  /* Free environment */
  GRBfreeenv(env);

  return 0;
}
