/* Copyright 2020, Gurobi Optimization, LLC */

/* Facility location: a company currently ships its product from 5 plants
   to 4 warehouses. It is considering closing some plants to reduce
   costs. What plant(s) should the company close, in order to minimize
   transportation and fixed costs?

   Since the plant fixed costs and the warehouse demands are uncertain, a
   scenario approach is chosen.

   Note that this example is similar to the facility_c.c example. Here we
   added scenarios in order to illustrate the multi-scenario feature.

   Based on an example from Frontline Systems:
   http://www.solver.com/disfacility.htm
   Used with permission.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gurobi_c.h"


#define opencol(p)         p
#define transportcol(w,p)  nPlants*(w+1)+p
#define demandconstr(w)    nPlants+w
#define MAXSTR             128

int
main(int   argc,
     char *argv[])
{
  GRBenv   *env      = NULL;
  GRBenv   *modelenv = NULL;
  GRBmodel *model    = NULL;

  double  *cval         = NULL;
  double  *rhs          = NULL;
  int     *cbeg         = NULL;
  int     *cind         = NULL;
  char   **cname        = NULL;
  char    *sense        = NULL;

  double maxFixed = -GRB_INFINITY;
  double minFixed = GRB_INFINITY;
  int    cnamect  = 0;
  int    error    = 0;

  int    p, s, w, col;
  int    idx, rowct;
  int    nScenarios;
  char   vname[MAXSTR];

  /* Number of plants, warehouses and scenarios */
  const int nPlants     = 5;
  const int nWarehouses = 4;

  /* Warehouse demand in thousands of units */
  double Demand[] = { 15, 18, 14, 20 };

  /* Plant capacity in thousands of units */
  double Capacity[] = { 20, 22, 17, 19, 18 };

  /* Fixed costs for each plant */
  double FixedCosts[] =
    { 12000, 15000, 17000, 13000, 16000 };

  /* Transportation costs per thousand units */
  double TransCosts[4][5] = {
    { 4000, 2000, 3000, 2500, 4500 },
    { 2500, 2600, 3400, 3000, 4000 },
    { 1200, 1800, 2600, 4100, 3000 },
    { 2200, 2600, 3100, 3700, 3200 }
  };

  /* Compute minimal and maximal fixed cost */
  for (p = 0; p < nPlants; p++) {
    if (FixedCosts[p] > maxFixed)
      maxFixed = FixedCosts[p];

    if (FixedCosts[p] < minFixed)
      minFixed = FixedCosts[p];
  }

  /* Create environment */
  error = GRBloadenv(&env, "multiscenario.log");
  if (error) goto QUIT;

  /* Create initial model */
  error = GRBnewmodel(env, &model, "multiscenario", nPlants * (nWarehouses + 1),
                      NULL, NULL, NULL, NULL, NULL);
  if (error) goto QUIT;

  modelenv = GRBgetenv(model);

  /* Initialize decision variables for plant open variables */
  for (p = 0; p < nPlants; p++) {
    col = opencol(p);
    error = GRBsetcharattrelement(model, GRB_CHAR_ATTR_VTYPE,
                                  col, GRB_BINARY);
    if (error) goto QUIT;
    error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ,
                                 col, FixedCosts[p]);
    if (error) goto QUIT;
    sprintf(vname, "Open%i", p);
    error = GRBsetstrattrelement(model, GRB_STR_ATTR_VARNAME,
                                 col, vname);
    if (error) goto QUIT;
  }

  /* Initialize decision variables for transportation decision variables:
     how much to transport from a plant p to a warehouse w */
  for (w = 0; w < nWarehouses; w++) {
    for (p = 0; p < nPlants; p++) {
      col = transportcol(w, p);
      error = GRBsetdblattrelement(model, GRB_DBL_ATTR_OBJ,
                                   col, TransCosts[w][p]);
      if (error) goto QUIT;
      sprintf(vname, "Trans%i.%i", p, w);
      error = GRBsetstrattrelement(model, GRB_STR_ATTR_VARNAME,
                                   col, vname);
      if (error) goto QUIT;
    }
  }

  /* The objective is to minimize the total fixed and variable costs */
  error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MINIMIZE);
  if (error) goto QUIT;

  /* Make space for constraint data */
  rowct = (nPlants > nWarehouses) ? nPlants : nWarehouses;
  cbeg = malloc(sizeof(int) * rowct);
  if (!cbeg) goto QUIT;
  cind = malloc(sizeof(int) * (nPlants * (nWarehouses + 1)));
  if (!cind) goto QUIT;
  cval = malloc(sizeof(double) * (nPlants * (nWarehouses + 1)));
  if (!cval) goto QUIT;
  rhs = malloc(sizeof(double) * rowct);
  if (!rhs) goto QUIT;
  sense = malloc(sizeof(char) * rowct);
  if (!sense) goto QUIT;
  cname = calloc(rowct, sizeof(char*));
  if (!cname) goto QUIT;

  /* Production constraints
     Note that the limit sets the production to zero if
     the plant is closed */
  idx = 0;
  for (p = 0; p < nPlants; p++) {
    cbeg[p] = idx;
    rhs[p] = 0.0;
    sense[p] = GRB_LESS_EQUAL;
    cname[p] = malloc(sizeof(char) * MAXSTR);
    if (!cname[p]) goto QUIT;
    cnamect++;
    sprintf(cname[p], "Capacity%i", p);
    for (w = 0; w < nWarehouses; w++) {
      cind[idx] = transportcol(w, p);
      cval[idx++] = 1.0;
    }
    cind[idx] = opencol(p);
    cval[idx++] = -Capacity[p];
  }
  error = GRBaddconstrs(model, nPlants, idx, cbeg, cind, cval, sense,
                        rhs, cname);
  if (error) goto QUIT;

  /* Demand constraints */
  idx = 0;
  for (w = 0; w < nWarehouses; w++) {
    cbeg[w] = idx;
    sense[w] = GRB_EQUAL;
    sprintf(cname[w], "Demand%i", w);
    for (p = 0; p < nPlants; p++) {
      cind[idx] = transportcol(w, p);
      cval[idx++] = 1.0;
    }
  }
  error = GRBaddconstrs(model, nWarehouses, idx, cbeg, cind, cval, sense,
                        Demand, cname);
  if (error) goto QUIT;

  /* We constructed the base model, now we add 7 scenarios

     Scenario 0: Represents the base model, hence, no manipulations.
     Scenario 1: Manipulate the warehouses demands slightly (constraint right
                 hand sides).
     Scenario 2: Double the warehouses demands (constraint right hand sides).
     Scenario 3: Manipulate the plant fixed costs (objective coefficients).
     Scenario 4: Manipulate the warehouses demands and fixed costs.
     Scenario 5: Force the plant with the largest fixed cost to stay open
                 (variable bounds).
     Scenario 6: Force the plant with the smallest fixed cost to be closed
                 (variable bounds). */

  error = GRBsetintattr(model, GRB_INT_ATTR_NUMSCENARIOS, 7);
  if (error) goto QUIT;

  /* Scenario 0: Base model, hence, nothing to do except giving the
                 scenario a name */
  error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, 0);
  if (error) goto QUIT;
  error = GRBsetstrattr(model, GRB_STR_ATTR_SCENNNAME, "Base model");
  if (error) goto QUIT;

  /* Scenario 1: Increase the warehouse demands by 10% */
  error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, 1);
  if (error) goto QUIT;
  error = GRBsetstrattr(model, GRB_STR_ATTR_SCENNNAME,
                        "Increased warehouse demands");
  if (error) goto QUIT;

  for (w = 0; w < nWarehouses; w++) {
    error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNRHS,
                                 demandconstr(w), Demand[w] * 1.1);
    if (error) goto QUIT;
  }

  /* Scenario 2: Double the warehouse demands */
  error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, 2);
  if (error) goto QUIT;
  error = GRBsetstrattr(model, GRB_STR_ATTR_SCENNNAME,
                        "Double the warehouse demands");
  if (error) goto QUIT;

  for (w = 0; w < nWarehouses; w++) {
    error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNRHS,
                                 demandconstr(w), Demand[w] * 2.0);
    if (error) goto QUIT;
  }

  /* Scenario 3: Decrease the plant fixed costs by 5% */
  error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, 3);
  if (error) goto QUIT;
  error = GRBsetstrattr(model, GRB_STR_ATTR_SCENNNAME,
                        "Decreased plant fixed costs");
  if (error) goto QUIT;

  for (p  = 0; p < nPlants; p++) {
    error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNOBJ,
                                 opencol(p), FixedCosts[p] * 0.95);
    if (error) goto QUIT;
  }

  /* Scenario 4: Combine scenario 1 and scenario 3 */
  error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, 4);
  if (error) goto QUIT;
  error = GRBsetstrattr(model, GRB_STR_ATTR_SCENNNAME,
                        "Increased warehouse demands and decreased plant fixed costs");

  for (w = 0; w < nWarehouses; w++) {
    error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNRHS,
                                 demandconstr(w), Demand[w] * 1.1);
    if (error) goto QUIT;
  }
  for (p  = 0; p < nPlants; p++) {
    error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNOBJ,
                                 opencol(p), FixedCosts[p] * 0.95);
    if (error) goto QUIT;
  }

  /* Scenario 5: Force the plant with the largest fixed cost to stay
                 open */
  error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, 5);
  if (error) goto QUIT;
  error = GRBsetstrattr(model, GRB_STR_ATTR_SCENNNAME,
                        "Force plant with largest fixed cost to stay open");
  if (error) goto QUIT;

  for (p  = 0; p < nPlants; p++) {
    if (FixedCosts[p] == maxFixed) {
      error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNLB,
                                   opencol(p), 1.0);
      if (error) goto QUIT;
      break;
    }
  }

  /* Scenario 6: Force the plant with the smallest fixed cost to be
                 closed */
  error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, 6);
  if (error) goto QUIT;
  error = GRBsetstrattr(model, GRB_STR_ATTR_SCENNNAME,
                        "Force plant with smallest fixed cost to be closed");
  if (error) goto QUIT;

  for (p  = 0; p < nPlants; p++) {
    if (FixedCosts[p] == minFixed) {
      error = GRBsetdblattrelement(model, GRB_DBL_ATTR_SCENNUB,
                                   opencol(p), 0.0);
      if (error) goto QUIT;
      break;
    }
  }

  /* Guess at the starting point: close the plant with the highest
     fixed costs; open all others */

  /* First, open all plants */
  for (p = 0; p < nPlants; p++) {
    error = GRBsetdblattrelement(model, GRB_DBL_ATTR_START, opencol(p), 1.0);
    if (error) goto QUIT;
  }

  /* Now close the plant with the highest fixed cost */
  printf("Initial guess:\n");
  for (p = 0; p < nPlants; p++) {
    if (FixedCosts[p] == maxFixed) {
      error = GRBsetdblattrelement(model, GRB_DBL_ATTR_START, opencol(p), 0.0);
      if (error) goto QUIT;
      printf("Closing plant %i\n\n", p);
      break;
    }
  }

  /* Use barrier to solve root relaxation */
  error = GRBsetintparam(modelenv,
                         GRB_INT_PAR_METHOD,
                         GRB_METHOD_BARRIER);
  if (error) goto QUIT;

  /* Solve multi-scenario model */
  error = GRBoptimize(model);
  if (error) goto QUIT;

  error = GRBgetintattr(model, GRB_INT_ATTR_NUMSCENARIOS, &nScenarios);
  if (error) goto QUIT;

  /* Print solution for each */
  for (s = 0; s < nScenarios; s++) {
    char   *scenarioName;
    double  scenNObjBound;
    double  scenNObjVal;
    int     modelSense = GRB_MINIMIZE;

    /* Set the scenario number to query the information for this
       scenario */
    error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, s);
    if (error) goto QUIT;

    /* Collect result for the scenario */
    error = GRBgetstrattr(model, GRB_STR_ATTR_SCENNNAME, &scenarioName);
    if (error) goto QUIT;
    error = GRBgetdblattr(model, GRB_DBL_ATTR_SCENNOBJBOUND, &scenNObjBound);
    if (error) goto QUIT;
    error = GRBgetdblattr(model, GRB_DBL_ATTR_SCENNOBJVAL, &scenNObjVal);
    if (error) goto QUIT;

    printf("\n\n------ Scenario %d (%s)\n", s, scenarioName);

    /* Check if we found a feasible solution for this scenario */
    if (scenNObjVal >= modelSense * GRB_INFINITY)
      if (scenNObjBound >= modelSense * GRB_INFINITY)
        /* Scenario was proven to be infeasible */
        printf("\nINFEASIBLE\n");
      else
        /* We did not find any feasible solution - should not happen in
           this case, because we did not set any limit (like a time
           limit) on the optimization process */
        printf("\nNO SOLUTION\n");
    else {
      printf("\nTOTAL COSTS: %g\n", scenNObjVal);
      printf("SOLUTION:\n");
      for (p = 0; p < nPlants; p++) {
        double scenNX;

        error = GRBgetdblattrelement(model, GRB_DBL_ATTR_SCENNX,
                                     opencol(p), &scenNX);
        if (error) goto QUIT;

        if (scenNX > 0.5) {
          printf("Plant %i open\n", p);
          for (w = 0; w < nWarehouses; w++) {
            error = GRBgetdblattrelement(model, GRB_DBL_ATTR_SCENNX,
                                         transportcol(w, p), &scenNX);
            if (error) goto QUIT;
            if (scenNX > 0.0001)
              printf("  Transport %g units to warehouse %i\n",
                     scenNX, w);
          }
        } else
          printf("Plant %i closed!\n",  p);
      }
    }
  }

  /* Print a summary table: for each scenario we add a single summary
     line */
  printf("\n\nSummary: Closed plants depending on scenario\n\n");
  printf("%8s | %17s %13s\n", "", "Plant", "|");

  printf("%8s |", "Scenario");
  for (p = 0; p < nPlants; p++)
    printf(" %5d", p);
  printf(" | %6s  %s\n", "Costs", "Name");

  for (s = 0; s < nScenarios; s++) {
    char   *scenarioName;
    double  scenNObjBound;
    double  scenNObjVal;
    int     modelSense = GRB_MINIMIZE;

    /* Set the scenario number to query the information for this scenario */
    error = GRBsetintparam(modelenv, GRB_INT_PAR_SCENARIONUMBER, s);
    if (error) goto QUIT;

    /* collect result for the scenario */
    error = GRBgetstrattr(model, GRB_STR_ATTR_SCENNNAME, &scenarioName);
    if (error) goto QUIT;
    error = GRBgetdblattr(model, GRB_DBL_ATTR_SCENNOBJBOUND, &scenNObjBound);
    if (error) goto QUIT;
    error = GRBgetdblattr(model, GRB_DBL_ATTR_SCENNOBJVAL, &scenNObjVal);
    if (error) goto QUIT;

    printf("%-8d |", s);

    /* Check if we found a feasible solution for this scenario */
    if (scenNObjVal >= modelSense * GRB_INFINITY)
      if (scenNObjBound >= modelSense * GRB_INFINITY)
        /* Scenario was proven to be infeasible */
        printf(" %-30s| %6s  %s\n", "infeasible", "-", scenarioName);
      else
        /* We did not find any feasible solution - should not happen in
           this case, because we did not set any limit (like a time
           limit) on the optimization process */
        printf(" %-30s| %6s  %s\n", "no solution found", "-", scenarioName);
    else {
      for (p = 0; p < nPlants; p++) {
        double scenNX;

        error = GRBgetdblattrelement(model, GRB_DBL_ATTR_SCENNX,
                                     opencol(p), &scenNX);
        if (scenNX  > 0.5)
          printf(" %5s", " ");
        else
          printf(" %5s", "x");
      }

      printf(" | %6g  %s\n", scenNObjVal, scenarioName);
    }
  }


QUIT:

  /* Error reporting */
  if (error) {
    printf("ERROR: %s\n", GRBgeterrormsg(env));
    exit(1);
  }

  /* Free data */
  free(cbeg);
  free(cind);
  free(cval);
  free(rhs);
  free(sense);
  for (p = 0; p < cnamect; p++)
    free(cname[p]);
  free(cname);

  /* Free model */
  GRBfreemodel(model);

  /* Free environment */
  GRBfreeenv(env);

  return 0;
}
