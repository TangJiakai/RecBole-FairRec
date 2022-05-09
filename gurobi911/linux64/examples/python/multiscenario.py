#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# Facility location: a company currently ships its product from 5 plants to
# 4 warehouses. It is considering closing some plants to reduce costs. What
# plant(s) should the company close, in order to minimize transportation
# and fixed costs?
#
# Since the plant fixed costs and the warehouse demands are uncertain, a
# scenario approach is chosen.
#
# Note that this example is similar to the facility.py example. Here we
# added scenarios in order to illustrate the multi-scenario feature.
#
# Note that this example uses lists instead of dictionaries.  Since
# it does not work with sparse data, lists are a reasonable option.
#
# Based on an example from Frontline Systems:
#   http://www.solver.com/disfacility.htm
# Used with permission.

import gurobipy as gp
from gurobipy import GRB


# Warehouse demand in thousands of units
demand = [15, 18, 14, 20]

# Plant capacity in thousands of units
capacity = [20, 22, 17, 19, 18]

# Fixed costs for each plant
fixedCosts = [12000, 15000, 17000, 13000, 16000]
maxFixed = max(fixedCosts)
minFixed = min(fixedCosts)

# Transportation costs per thousand units
transCosts = [[4000, 2000, 3000, 2500, 4500],
              [2500, 2600, 3400, 3000, 4000],
              [1200, 1800, 2600, 4100, 3000],
              [2200, 2600, 3100, 3700, 3200]]

# Range of plants and warehouses
plants = range(len(capacity))
warehouses = range(len(demand))

# Model
m = gp.Model('multiscenario')

# Plant open decision variables: open[p] == 1 if plant p is open.
open = m.addVars(plants,
                 vtype=GRB.BINARY,
                 obj=fixedCosts,
                 name="open")

# Transportation decision variables: transport[w,p] captures the
# optimal quantity to transport to warehouse w from plant p
transport = m.addVars(warehouses, plants, obj=transCosts, name="trans")

# You could use Python looping constructs and m.addVar() to create
# these decision variables instead.  The following would be equivalent
# to the preceding two statements...
#
# open = []
# for p in plants:
#     open.append(m.addVar(vtype=GRB.BINARY,
#                          obj=fixedCosts[p],
#                          name="open[%d]" % p))
#
# transport = []
# for w in warehouses:
#     transport.append([])
#     for p in plants:
#         transport[w].append(m.addVar(obj=transCosts[w][p],
#                                      name="trans[%d,%d]" % (w, p)))

# The objective is to minimize the total fixed and variable costs
m.modelSense = GRB.MINIMIZE

# Production constraints
# Note that the right-hand limit sets the production to zero if the plant
# is closed
m.addConstrs(
    (transport.sum('*', p) <= capacity[p]*open[p] for p in plants),
    "Capacity")

# Using Python looping constructs, the preceding would be...
#
# for p in plants:
#     m.addConstr(sum(transport[w][p] for w in warehouses)
#                 <= capacity[p] * open[p], "Capacity[%d]" % p)

# Demand constraints
demandConstr = m.addConstrs(
    (transport.sum(w) == demand[w] for w in warehouses), "Demand")

# ... and the preceding would be ...
# for w in warehouses:
#     m.addConstr(sum(transport[w][p] for p in plants) == demand[w],
#                 "Demand[%d]" % w)

# We constructed the base model, now we add 7 scenarios
#
# Scenario 0: Represents the base model, hence, no manipulations.
# Scenario 1: Manipulate the warehouses demands slightly (constraint right
#             hand sides).
# Scenario 2: Double the warehouses demands (constraint right hand sides).
# Scenario 3: Manipulate the plant fixed costs (objective coefficients).
# Scenario 4: Manipulate the warehouses demands and fixed costs.
# Scenario 5: Force the plant with the largest fixed cost to stay open
#             (variable bounds).
# Scenario 6: Force the plant with the smallest fixed cost to be closed
#             (variable bounds).

m.NumScenarios = 7

# Scenario 0: Base model, hence, nothing to do except giving the scenario a
#             name
m.Params.ScenarioNumber = 0
m.ScenNName = 'Base model'

# Scenario 1: Increase the warehouse demands by 10%
m.Params.ScenarioNumber = 1
m.ScenNName = 'Increased warehouse demands'
for w in warehouses:
    demandConstr[w].ScenNRhs = demand[w] * 1.1

