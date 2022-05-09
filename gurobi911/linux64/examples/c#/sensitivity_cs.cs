// Copyright 2020, Gurobi Optimization, LLC

// A simple sensitivity analysis example which reads a MIP model from a
// file and solves it. Then uses the scenario feature to analyze the impact
// w.r.t. the objective function of each binary variable if it is set to
// 1-X, where X is its value in the optimal solution.
//
// Usage:
//     sensitivity_cs <model filename>

using System;
using Gurobi;

class sensitivity_cs
{
  // Maximum number of scenarios to be considered
  public const int MAXSCENARIOS = 100;

  static void Main(string[] args)
  {
    const int maxscenarios = sensitivity_cs.MAXSCENARIOS;

    if (args.Length < 1) {
      Console.Out.WriteLine("Usage: sensitivity_cs filename");
      return;
    }

    try {

      // Create environment
      GRBEnv env = new GRBEnv();

      // Read model
      GRBModel model = new GRBModel(env, args[0]);

      int scenarios;

      if (model.IsMIP == 0) {
        Console.WriteLine("Model is not a MIP");
        return;
      }

      // Solve model
      model.Optimize();

      if (model.Status != GRB.Status.OPTIMAL) {
        Console.WriteLine("Optimization ended with status " + model.Status);
        return;
      }

      // Store the optimal solution
      double   origObjVal = model.ObjVal;
      GRBVar[] vars       = model.GetVars();
      double[] origX      = model.Get(GRB.DoubleAttr.X, vars);

      scenarios = 0;

      // Count number of unfixed, binary variables in model. For each we
      // create a scenario.
      for (int i = 0; i < vars.Length; i++) {
        GRBVar v     = vars[i];
        char   vType = v.VType;

        if (v.LB == 0.0 && v.UB == 1.0                    &&
            (vType == GRB.BINARY || vType == GRB.INTEGER)   ) {
          scenarios++;

          if (scenarios >= maxscenarios)
            break;
        }
      }

      Console.WriteLine("###  construct multi-scenario model with "
                        + scenarios + " scenarios");

      // Set the number of scenarios in the model */
      model.NumScenarios = scenarios;

      scenarios = 0;

      // Create a (single) scenario model by iterating through unfixed
      // binary variables in the model and create for each of these
      // variables a scenario by fixing the variable to 1-X, where X is its
      // value in the computed optimal solution
      for (int i = 0; i < vars.Length; i++) {
        GRBVar v     = vars[i];
        char   vType = v.VType;

        if (v.LB == 0.0 && v.UB == 1.0                    &&
            (vType == GRB.BINARY || vType == GRB.INTEGER) &&
            scenarios < maxscenarios                        ) {

          // Set ScenarioNumber parameter to select the corresponding
          // scenario for adjustments
          model.Parameters.ScenarioNumber = scenarios;

          // Set variable to 1-X, where X is its value in the optimal solution */
          if (origX[i] < 0.5)
            v.ScenNLB = 1.0;
          else
            v.ScenNUB = 0.0;

          scenarios++;
        } else {
          // Add MIP start for all other variables using the optimal solution
          // of the base model
          v.Start = origX[i];
        }
      }

      // Solve multi-scenario model
      model.Optimize();

      // In case we solved the scenario model to optimality capture the
      // sensitivity information
      if (model.Status == GRB.Status.OPTIMAL) {

        // get the model sense (minimization or maximization)
        int modelSense = model.ModelSense;

        scenarios = 0;

        for (int i = 0; i < vars.Length; i++) {
          GRBVar v     = vars[i];
          char   vType = v.VType;

          if (v.LB == 0.0 && v.UB == 1.0                    &&
              (vType == GRB.BINARY || vType == GRB.INTEGER)   ) {

            // Set scenario parameter to collect the objective value of the
            // corresponding scenario
            model.Parameters.ScenarioNumber = scenarios;

            double scenarioObjVal = model.ScenNObjVal;
            double scenarioObjBound = model.ScenNObjBound;

            Console.Write("Objective sensitivity for variable "
                          + v.VarName + " is ");

            // Check if we found a feasible solution for this scenario
            if (scenarioObjVal >= modelSense * GRB.INFINITY) {
              // Check if the scenario is infeasible
              if (scenarioObjBound >= modelSense * GRB.INFINITY)
                Console.WriteLine("infeasible");
              else
                Console.WriteLine("unknown (no solution available)");
            } else {
              // Scenario is feasible and a solution is available
              Console.WriteLine(modelSense * (scenarioObjVal - origObjVal));
            }

            scenarios++;

            if (scenarios >= maxscenarios)
              break;
          }
        }
      }

      // Dispose of model and environment
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode);
      Console.WriteLine(e.Message);
      Console.WriteLine(e.StackTrace);
    }
  }
}
