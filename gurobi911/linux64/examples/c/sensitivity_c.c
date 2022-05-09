/* Copyright 2020, Gurobi Optimization, LLC */

/* A simple sensitivity analysis example which reads a MIP model from a
 * file and solves it. Then uses the scenario feature to analyze the impact
 * w.r.t. the objective function of each binary variable if it is set to
 * 1-X, where X is its value in the optimal solution.
 *
 * Usage:
 *     sensitivity_c <model filename>
 */

#include <stdlib.h>
#include <stdio.h>
#include "gurobi_c.h"

/* Maximum number of scenarios to be considered */
#define MAXSCENARIOS 100

int
main(int   argc,
     char *argv[])
{
  GRBenv   *env      = NULL;
  GRBenv   *modelenv = NULL;
  GRBmodel *model    = NULL;

  double *origx = NULL;
  double  origObjVal;

  int ismip, status, numvars, i;
  int scenarios;
  int error = 0;

  if (argc < 2) {
    fprintf(stderr, "Usage: sensitivity_c filename\n");
    goto QUIT;
  }

  /* Create environment */
  error = GRBloadenv(&env, "sensitivity.log");
  if (error) goto QUIT;

  /* Read model */
  error = GRBreadmodel(env, argv[1], &model);
  if (error) goto QUIT;

  modelenv = GRBgetenv(model);
  if (error) goto QUIT;

  error = GRBgetintattr(model, GRB_INT_ATTR_IS_MIP, &ismip);
  if (error) goto QUIT;
  if (ismip == 0) {
    printf("Model is not a MIP\n");
    goto QUIT;
  }

  /* Solve model */
  error = GRBoptimize(model);
  if (error) goto QUIT;

  error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &status);
  if (error) goto QUIT;
  if (status != GRB_OPTIMAL) {
    printf("Optimization ended with status %d\n", status);
    goto QUIT;
  }

  /* Store the optimal solution */
  error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &origObjVal);
  if (error) goto QUIT;
  error = GRBgetintattr(model, GRB_INT_ATTR_NUMVARS, &numvars);
  if (error) goto QUIT;
  origx = (double *) malloc(numvars * sizeof(double));
  if (origx == NULL) {
    printf("Out of memory\n");
    goto QUIT;
  }
  error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, numvars, origx);
  if (error) goto QUIT;

  scenarios = 0;

  /* Count number of unfixed, binary variables in model. For each we create a
   * scenario.
   */
  for (i = 0; i < numvars; i++) {
    double lb, ub;
    char   vtype;

    error = GRBgetdblattrelement(model, GRB_DBL_ATTR_LB, i, &lb);
    if (error) goto QUIT;
    error = GRBgetdblattrelement(model, GRB_DBL_ATTR_UB, i, &ub);
    if (error) goto QUIT;
    error = GRBgetcharattrelement(model, GRB_CHAR_ATTR_VTYPE, i, &vtype);
    if (error) goto QUIT;

    if (lb == 0.0 && ub == 1.0                        &&
        (vtype == GRB_BINARY || vtype == GRB_INTEGER)   ) {
      scenarios++;

      if (scenarios >= MAXSCENARIOS)
        break;
    }
  }

  printf("###  construct multi-scenario model with %d scenarios\n", scenarios);

  /* Set the number of scenarios in the model */
  error = GRBsetintattr(model, GRB_INT_ATTR_NUMSCENARIOS, scenarios);
  if (error) goto QUIT;

  scenarios = 0;

  /* Create a (single) scenario model by iterating through unfixed binary
   * variables in the model and create for each of these variables a
   * scenario by fixing the variable to 1-X, where X is its value in the
   * computed optimal solution
   */
  for (i = 0; i < numvars; i++) {
    double lb, ub;
    char   vtype;

    error = GRBgetdblattrelement(model, GRB_DBL_ATTR_LB, i, &lb);
    if (error) goto QUIT;
    error = GRBgetdblattrelement(model, GRB_DBL_ATTR_UB, i, &ub);
    if (error) goto QUIT;
    error = GRBgetcharattrelement(model, GRB_CHAR_ATTR_VTYPE, i, &vtype);
    if (error) goto QUIT;

    if (lb == 0.0 && ub == 1.0                        &&
        (vtype == GRB_BINARY || vtype == GRB_INTEGER) &&
        scenarios < MAXSCENARIOS                        ) {

      /* Set ScenarioNumber parameter to select the corresponding scenario
       * for adjustments
       */
      error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, scenarios);
      if (error) goto QUIT;

      /* Set variable to 1-X, where X is its value in the optimal solution */
      if (origx[i] < 0.5) {
        error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNLB, i, 1.0);
        if (error) goto QUIT;
      } else {
        error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNUB, i, 0.0);
        if (error) goto QUIT;
      }


      scenarios++;
    } else {
      /* Add MIP start for all other variables using the optimal solution
       * of the base model
       */
      error = GRBsetdblattrelement(model, GRB_DBL_ATTR_START, i, origx[i]);
      if (error) goto QUIT;
    }
  }

  /* Solve multi-scenario model */
  error = GRBoptimize(model);
  if (error) goto QUIT;

  /* Collect the status */
  error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &status);
  if (error) goto QUIT;

  /* In case we solved the scenario model to optimality capture the
   * sensitivity information
   */
  if (status == GRB_OPTIMAL) {
    int modelSense;

    scenarios = 0;

    /* Get model sense (minimization or maximization) */
    error = GRBgetintattr(model, GRB_INT_ATTR_MODELSENSE, &modelSense);
    if (error) goto QUIT;

    for (i = 0; i < numvars; i++) {
      double lb, ub;
      char   vtype;

      error = GRBgetdblattrelement(model, GRB_DBL_ATTR_LB, i, &lb);
      if (error) goto QUIT;
      error = GRBgetdblattrelement(model, GRB_DBL_ATTR_UB, i, &ub);
      if (error) goto QUIT;
      error = GRBgetcharattrelement(model, GRB_CHAR_ATTR_VTYPE, i, &vtype);
      if (error) goto QUIT;

      if (lb == 0.0 && ub == 1.0                        &&
          (vtype == GRB_BINARY || vtype == GRB_INTEGER)   ) {

        double scenarioObjVal;
        double scenarioObjBound;
        char  *varName;

        /* Set scenario parameter to collect the objective value of the
         * corresponding scenario
         */
        error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, scenarios);
        if (error) goto QUIT;

        /* Collect objective value and bound for the scenario */
        error = GRBgetdblattr(model, GRB_DBL_ATTR_SCENNOBJVAL, &scenarioObjVal);
        if (error) goto QUIT;
        error = GRBgetdblattr(model, GRB_DBL_ATTR_SCENNOBJBOUND, &scenarioObjBound);
        if (error) goto QUIT;

        error = GRBgetstrattrelement(model, GRB_STR_ATTR_VARNAME, i, &varName);
        if (error) goto QUIT;

        /* Check if we found a feasible solution for this scenario */
        if (scenarioObjVal >= modelSense * GRB_INFINITY) {
          /* Check if the scenario is infeasible */
          if (scenarioObjBound >= modelSense * GRB_INFINITY)
            printf("Objective sensitivity for variable %s is infeasible\n",
                   varName);
          else
            printf("Objective sensitivity for variable %s is unknown (no solution available)\n",
                   varName);
        } else {
          /* Scenario is feasible and a solution is available */
          printf("Objective sensitivity for variable %s is %g\n",
                 varName, modelSense * (scenarioObjVal - origObjVal));
        }

        scenarios++;

        if (scenarios >= MAXSCENARIOS)
          break;
      }
    }
  }

QUIT:

  /* Error reporting */
  if (error != 0) {
    printf("ERROR: %s\n", GRBgeterrormsg(env));
    exit(1);
  }

  /* Free data */
  free(origx);

  /* Free model */
  GRBfreemodel(model);

  /* Free environment */
  GRBfreeenv(env);

  return 0;
}
