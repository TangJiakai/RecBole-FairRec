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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gurobi_c.h"

#define MAXSTR    128
#define NLITERALS 4
#define NCLAUSES  8
#define NOBJ      2
#define NVARS     (2 * NLITERALS + NCLAUSES + NOBJ)
#define LIT(n)    (n)
#define NOTLIT(n) (NLITERALS + n)
#define CLA(n)    (2 * NLITERALS + n)
#define OBJ(n)    (2 * NLITERALS + NCLAUSES + n)


int
main(void)
{
  GRBenv   *env   = NULL;
  GRBmodel *model = NULL;
  int       error = 0;
  int       cind[NVARS];
  double    cval[NVARS];
  char      buffer[MAXSTR];
  int col, i, status;
  double objval;

  /* Example data */
  const int Clauses[][3] = {{LIT(0), NOTLIT(1), LIT(2)},
                            {LIT(1), NOTLIT(2), LIT(3)},
                            {LIT(2), NOTLIT(3), LIT(0)},
                            {LIT(3), NOTLIT(0), LIT(1)},
                            {NOTLIT(0), NOTLIT(1), LIT(2)},
                            {NOTLIT(1), NOTLIT(2), LIT(3)},
                            {NOTLIT(2), NOTLIT(3), LIT(0)},
                            {NOTLIT(3), NOTLIT(0), LIT(1)}};

  /* Create environment */
  error = GRBloadenv(&env, "genconstr_c.log");
  if (error) goto QUIT;

  /* Create initial model */
  error = GRBnewmodel(env, &model, "genconstr_c", NVARS, NULL,
                      NULL, NULL, NULL, NULL);
  if (error) goto QUIT;

  /* Initialize decision variables and objective */
  for (i = 0; i < NLITERALS; i++) {
    col = LIT(i);
    sprintf(buffer, "X%d", i);
    error = GRBsetcharattrelement(model, "VType", col, GRB_BINARY);
    if (error) goto QUIT;

    error = GRBsetstrattrelement(model, "VarName", col, buffer);
    if (error) goto QUIT;

    col = NOTLIT(i);
    sprintf(buffer, "notX%d", i);
    error = GRBsetcharattrelement(model, "VType", col, GRB_BINARY);
    if (error) goto QUIT;

    error = GRBsetstrattrelement(model, "VarName", col, buffer);
    if (error) goto QUIT;
  }

  for (i = 0; i < NCLAUSES; i++) {
    col = CLA(i);
    sprintf(buffer, "Clause%d", i);
    error = GRBsetcharattrelement(model, "VType", col, GRB_BINARY);
    if (error) goto QUIT;

    error = GRBsetstrattrelement(model, "VarName", col, buffer);
    if (error) goto QUIT;
  }

  for (i = 0; i < NOBJ; i++) {
    col = OBJ(i);
    sprintf(buffer, "Obj%d", i);
    error = GRBsetcharattrelement(model, "VType", col, GRB_BINARY);
    if (error) goto QUIT;

    error = GRBsetstrattrelement(model, "VarName", col, buffer);
    if (error) goto QUIT;

    error = GRBsetdblattrelement(model, "Obj", col, 1.0);
    if (error) goto QUIT;
  }

  /* Link Xi and notXi */
  for (i = 0; i < NLITERALS; i++) {
    sprintf(buffer,"CNSTR_X%d",i);
    cind[0] = LIT(i);
    cind[1] = NOTLIT(i);
    cval[0] = cval[1] = 1;
    error = GRBaddconstr(model, 2, cind, cval, GRB_EQUAL, 1.0, buffer);
    if (error) goto QUIT;
  }

  /* Link clauses and literals */
  for (i = 0; i < NCLAUSES; i++) {
    sprintf(buffer,"CNSTR_Clause%d",i);
    error = GRBaddgenconstrOr(model, buffer, CLA(i), 3, Clauses[i]);
    if (error) goto QUIT;
  }

  /* Link objs with clauses */
  for (i = 0; i < NCLAUSES; i++) {
    cind[i] = CLA(i);
    cval[i] = 1;
  }
  error = GRBaddgenconstrMin(model, "CNSTR_Obj0", OBJ(0), NCLAUSES, cind, GRB_INFINITY);
  if (error) goto QUIT;

  /* note that passing 4 instead of 4.0 will produce undefined behavior */
  error = GRBaddgenconstrIndicator(model, "CNSTR_Obj1",
                                   OBJ(1), 1, NCLAUSES, cind, cval,
                                   GRB_GREATER_EQUAL, 4.0);
  if (error) goto QUIT;

  /* Set global objective sense */
  error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
  if (error) goto QUIT;

  /* Save problem */
  error = GRBwrite(model, "genconstr_c.mps");
  if (error) goto QUIT;

  error = GRBwrite(model, "genconstr_c.lp");
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

  /* Print result */
  error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
  if (error) goto QUIT;

  if (objval > 1.9)
    printf("Logical expression is satisfiable\n");
  else if (objval > 0.9)
    printf("At least four clauses can be satisfied\n");
  else
    printf("At most three clauses may be satisfied\n");

QUIT:

  if (model != NULL) GRBfreemodel(model);
  if (env != NULL)   GRBfreeenv(env);

  return error;
}
