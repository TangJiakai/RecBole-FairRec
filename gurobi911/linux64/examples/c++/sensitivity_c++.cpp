// Copyright 2020, Gurobi Optimization, LLC

// A simple sensitivity analysis example which reads a MIP model from a
// file and solves it. Then uses the scenario feature to analyze the impact
// w.r.t. the objective function of each binary variable if it is set to
// 1-X, where X is its value in the optimal solution.
//
// Usage:
//     sensitivity_c++ <model filename>

#include "gurobi_c++.h"
using namespace std;

// Maximum number of scenarios to be considered
#define MAXSCENARIOS 100

int
main(int   argc,
     char *argv[])
{
  if (argc < 2) {
    cout << "Usage: sensitivity_c++ filename" << endl;
    return 1;
  }

  GRBVar *vars  = NULL;
  double *origX = NULL;

  try {

    // Create environment
    GRBEnv env = GRBEnv();

    // Read model
    GRBModel model = GRBModel(env, argv[1]);

    int scenarios;

    if (model.get(GRB_IntAttr_IsMIP) == 0) {
      cout << "Model is not a MIP" << endl;
      return 1;
    }

    // Solve model
    model.optimize();

    if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
      cout << "Optimization ended with status "
           << model.get(GRB_IntAttr_Status) << endl;
      return 1;
    }

    // Store the optimal solution
    double origObjVal = model.get(GRB_DoubleAttr_ObjVal);
    vars = model.getVars();
    int numVars = model.get(GRB_IntAttr_NumVars);
    origX = model.get(GRB_DoubleAttr_X, vars, numVars);

    scenarios = 0;

    // Count number of unfixed, binary variables in model. For each we
    // create a scenario.
    for (int i = 0; i < numVars; i++) {
      GRBVar v     = vars[i];
      char   vType = v.get(GRB_CharAttr_VType);

      if (v.get(GRB_DoubleAttr_LB) == 0.0               &&
          v.get(GRB_DoubleAttr_UB) == 1.0               &&
          (vType == GRB_BINARY || vType == GRB_INTEGER)   ) {
        scenarios++;

        if (scenarios >= MAXSCENARIOS)
          break;
      }
    }

    cout << "###  construct multi-scenario model with "
         << scenarios << " scenarios" << endl;

    // Set the number of scenarios in the model */
    model.set(GRB_IntAttr_NumScenarios, scenarios);

    scenarios = 0;

    // Create a (single) scenario model by iterating through unfixed binary
    // variables in the model and create for each of these variables a
    // scenario by fixing the variable to 1-X, where X is its value in the
    // computed optimal solution
    for (int i = 0; i < numVars; i++) {
      GRBVar v     = vars[i];
      char   vType = v.get(GRB_CharAttr_VType);

      if (v.get(GRB_DoubleAttr_LB) == 0.0               &&
          v.get(GRB_DoubleAttr_UB) == 1-0               &&
          (vType == GRB_BINARY || vType == GRB_INTEGER) &&
          scenarios < MAXSCENARIOS                        ) {

        // Set ScenarioNumber parameter to select the corresponding
        // scenario for adjustments
        model.set(GRB_IntParam_ScenarioNumber, scenarios);

        // Set variable to 1-X, where X is its value in the optimal solution */
        if (origX[i] < 0.5)
          v.set(GRB_DoubleAttr_ScenNLB, 1.0);
        else
          v.set(GRB_DoubleAttr_ScenNUB, 0.0);

        scenarios++;
      } else {
        // Add MIP start for all other variables using the optimal solution
        // of the base model
        v.set(GRB_DoubleAttr_Start, origX[i]);
      }
    }

    // Solve multi-scenario model
    model.optimize();

    // In case we solved the scenario model to optimality capture the
    // sensitivity information
    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {

      // get the model sense (minimization or maximization)
      int modelSense = model.get(GRB_IntAttr_ModelSense);

      scenarios = 0;

      for (int i = 0; i < numVars; i++) {
        GRBVar v     = vars[i];
        char   vType = v.get(GRB_CharAttr_VType);

        if (v.get(GRB_DoubleAttr_LB) == 0.0               &&
            v.get(GRB_DoubleAttr_UB) == 1-0               &&
            (vType == GRB_BINARY || vType == GRB_INTEGER)   ) {

          // Set scenario parameter to collect the objective value of the
          // corresponding scenario
          model.set(GRB_IntParam_ScenarioNumber, scenarios);

          // Collect objective value and bound for the scenario
          double scenarioObjVal = model.get(GRB_DoubleAttr_ScenNObjVal);
          double scenarioObjBound = model.get(GRB_DoubleAttr_ScenNObjBound);

          cout << "Objective sensitivity for variable "
               << v.get(GRB_StringAttr_VarName)
               << " is ";

          // Check if we found a feasible solution for this scenario
          if (scenarioObjVal >= modelSense * GRB_INFINITY) {
            // Check if the scenario is infeasible
            if (scenarioObjBound >= modelSense * GRB_INFINITY)
              cout << "infeasible"  << endl;
            else
              cout << "unknown (no solution available)"  << endl;
          } else {
            // Scenario is feasible and a solution is available
            cout << modelSense * (scenarioObjVal - origObjVal) << endl;
          }

          scenarios++;

          if (scenarios >= MAXSCENARIOS)
            break;
        }
      }
    }
  } catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch (...) {
    cout << "Error during optimization" << endl;
  }

  delete[] vars;
  delete[] origX;

  return 0;
}
