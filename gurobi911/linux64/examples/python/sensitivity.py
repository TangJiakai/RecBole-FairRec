#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# A simple sensitivity analysis example which reads a MIP model from a file
# and solves it. Then uses the scenario feature to analyze the impact
# w.r.t. the objective function of each binary variable if it is set to
# 1-X, where X is its value in the optimal solution.
#
# Usage:
#     sensitivity.py <model filename>
#

import sys
import gurobipy as gp
from gurobipy import GRB


# Maximum number of scenarios to be considered
maxScenarios = 100

if len(sys.argv) < 2:
    print('Usage: sensitivity.py filename')
    sys.exit(0)

# Read model
model = gp.read(sys.argv[1])

if model.IsMIP == 0:
    print('Model is not a MIP')
    sys.exit(0)

# Solve model
model.optimize()

if model.status != GRB.OPTIMAL:
    print('Optimization ended with status %d' % model.status)
    sys.exit(0)

# Store the optimal solution
origObjVal = model.ObjVal
for v in model.getVars():
    v._origX = v.X


scenarios = 0

# Count number of unfixed, binary variables in model. For each we create a
# scenario.
for v in model.getVars():
    if (v.LB == 0.0 and v.UB == 1.0 and v.VType in (GRB.BINARY, GRB.INTEGER)):
        scenarios += 1

        if scenarios >= maxScenarios:
            break


# Set the number of scenarios in the model
model.NumScenarios = scenarios
scenarios = 0

print('###  construct multi-scenario model with %d scenarios' % scenarios)

# Create a (single) scenario model by iterating through unfixed binary
# variables in the model and create for each of these variables a scenario
# by fixing the variable to 1-X, where X is its value in the computed
# optimal solution
for v in model.getVars():
    if (v.LB == 0.0 and v.UB == 1.0
        and v.VType in (GRB.BINARY, GRB.INTEGER)
        and scenarios < maxScenarios):

        # Set ScenarioNumber parameter to select the corresponding scenario
        # for adjustments
        model.params.ScenarioNumber = scenarios

        # Set variable to 1-X, where X is its value in the optimal solution
        if v._origX < 0.5:
            v.ScenNLB = 1.0
        else:
            v.ScenNUB = 0.0

        scenarios += 1

    else:
        # Add MIP start for all other variables using the optimal solution
        # of the base model
        v.Start = v._origX


# Solve multi-scenario model
model.optimize()


# In case we solved the scenario model to optimality capture the
# sensitivity information
if model.status == GRB.OPTIMAL:

    modelSense = model.ModelSense
    scenarios = 0

    # Capture sensitivity information from each scenario
    for v in model.getVars():
        if (v.LB == 0.0 and v.UB == 1.0 and v.VType in (GRB.BINARY, GRB.INTEGER)):

            # Set scenario parameter to collect the objective value of the
            # corresponding scenario
            model.params.ScenarioNumber = scenarios

            # Collect objective value and bound for the scenario
            scenarioObjVal = model.ScenNObjVal
            scenarioObjBound = model.ScenNObjBound

            # Check if we found a feasible solution for this scenario
            if scenarioObjVal >= modelSense * GRB.INFINITY:
                # Check if the scenario is infeasible
                if scenarioObjBound >= modelSense * GRB.INFINITY:
                    print('Objective sensitivity for variable %s is infeasible' %
                          v.VarName)
                else:
                    print('Objective sensitivity for variable %s is unknown (no solution available)' %
                          v.VarName)
            else:
                # Scenario is feasible and a solution is available
                print('Objective sensitivity for variable %s is %g' %
                      (v.VarName, modelSense * (scenarioObjVal - origObjVal)))

            scenarios += 1

            if scenarios >= maxScenarios:
                break
