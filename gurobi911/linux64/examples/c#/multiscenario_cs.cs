// Copyright 2020, Gurobi Optimization, LLC

// Facility location: a company currently ships its product from 5 plants
// to 4 warehouses. It is considering closing some plants to reduce
// costs. What plant(s) should the company close, in order to minimize
// transportation and fixed costs?
//
// Since the plant fixed costs and the warehouse demands are uncertain, a
// scenario approach is chosen.
//
// Note that this example is similar to the facility_cs.cs example. Here we
// added scenarios in order to illustrate the multi-scenario feature.
//
// Based on an example from Frontline Systems:
// http://www.solver.com/disfacility.htm
// Used with permission.

using System;
using Gurobi;

class multiscenario_cs
{
  static void Main()
  {
    try {

      // Warehouse demand in thousands of units
      double[] Demand = new double[] { 15, 18, 14, 20 };

      // Plant capacity in thousands of units
      double[] Capacity = new double[] { 20, 22, 17, 19, 18 };

      // Fixed costs for each plant
      double[] FixedCosts =
        new double[] { 12000, 15000, 17000, 13000, 16000 };

      // Transportation costs per thousand units
      double[,] TransCosts =
        new double[,] { { 4000, 2000, 3000, 2500, 4500 },
                        { 2500, 2600, 3400, 3000, 4000 },
                        { 1200, 1800, 2600, 4100, 3000 },
                        { 2200, 2600, 3100, 3700, 3200 } };

      // Number of plants and warehouses
      int nPlants = Capacity.Length;
      int nWarehouses = Demand.Length;

      double maxFixed = -GRB.INFINITY;
      double minFixed = GRB.INFINITY;
      for (int p = 0; p < nPlants; ++p) {
        if (FixedCosts[p] > maxFixed)
          maxFixed = FixedCosts[p];

        if (FixedCosts[p] < minFixed)
          minFixed = FixedCosts[p];
      }

      // Model
      GRBEnv env = new GRBEnv();
      GRBModel model = new GRBModel(env);

      model.ModelName = "multiscenario";

      // Plant open decision variables: open[p] == 1 if plant p is open.
      GRBVar[] open = new GRBVar[nPlants];
      for (int p = 0; p < nPlants; ++p) {
        open[p] = model.AddVar(0, 1, FixedCosts[p], GRB.BINARY, "Open" + p);
      }

      // Transportation decision variables: how much to transport from
      // a plant p to a warehouse w
      GRBVar[,] transport = new GRBVar[nWarehouses,nPlants];
      for (int w = 0; w < nWarehouses; ++w) {
        for (int p = 0; p < nPlants; ++p) {
          transport[w,p] = model.AddVar(0, GRB.INFINITY, TransCosts[w,p],
                                        GRB.CONTINUOUS, "Trans" + p + "." + w);
        }
      }

      // The objective is to minimize the total fixed and variable costs
      model.ModelSense = GRB.MINIMIZE;

      // Production constraints
      // Note that the right-hand limit sets the production to zero if
      // the plant is closed
      for (int p = 0; p < nPlants; ++p) {
        GRBLinExpr ptot = 0.0;
        for (int w = 0; w < nWarehouses; ++w)
          ptot.AddTerm(1.0, transport[w,p]);
        model.AddConstr(ptot <= Capacity[p] * open[p], "Capacity" + p);
      }

      // Demand constraints
      GRBConstr[] demandConstr = new GRBConstr[nWarehouses];
      for (int w = 0; w < nWarehouses; ++w) {
        GRBLinExpr dtot = 0.0;
        for (int p = 0; p < nPlants; ++p)
          dtot.AddTerm(1.0, transport[w,p]);
        demandConstr[w] = model.AddConstr(dtot == Demand[w], "Demand" + w);
      }

      // We constructed the base model, now we add 7 scenarios
      //
      // Scenario 0: Represents the base model, hence, no manipulations.
      // Scenario 1: Manipulate the warehouses demands slightly (constraint right
      //             hand sides).
      // Scenario 2: Double the warehouses demands (constraint right hand sides).
      // Scenario 3: Manipulate the plant fixed costs (objective coefficients).
      // Scenario 4: Manipulate the warehouses demands and fixed costs.
      // Scenario 5: Force the plant with the largest fixed cost to stay open
      //             (variable bounds).
      // Scenario 6: Force the plant with the smallest fixed cost to be closed
      //             (variable bounds).

      model.NumScenarios = 7;

      // Scenario 0: Base model, hence, nothing to do except giving the
      //             scenario a name
      model.Parameters.ScenarioNumber = 0;
      model.ScenNName = "Base model";

      // Scenario 1: Increase the warehouse demands by 10%
      model.Parameters.ScenarioNumber = 1;
      model.ScenNName = "Increased warehouse demands";

      for (int w = 0; w < nWarehouses; w++) {
        demandConstr[w].ScenNRHS = Demand[w] * 1.1;
      }

      // Scenario 2: Double the warehouse demands
      model.Parameters.ScenarioNumber = 2;
      model.ScenNName = "Double the warehouse demands";

      for (int w = 0; w < nWarehouses; w++) {
        demandConstr[w].ScenNRHS = Demand[w] * 2.0;
      }

      // Scenario 3: Decrease the plant fixed costs by 5%
      model.Parameters.ScenarioNumber = 3;
      model.ScenNName = "Decreased plant fixed costs";

      for (int p = 0; p < nPlants; p++) {
        open[p].ScenNObj = FixedCosts[p] * 0.95;
      }

      // Scenario 4: Combine scenario 1 and scenario 3 */
      model.Parameters.ScenarioNumber = 4;
      model.ScenNName = "Increased warehouse demands and decreased plant fixed costs";

      for (int w = 0; w < nWarehouses; w++) {
        demandConstr[w].ScenNRHS = Demand[w] * 1.1;
      }
      for (int p = 0; p < nPlants; p++) {
        open[p].ScenNObj = FixedCosts[p] * 0.95;
      }

      // Scenario 5: Force the plant with the largest fixed cost to stay
      //             open
      model.Parameters.ScenarioNumber = 5;
      model.ScenNName = "Force plant with largest fixed cost to stay open";

      for (int p = 0; p < nPlants; p++) {
        if (FixedCosts[p] == maxFixed) {
          open[p].ScenNLB = 1.0;
          break;
        }
      }

      // Scenario 6: Force the plant with the smallest fixed cost to be
      //             closed
      model.Parameters.ScenarioNumber = 6;
      model.ScenNName = "Force plant with smallest fixed cost to be closed";

      for (int p = 0; p < nPlants; p++) {
        if (FixedCosts[p] == minFixed) {
          open[p].ScenNUB = 0.0;
          break;
        }
      }

      // Guess at the starting point: close the plant with the highest
      // fixed costs; open all others

      // First, open all plants
      for (int p = 0; p < nPlants; ++p) {
        open[p].Start = 1.0;
      }

      // Now close the plant with the highest fixed cost
      Console.WriteLine("Initial guess:");
      for (int p = 0; p < nPlants; ++p) {
        if (FixedCosts[p] == maxFixed) {
          open[p].Start = 0.0;
          Console.WriteLine("Closing plant " + p + "\n");
          break;
        }
      }

      // Use barrier to solve root relaxation
      model.Parameters.Method = GRB.METHOD_BARRIER;

      // Solve multi-scenario model
      model.Optimize();

      int nScenarios = model.NumScenarios;

      for (int s = 0; s < nScenarios; s++) {
        int modelSense = GRB.MINIMIZE;

        // Set the scenario number to query the information for this scenario
        model.Parameters.ScenarioNumber = s;

        // collect result for the scenario
        double scenNObjBound = model.ScenNObjBound;
        double scenNObjVal = model.ScenNObjVal;

        Console.WriteLine("\n\n------ Scenario " + s
                          + " (" +  model.ScenNName + ")");

        // Check if we found a feasible solution for this scenario
        if (scenNObjVal >= modelSense * GRB.INFINITY)
          if (scenNObjBound >= modelSense * GRB.INFINITY)
            // Scenario was proven to be infeasible
            Console.WriteLine("\nINFEASIBLE");
          else
            // We did not find any feasible solution - should not happen in
            // this case, because we did not set any limit (like a time
            // limit) on the optimization process
            Console.WriteLine("\nNO SOLUTION");
        else {
          Console.WriteLine("\nTOTAL COSTS: " + scenNObjVal);
          Console.WriteLine("SOLUTION:");
          for (int p = 0; p < nPlants; p++) {
            double scenNX = open[p].ScenNX;

            if (scenNX > 0.5) {
              Console.WriteLine("Plant " + p + " open");
              for (int w = 0; w < nWarehouses; w++) {
                scenNX = transport[w,p].ScenNX;

                if (scenNX > 0.0001)
                  Console.WriteLine("  Transport " + scenNX
                                    + " units to warehouse " + w);
              }
            } else
              Console.WriteLine("Plant " + p + " closed!");
          }
        }
      }

      // Print a summary table: for each scenario we add a single summary
      // line
      Console.WriteLine("\n\nSummary: Closed plants depending on scenario\n");
      Console.WriteLine("{0,8} | {1,17} {2,13}", "", "Plant", "|");

      Console.Write("{0,8} |", "Scenario");
      for (int p = 0; p < nPlants; p++)
        Console.Write("{0,6}", p);
      Console.WriteLine(" | {0,6}  Name", "Costs");

      for (int s = 0; s < nScenarios; s++) {
        int modelSense = GRB.MINIMIZE;

        // Set the scenario number to query the information for this scenario
        model.Parameters.ScenarioNumber = s;

        // Collect result for the scenario
        double scenNObjBound = model.ScenNObjBound;
        double scenNObjVal = model.ScenNObjVal;

        Console.Write("{0,-8} |", s);

        // Check if we found a feasible solution for this scenario
        if (scenNObjVal >= modelSense * GRB.INFINITY) {
          if (scenNObjBound >= modelSense * GRB.INFINITY)
            // Scenario was proven to be infeasible
            Console.WriteLine(" {0,-30}| {1,6}  " + model.ScenNName,
                              "infeasible", "-");
          else
            // We did not find any feasible solution - should not happen in
            // this case, because we did not set any limit (like a time
            // limit) on the optimization process
            Console.WriteLine(" {0,-30}| {1,6}  " + model.ScenNName,
                              "no solution found", "-");
        } else {
          for (int p = 0; p < nPlants; p++) {
            double scenNX = open[p].ScenNX;
            if (scenNX  > 0.5)
              Console.Write("{0,6}", " ");
            else
              Console.Write("{0,6}", "x");
          }

          Console.WriteLine(" | {0,6}  "+ model.ScenNName, scenNObjVal);
        }
      }

      // Dispose of model and env
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " + e.Message);
    }
  }
}
