// Copyright 2020, Gurobi Optimization, LLC

// Facility location: a company currently ships its product from 5 plants
// to 4 warehouses. It is considering closing some plants to reduce
// costs. What plant(s) should the company close, in order to minimize
// transportation and fixed costs?
//
// Since the plant fixed costs and the warehouse demands are uncertain, a
// scenario approach is chosen.
//
// Note that this example is similar to the Facility.java example. Here we
// added scenarios in order to illustrate the multi-scenario feature.
//
// Based on an example from Frontline Systems:
// http://www.solver.com/disfacility.htm
// Used with permission.

import gurobi.*;

public class Multiscenario {

  public static void main(String[] args) {
    try {

      // Warehouse demand in thousands of units
      double Demand[] = new double[] { 15, 18, 14, 20 };

      // Plant capacity in thousands of units
      double Capacity[] = new double[] { 20, 22, 17, 19, 18 };

      // Fixed costs for each plant
      double FixedCosts[] =
        new double[] { 12000, 15000, 17000, 13000, 16000 };

      // Transportation costs per thousand units
      double TransCosts[][] =
        new double[][] { { 4000, 2000, 3000, 2500, 4500 },
                         { 2500, 2600, 3400, 3000, 4000 },
                         { 1200, 1800, 2600, 4100, 3000 },
                         { 2200, 2600, 3100, 3700, 3200 } };

      // Number of plants and warehouses
      int nPlants = Capacity.length;
      int nWarehouses = Demand.length;

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
      model.set(GRB.StringAttr.ModelName, "multiscenario");

      // Plant open decision variables: open[p] == 1 if plant p is open.
      GRBVar[] open = new GRBVar[nPlants];
      for (int p = 0; p < nPlants; ++p) {
        open[p] = model.addVar(0, 1, FixedCosts[p], GRB.BINARY, "Open" + p);
      }

      // Transportation decision variables: how much to transport from
      // a plant p to a warehouse w
      GRBVar[][] transport = new GRBVar[nWarehouses][nPlants];
      for (int w = 0; w < nWarehouses; ++w) {
        for (int p = 0; p < nPlants; ++p) {
          transport[w][p] = model.addVar(0, GRB.INFINITY, TransCosts[w][p],
                                         GRB.CONTINUOUS, "Trans" + p + "." + w);
        }
      }

      // The objective is to minimize the total fixed and variable costs
      model.set(GRB.IntAttr.ModelSense, GRB.MINIMIZE);

      // Production constraints
      // Note that the right-hand limit sets the production to zero if
      // the plant is closed
      for (int p = 0; p < nPlants; ++p) {
        GRBLinExpr ptot = new GRBLinExpr();
        for (int w = 0; w < nWarehouses; ++w) {
          ptot.addTerm(1.0, transport[w][p]);
        }
        GRBLinExpr limit = new GRBLinExpr();
        limit.addTerm(Capacity[p], open[p]);
        model.addConstr(ptot, GRB.LESS_EQUAL, limit, "Capacity" + p);
      }

      // Demand constraints
      GRBConstr[] demandConstr = new GRBConstr[nWarehouses];
      for (int w = 0; w < nWarehouses; ++w) {
        GRBLinExpr dtot = new GRBLinExpr();
        for (int p = 0; p < nPlants; ++p) {
          dtot.addTerm(1.0, transport[w][p]);
        }
        demandConstr[w] = model.addConstr(dtot, GRB.EQUAL, Demand[w], "Demand" + w);
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

      model.set(GRB.IntAttr.NumScenarios, 7);

      // Scenario 0: Base model, hence, nothing to do except giving the
      //             scenario a name
      model.set(GRB.IntParam.ScenarioNumber, 0);
      model.set(GRB.StringAttr.ScenNName, "Base model");

      // Scenario 1: Increase the warehouse demands by 10%
      model.set(GRB.IntParam.ScenarioNumber, 1);
      model.set(GRB.StringAttr.ScenNName, "Increased warehouse demands");

      for (int w = 0; w < nWarehouses; w++) {
        demandConstr[w].set(GRB.DoubleAttr.ScenNRHS, Demand[w] * 1.1);
      }

      // Scenario 2: Double the warehouse demands
      model.set(GRB.IntParam.ScenarioNumber, 2);
      model.set(GRB.StringAttr.ScenNName, "Double the warehouse demands");

      for (int w = 0; w < nWarehouses; w++) {
        demandConstr[w].set(GRB.DoubleAttr.ScenNRHS, Demand[w] * 2.0);
      }

      // Scenario 3: Decrease the plant fixed costs by 5%
      model.set(GRB.IntParam.ScenarioNumber, 3);
      model.set(GRB.StringAttr.ScenNName, "Decreased plant fixed costs");

      for (int p = 0; p < nPlants; p++) {
        open[p].set(GRB.DoubleAttr.ScenNObj, FixedCosts[p] * 0.95);
      }

      // Scenario 4: Combine scenario 1 and scenario 3 */
      model.set(GRB.IntParam.ScenarioNumber, 4);
      model.set(GRB.StringAttr.ScenNName, "Increased warehouse demands and decreased plant fixed costs");

      for (int w = 0; w < nWarehouses; w++) {
        demandConstr[w].set(GRB.DoubleAttr.ScenNRHS, Demand[w] * 1.1);
      }
      for (int p = 0; p < nPlants; p++) {
        open[p].set(GRB.DoubleAttr.ScenNObj, FixedCosts[p] * 0.95);
      }

      // Scenario 5: Force the plant with the largest fixed cost to stay
      //             open
      model.set(GRB.IntParam.ScenarioNumber, 5);
      model.set(GRB.StringAttr.ScenNName, "Force plant with largest fixed cost to stay open");

      for (int p = 0; p < nPlants; p++) {
        if (FixedCosts[p] == maxFixed) {
          open[p].set(GRB.DoubleAttr.ScenNLB, 1.0);
          break;
        }
      }

      // Scenario 6: Force the plant with the smallest fixed cost to be
      //             closed
      model.set(GRB.IntParam.ScenarioNumber, 6);
      model.set(GRB.StringAttr.ScenNName, "Force plant with smallest fixed cost to be closed");

      for (int p = 0; p < nPlants; p++) {
        if (FixedCosts[p] == minFixed) {
          open[p].set(GRB.DoubleAttr.ScenNUB, 0.0);
          break;
        }
      }

      // Guess at the starting point: close the plant with the highest
      // fixed costs; open all others

      // First, open all plants
      for (int p = 0; p < nPlants; ++p) {
        open[p].set(GRB.DoubleAttr.Start, 1.0);
      }

      // Now close the plant with the highest fixed cost
      System.out.println("Initial guess:");
      for (int p = 0; p < nPlants; ++p) {
        if (FixedCosts[p] == maxFixed) {
          open[p].set(GRB.DoubleAttr.Start, 0.0);
          System.out.println("Closing plant " + p + "\n");
          break;
        }
      }

      // Use barrier to solve root relaxation
      model.set(GRB.IntParam.Method, GRB.METHOD_BARRIER);

      // Solve multi-scenario model
      model.optimize();

      int nScenarios = model.get(GRB.IntAttr.NumScenarios);

      // Print solution for each */
      for (int s = 0; s < nScenarios; s++) {
        int modelSense = GRB.MINIMIZE;

        // Set the scenario number to query the information for this scenario
        model.set(GRB.IntParam.ScenarioNumber, s);

        // collect result for the scenario
        double scenNObjBound = model.get(GRB.DoubleAttr.ScenNObjBound);
        double scenNObjVal = model.get(GRB.DoubleAttr.ScenNObjVal);

        System.out.println("\n\n------ Scenario " + s +
                           " (" +  model.get(GRB.StringAttr.ScenNName) + ")");

        // Check if we found a feasible solution for this scenario
        if (scenNObjVal >= modelSense * GRB.INFINITY)
          if (scenNObjBound >= modelSense * GRB.INFINITY)
            // Scenario was proven to be infeasible
            System.out.println("\nINFEASIBLE");
          else
            // We did not find any feasible solution - should not happen in
            // this case, because we did not set any limit (like a time
            // limit) on the optimization process
            System.out.println("\nNO SOLUTION");
        else {
          System.out.println("\nTOTAL COSTS: " + scenNObjVal);
          System.out.println("SOLUTION:");
          for (int p = 0; p < nPlants; p++) {
            double scenNX = open[p].get(GRB.DoubleAttr.ScenNX);

            if (scenNX > 0.5) {
              System.out.println("Plant " + p + " open");
              for (int w = 0; w < nWarehouses; w++) {
                scenNX = transport[w][p].get(GRB.DoubleAttr.ScenNX);

                if (scenNX > 0.0001)
                  System.out.println("  Transport " + scenNX +
                                     " units to warehouse " + w);
              }
            } else
              System.out.println("Plant " + p + " closed!");
          }
        }
      }

      // Print a summary table: for each scenario we add a single summary
      // line
      System.out.println("\n\nSummary: Closed plants depending on scenario\n");
      System.out.format("%8s | %17s %13s\n", "", "Plant", "|");

      System.out.format("%8s |", "Scenario");
      for (int p = 0; p < nPlants; p++)
        System.out.format(" %5d", p);
      System.out.format(" | %6s  %s\n", "Costs", "Name");

      for (int s = 0; s < nScenarios; s++) {
        int modelSense = GRB.MINIMIZE;

        // Set the scenario number to query the information for this scenario
        model.set(GRB.IntParam.ScenarioNumber, s);

        // Collect result for the scenario
        double scenNObjBound = model.get(GRB.DoubleAttr.ScenNObjBound);
        double scenNObjVal = model.get(GRB.DoubleAttr.ScenNObjVal);

        System.out.format("%-8d |", s);

        // Check if we found a feasible solution for this scenario
        if (scenNObjVal >= modelSense * GRB.INFINITY) {
          if (scenNObjBound >= modelSense * GRB.INFINITY)
            // Scenario was proven to be infeasible
            System.out.format(" %-30s| %6s  %s\n",
                              "infeasible", "-", model.get(GRB.StringAttr.ScenNName));
          else
            // We did not find any feasible solution - should not happen in
            // this case, because we did not set any limit (like a time
            // limit) on the optimization process
            System.out.format(" %-30s| %6s  %s\n",
                              "no solution found", "-", model.get(GRB.StringAttr.ScenNName));
        } else {
          for (int p = 0; p < nPlants; p++) {
            double scenNX = open[p].get(GRB.DoubleAttr.ScenNX);
            if (scenNX  > 0.5)
              System.out.format("%6s", " ");
            else
              System.out.format("%6s", "x");
          }

          System.out.format(" | %6g  %s\n", scenNObjVal, model.get(GRB.StringAttr.ScenNName));
        }
      }

      // Dispose of model and environment
      model.dispose();
      env.dispose();
    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode() + ". " +
                         e.getMessage());
    }
  }
}
