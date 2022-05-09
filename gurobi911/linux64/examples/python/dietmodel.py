#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# Solve the classic diet model.  This file implements
# a function that formulates and solves the model,
# but it contains no model data.  The data is
# passed in by the calling program.  Run example 'diet2.py',
# 'diet3.py', or 'diet4.py' to invoke this function.

import gurobipy as gp
from gurobipy import GRB


def solve(categories, minNutrition, maxNutrition, foods, cost,
          nutritionValues):
    # Model
    m = gp.Model("diet")

    # Create decision variables for the foods to buy
    buy = m.addVars(foods, name="buy")

    # The objective is to minimize the costs
    m.setObjective(buy.prod(cost), GRB.MINIMIZE)

    # Nutrition constraints
    m.addConstrs((gp.quicksum(nutritionValues[f, c] * buy[f] for f in foods) ==
                 [minNutrition[c], maxNutrition[c]] for c in categories), "_")

    def printSolution():
        if m.status == GRB.OPTIMAL:
            print('\nCost: %g' % m.objVal)
            print('\nBuy:')
            for f in foods:
                if buy[f].x > 0.0001:
                    print('%s %g' % (f, buy[f].x))
        else:
            print('No solution')

    # Solve
    m.optimize()
    printSolution()

    print('\nAdding constraint: at most 6 servings of dairy')
    m.addConstr(buy.sum(['milk', 'ice cream']) <= 6, "limit_dairy")

    # Solve
    m.optimize()
    printSolution()