# Scenario 2: Double the warehouse demands
m.Params.ScenarioNumber = 2
m.ScenNName = 'Double the warehouse demands'
for w in warehouses:
    demandConstr[w].ScenNRhs = demand[w] * 2.0

# Scenario 3: Decrease the plant fixed costs by 5%
m.Params.ScenarioNumber = 3
m.ScenNName = 'Decreased plant fixed costs'
for p in plants:
    open[p].ScenNObj = fixedCosts[p] * 0.95

# Scenario 4: Combine scenario 1 and scenario 3
m.Params.ScenarioNumber = 4
m.ScenNName = 'Increased warehouse demands and decreased plant fixed costs'
for w in warehouses:
    demandConstr[w].ScenNRhs = demand[w] * 1.1
for p in plants:
    open[p].ScenNObj = fixedCosts[p] * 0.95

# Scenario 5: Force the plant with the largest fixed cost to stay open
m.Params.ScenarioNumber = 5
m.ScenNName = 'Force plant with largest fixed cost to stay open'
open[fixedCosts.index(maxFixed)].ScenNLB = 1.0

# Scenario 6: Force the plant with the smallest fixed cost to be closed
m.Params.ScenarioNumber = 6
m.ScenNName = 'Force plant with smallest fixed cost to be closed'
open[fixedCosts.index(minFixed)].ScenNUB = 0.0

# Save model
m.write('multiscenario.lp')

# Guess at the starting point: close the plant with the highest fixed costs;
# open all others

# First open all plants
for p in plants:
    open[p].start = 1.0

# Now close the plant with the highest fixed cost
p = fixedCosts.index(maxFixed)
open[p].start = 0.0
print('Initial guess: Closing plant %d\n' % p)

# Use barrier to solve root relaxation
m.Params.method = 2

# Solve multi-scenario model
m.optimize()

# Print solution for each scenario
for s in range(m.NumScenarios):
    # Set the scenario number to query the information for this scenario
    m.Params.ScenarioNumber = s

    print('\n\n------ Scenario %d (%s)' % (s, m.ScenNName))

    # Check if we found a feasible solution for this scenario
    if m.ScenNObjVal >= m.ModelSense * GRB.INFINITY:
        if m.ScenNObjBound >= m.ModelSense * GRB.INFINITY:
            # Scenario was proven to be infeasible
            print('\nINFEASIBLE')
        else:
            # We did not find any feasible solution - should not happen in
            # this case, because we did not set any limit (like a time
            # limit) on the optimization process
            print('\nNO SOLUTION')
    else:
        print('\nTOTAL COSTS: %g' % m.ScenNObjVal)
        print('SOLUTION:')
        for p in plants:
            if open[p].ScenNX > 0.5:
                print('Plant %s open' % p)
                for w in warehouses:
                    if transport[w, p].ScenNX > 0:
                        print('  Transport %g units to warehouse %s' %
                              (transport[w, p].ScenNX, w))
            else:
                print('Plant %s closed!' % p)

# Print a summary table: for each scenario we add a single summary line
print('\n\nSummary: Closed plants depending on scenario\n')
tableStr = '%8s | %17s %13s' % ('', 'Plant', '|')
print(tableStr)

tableStr = '%8s |' % 'Scenario'
for p in plants:
    tableStr = tableStr + ' %5d' % p
tableStr = tableStr + ' | %6s  %-s' % ('Costs', 'Name')
print(tableStr)

for s in range(m.NumScenarios):
    # Set the scenario number to query the information for this scenario
    m.Params.ScenarioNumber = s

    tableStr = '%-8d |' % s

    # Check if we found a feasible solution for this scenario
    if m.ScenNObjVal >= m.ModelSense * GRB.INFINITY:
        if m.ScenNObjBound >= m.ModelSense * GRB.INFINITY:
            # Scenario was proven to be infeasible
            print(tableStr + ' %-30s| %6s  %-s' %
                  ('infeasible', '-', m.ScenNName))
        else:
            # We did not find any feasible solution - should not happen in
            # this case, because we did not set any limit (like a time
            # limit) on the optimization process
            print(tableStr + ' %-30s| %6s  %-s' %
                  ('no solution found', '-', m.ScenNName))
    else:
        for p in plants:
            if open[p].ScenNX > 0.5:
                tableStr = tableStr + ' %5s' % ' '
            else:
                tableStr = tableStr + ' %5s' % 'x'

        print(tableStr + ' | %6g  %-s' % (m.ScenNObjVal, m.ScenNName))
