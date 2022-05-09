// Copyright 2020, Gurobi Optimization, LLC

// A simple sensitivity analysis example which reads a MIP model from a
// file and solves it. Then uses the scenario feature to analyze the impact
// w.r.t. the objective function of each binary variable if it is set to
// 1-X, where X is its value in the optimal solution.
//
// Usage:
//     java Sensitivity <model filename>

import gurobi.*;

public class Sensitivity {

    // Maximum number of scenarios to be considered
    private static final int MAXSCENARIOS = 100;

    public static void main(String[] args) {

    if (args.length < 1) {
      System.out.println("Usage: java Sensitivity filename");
      System.exit(1);
    }

    try {

      // Create environment
      GRBEnv env = new GRBEnv();

      // Read model
      GRBModel model = new GRBModel(env, args[0]);

      int scenarios;

      if (model.get(GRB.IntAttr.IsMIP) == 0) {
        System.out.println("Model is not a MIP");
        System.exit(1);
      }

      // Solve model
      model.optimize();

      if (model.get(GRB.IntAttr.Status) != GRB.OPTIMAL) {
        System.out.println("Optimization ended with status "
                           + model.get(GRB.IntAttr.Status));
        System.exit(1);
      }

      // Store the optimal solution
      double   origObjVal = model.get(GRB.DoubleAttr.ObjVal);
      GRBVar[] vars       = model.getVars();
      double[] origX      = model.get(GRB.DoubleAttr.X, vars);

      scenarios = 0;

      // Count number of unfixed, binary variables in model. For each we
      // create a scenario.
      for (int i = 0; i < vars.length; i++) {
        GRBVar v     = vars[i];
        char   vType = v.get(GRB.CharAttr.VType);

        if (v.get(GRB.DoubleAttr.LB) == 0                 &&
            v.get(GRB.DoubleAttr.UB) == 1                 &&
            (vType == GRB.BINARY || vType == GRB.INTEGER)   ) {

          scenarios++;

          if (scenarios >= MAXSCENARIOS)
            break;
        }
      }

      System.out.println("###  construct multi-scenario model with "
                         + scenarios + " scenarios");

      // Set the number of scenarios in the model */
      model.set(GRB.IntAttr.NumScenarios, scenarios);

      scenarios = 0;

      // Create a (single) scenario model by iterating through unfixed
      // binary variables in the model and create for each of these
      // variables a scenario by fixing the variable to 1-X, where X is its
      // value in the computed optimal solution
      for (int i = 0; i < vars.length; i++) {
        GRBVar v     = vars[i];
        char   vType = v.get(GRB.CharAttr.VType);

        if (v.get(GRB.DoubleAttr.LB) == 0                 &&
            v.get(GRB.DoubleAttr.UB) == 1                 &&
            (vType == GRB.BINARY || vType == GRB.INTEGER) &&
            scenarios < MAXSCENARIOS                        ) {

          // Set ScenarioNumber parameter to select the corresponding
          // scenario for adjustments
          model.set(GRB.IntParam.ScenarioNumber, scenarios);

          // Set variable to 1-X, where X is its value in the optimal solution */
          if (origX[i] < 0.5)
            v.set(GRB.DoubleAttr.ScenNLB, 1.0);
          else
            v.set(GRB.DoubleAttr.ScenNUB, 0.0);

          scenarios++;
        } else {
          // Add MIP start for all other variables using the optimal
          // solution of the base model
          v.set(GRB.DoubleAttr.Start, origX[i]);
        }
      }

      // Solve multi-scenario model
      model.optimize();

      // In case we solved the scenario model to optimality capture the
      // sensitivity information
      if (model.get(GRB.IntAttr.Status) == GRB.OPTIMAL) {

        // get the model sense (minimization or maximization)
        int modelSense = model.get(GRB.IntAttr.ModelSense);

        scenarios = 0;

        for (int i = 0; i < vars.length; i++) {
          GRBVar v     = vars[i];
          char   vType = v.get(GRB.CharAttr.VType);

          if (v.get(GRB.DoubleAttr.LB) == 0                 &&
              v.get(GRB.DoubleAttr.UB) == 1                 &&
              (vType == GRB.BINARY || vType == GRB.INTEGER)   ) {

            // Set scenario parameter to collect the objective value of the
            // corresponding scenario
            model.set(GRB.IntParam.ScenarioNumber, scenarios);

            // Collect objective value and bound for the scenario
            double scenarioObjVal = model.get(GRB.DoubleAttr.ScenNObjVal);
            double scenarioObjBound = model.get(GRB.DoubleAttr.ScenNObjBound);

            System.out.print("Objective sensitivity for variable "
                             + v.get(GRB.StringAttr.VarName) + " is ");

            // Check if we found a feasible solution for this scenario
            if (scenarioObjVal >= modelSense * GRB.INFINITY) {
              // Check if the scenario is infeasible
              if (scenarioObjBound >= modelSense * GRB.INFINITY)
                System.out.println("infeasible");
              else
                System.out.println("unknown (no solution available)");
            } else {
              // Scenario is feasible and a solution is available
              System.out.println("" + modelSense * (scenarioObjVal - origObjVal));
            }

            scenarios++;

            if (scenarios >= MAXSCENARIOS)
              break;
          }
        }
      }

      // Dispose of model and environment
      model.dispose();
      env.dispose();

    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode());
      System.out.println(e.getMessage());
      e.printStackTrace();
    }
  }
}
