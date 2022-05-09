// Copyright 2020, Gurobi Optimization, LLC

// Facility location: a company currently ships its product from 5 plants
// to 4 warehouses. It is considering closing some plants to reduce
// costs. What plant(s) should the company close, in order to minimize
// transportation and fixed costs?
//
// Since the plant fixed costs and the warehouse demands are uncertain, a
// scenario approach is chosen.
//
// Note that this example is similar to the facility_c++.cpp example. Here
// we added scenarios in order to illustrate the multi-scenario feature.
//
// Based on an example from Frontline Systems:
// http://www.solver.com/disfacility.htm
// Used with permission.

#include "gurobi_c++.h"
#include <sstream>
#include <iomanip>
using namespace std;

int
main(int   argc,
     char *argv[])
{
  GRBEnv     *env          = 0;
  GRBVar     *open         = 0;
  GRBVar    **transport    = 0;
  GRBConstr  *demandConstr = 0;

  int transportCt = 0;

  try {
    // Number of plants and warehouses
    const int nPlants = 5;
    const int nWarehouses = 4;

    // Warehouse demand in thousands of units
    double Demand[] = { 15, 18, 14, 20 };

    // Plant capacity in thousands of units
    double Capacity[] = { 20, 22, 17, 19, 18 };

    // Fixed costs for each plant
    double FixedCosts[] =
      { 12000, 15000, 17000, 13000, 16000 };

    // Transportation costs per thousand units
    double TransCosts[][nPlants] = {
      { 4000, 2000, 3000, 2500, 4500 },
      { 2500, 2600, 3400, 3000, 4000 },
      { 1200, 1800, 2600, 4100, 3000 },
      { 2200, 2600, 3100, 3700, 3200 }
    };

    double maxFixed = -GRB_INFINITY;
    double minFixed = GRB_INFINITY;

    int p;
    for (p = 0; p < nPlants; p++) {
      if (FixedCosts[p] > maxFixed)
        maxFixed = FixedCosts[p];

      if (FixedCosts[p] < minFixed)
        minFixed = FixedCosts[p];
    }

    // Model
    env = new GRBEnv();
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "multiscenario");

    // Plant open decision variables: open[p] == 1 if plant p is open.
    open = model.addVars(nPlants, GRB_BINARY);

    for (p = 0; p < nPlants; p++) {
      ostringstream vname;
      vname << "Open" << p;
      open[p].set(GRB_DoubleAttr_Obj, FixedCosts[p]);
      open[p].set(GRB_StringAttr_VarName, vname.str());
    }

    // Transportation decision variables: how much to transport from
    // a plant p to a warehouse w
    transport = new GRBVar* [nWarehouses];
    int w;
    for (w = 0; w < nWarehouses; w++) {
      transport[w] = model.addVars(nPlants);
      transportCt++;

      for (p = 0; p < nPlants; p++) {
        ostringstream vname;
        vname << "Trans" << p << "." << w;
        transport[w][p].set(GRB_DoubleAttr_Obj, TransCosts[w][p]);
        transport[w][p].set(GRB_StringAttr_VarName, vname.str());
      }
    }

    // The objective is to minimize the total fixed and variable costs
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    // Production constraints
    // Note that the right-hand limit sets the production to zero if
    // the plant is closed
    for (p = 0; p < nPlants; p++) {
      GRBLinExpr ptot = 0;
      for (w = 0; w < nWarehouses; w++) {
        ptot += transport[w][p];
      }
      ostringstream cname;
      cname << "Capacity" << p;
      model.addConstr(ptot <= Capacity[p] * open[p], cname.str());
    }

    // Demand constraints
    demandConstr = new GRBConstr[nWarehouses];
    for (w = 0; w < nWarehouses; w++) {
      GRBLinExpr dtot = 0;
      for (p = 0; p < nPlants; p++)
        dtot += transport[w][p];

      ostringstream cname;
      cname << "Demand" << w;
      demandConstr[w] = model.addConstr(dtot == Demand[w], cname.str());
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

    model.set(GRB_IntAttr_NumScenarios, 7);

    // Scenario 0: Base model, hence, nothing to do except giving the
    //             scenario a name
    model.set(GRB_IntParam_ScenarioNumber, 0);
    model.set(GRB_StringAttr_ScenNName, "Base model");

    // Scenario 1: Increase the warehouse demands by 10%
    model.set(GRB_IntParam_ScenarioNumber, 1);
    model.set(GRB_StringAttr_ScenNName, "Increased warehouse demands");

    for (w = 0; w < nWarehouses; w++) {
      demandConstr[w].set(GRB_DoubleAttr_ScenNRHS, Demand[w] * 1.1);
    }

    // Scenario 2: Double the warehouse demands
    model.set(GRB_IntParam_ScenarioNumber, 2);
    model.set(GRB_StringAttr_ScenNName, "Double the warehouse demands");

    for (w = 0; w < nWarehouses; w++) {
      demandConstr[w].set(GRB_DoubleAttr_ScenNRHS, Demand[w] * 2.0);
    }

    // Scenario 3: Decrease the plant fixed costs by 5%
    model.set(GRB_IntParam_ScenarioNumber, 3);
    model.set(GRB_StringAttr_ScenNName, "Decreased plant fixed costs");

    for (p = 0; p < nPlants; p++) {
      open[p].set(GRB_DoubleAttr_ScenNObj, FixedCosts[p] * 0.95);
    }

    // Scenario 4: Combine scenario 1 and scenario 3 */
    model.set(GRB_IntParam_ScenarioNumber, 4);
    model.set(GRB_StringAttr_ScenNName, "Increased warehouse demands and decreased plant fixed costs");

    for (w = 0; w < nWarehouses; w++) {
      demandConstr[w].set(GRB_DoubleAttr_ScenNRHS, Demand[w] * 1.1);
    }
    for (p = 0; p < nPlants; p++) {
      open[p].set(GRB_DoubleAttr_ScenNObj, FixedCosts[p] * 0.95);
    }

    // Scenario 5: Force the plant with the largest fixed cost to stay
    //             open
    model.set(GRB_IntParam_ScenarioNumber, 5);
    model.set(GRB_StringAttr_ScenNName, "Force plant with largest fixed cost to stay open");

    for (p = 0; p < nPlants; p++) {
      if (FixedCosts[p] == maxFixed) {
        open[p].set(GRB_DoubleAttr_ScenNLB, 1.0);
        break;
      }
    }

    // Scenario 6: Force the plant with the smallest fixed cost to be
    //             closed
    model.set(GRB_IntParam_ScenarioNumber, 6);
    model.set(GRB_StringAttr_ScenNName, "Force plant with smallest fixed cost to be closed");

    for (p = 0; p < nPlants; p++) {
      if (FixedCosts[p] == minFixed) {
        open[p].set(GRB_DoubleAttr_ScenNUB, 0.0);
        break;
      }
    }

    // Guess at the starting point: close the plant with the highest
    // fixed costs; open all others

    // First, open all plants
    for (p = 0; p < nPlants; p++)
      open[p].set(GRB_DoubleAttr_Start, 1.0);

    // Now close the plant with the highest fixed cost
    cout << "Initial guess:" << endl;
    for (p = 0; p < nPlants; p++) {
      if (FixedCosts[p] == maxFixed) {
        open[p].set(GRB_DoubleAttr_Start, 0.0);
        cout << "Closing plant " << p << endl << endl;
        break;
      }
    }

    // Use barrier to solve root relaxation
    model.set(GRB_IntParam_Method, GRB_METHOD_BARRIER);

    // Solve multi-scenario model
    model.optimize();

    int nScenarios = model.get(GRB_IntAttr_NumScenarios);

    // Print solution for each */
    for (int s = 0; s < nScenarios; s++) {
      int modelSense = GRB_MINIMIZE;

      // Set the scenario number to query the information for this scenario
      model.set(GRB_IntParam_ScenarioNumber, s);

      // collect result for the scenario
      double scenNObjBound = model.get(GRB_DoubleAttr_ScenNObjBound);
      double scenNObjVal = model.get(GRB_DoubleAttr_ScenNObjVal);

      cout << endl << endl << "------ Scenario " << s
           << " (" << model.get(GRB_StringAttr_ScenNName) << ")" << endl;

      // Check if we found a feasible solution for this scenario
      if (scenNObjVal >= modelSense * GRB_INFINITY)
        if (scenNObjBound >= modelSense * GRB_INFINITY)
          // Scenario was proven to be infeasible
          cout << endl << "INFEASIBLE" << endl;
        else
          // We did not find any feasible solution - should not happen in
          // this case, because we did not set any limit (like a time
          // limit) on the optimization process
          cout << endl << "NO SOLUTION" << endl;
      else {
        cout << endl << "TOTAL COSTS: " << scenNObjVal << endl;
        cout << "SOLUTION:" << endl;
        for (p = 0; p < nPlants; p++) {
          double scenNX = open[p].get(GRB_DoubleAttr_ScenNX);

          if (scenNX > 0.5) {
            cout << "Plant " << p << " open" << endl;
            for (w = 0; w < nWarehouses; w++) {
              scenNX = transport[w][p].get(GRB_DoubleAttr_ScenNX);

              if (scenNX > 0.0001)
                cout << "  Transport " << scenNX
                     << " units to warehouse " << w << endl;
            }
          } else
            cout << "Plant " << p << " closed!" << endl;
        }
      }
    }

    // Print a summary table: for each scenario we add a single summary
    // line
    cout << endl << endl << "Summary: Closed plants depending on scenario" << endl << endl;
    cout << setw(8) << " " << " | " << setw(17) << "Plant" << setw(14) << "|" << endl;

    cout << setw(8) << "Scenario" << " |";
    for (p = 0; p < nPlants; p++)
      cout << " " << setw(5) << p;
    cout << " | " << setw(6) << "Costs" << "  Name" << endl;

    for (int s = 0; s < nScenarios; s++) {
      int modelSense = GRB_MINIMIZE;

      // Set the scenario number to query the information for this scenario
      model.set(GRB_IntParam_ScenarioNumber, s);

      // Collect result for the scenario
      double scenNObjBound = model.get(GRB_DoubleAttr_ScenNObjBound);
      double scenNObjVal = model.get(GRB_DoubleAttr_ScenNObjVal);

      cout << left << setw(8) << s << right << " |";

      // Check if we found a feasible solution for this scenario
      if (scenNObjVal >= modelSense * GRB_INFINITY) {
        if (scenNObjBound >= modelSense * GRB_INFINITY)
          // Scenario was proven to be infeasible
          cout << " " << left << setw(30) << "infeasible" << right;
        else
          // We did not find any feasible solution - should not happen in
          // this case, because we did not set any limit (like a time
          // limit) on the optimization process
          cout << " " << left << setw(30) << "no solution found" << right;

        cout << "| " << setw(6) << "-"
             << "  " << model.get(GRB_StringAttr_ScenNName)
             << endl;
      } else {
        for (p = 0; p < nPlants; p++) {
          double scenNX = open[p].get(GRB_DoubleAttr_ScenNX);
          if (scenNX  > 0.5)
            cout << setw(6) << " ";
          else
            cout << " " << setw(5) << "x";
        }

        cout << " | " << setw(6) << scenNObjVal
             << "  " << model.get(GRB_StringAttr_ScenNName)
             << endl;
      }
    }
  }
  catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  }
  catch (...) {
    cout << "Exception during optimization" << endl;
  }

  delete[] open;
  for (int i = 0; i < transportCt; ++i) {
    delete[] transport[i];
  }
  delete[] transport;
  delete[] demandConstr;
  delete env;
  return 0;
}
