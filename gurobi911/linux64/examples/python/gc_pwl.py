#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# This example formulates and solves the following simple model
# with PWL constraints:
#
#  maximize
#        sum c[j] * x[j]
#  subject to
#        sum A[i,j] * x[j] <= 0,  for i = 0, ..., m-1
#        sum y[j] <= 3
#        y[j] = pwl(x[j]),        for j = 0, ..., n-1
#        x[j] free, y[j] >= 0,    for j = 0, ..., n-1
#  where pwl(x) = 0,     if x  = 0
#               = 1+|x|, if x != 0
#
#  Note
#   1. sum pwl(x[j]) <= b is to bound x vector and also to favor sparse x vector.
#      Here b = 3 means that at most two x[j] can be nonzero and if two, then
#      sum x[j] <= 1
#   2. pwl(x) jumps from 1 to 0 and from 0 to 1, if x moves from negative 0 to 0,
#      then to positive 0, so we need three points at x = 0. x has infinite bounds
#      on both sides, the piece defined with two points (-1, 2) and (0, 1) can
#      extend x to -infinite. Overall we can use five points (-1, 2), (0, 1),
#      (0, 0), (0, 1) and (1, 2) to define y = pwl(x)
#

import gurobipy as gp
from gurobipy import GRB

try:

    n = 5
    m = 5
    c = [0.5, 0.8, 0.5, 0.1, -1]
    A = [[0, 0, 0, 1, -1],
         [0, 0, 1, 1, -1],
         [1, 1, 0, 0, -1],
         [1, 0, 1, 0, -1],
         [1, 0, 0, 1, -1]]

    # Create a new model
    model = gp.Model("gc_pwl")

    # Create variables
    x = model.addVars(n, lb=-GRB.INFINITY, name="x")
    y = model.addVars(n, name="y")

    # Set objective
    model.setObjective(gp.quicksum(c[j]*x[j] for j in range(n)), GRB.MAXIMIZE)

    # Add Constraints
    for i in range(m):
        model.addConstr(gp.quicksum(A[i][j]*x[j] for j in range(n)) <= 0)

    model.addConstr(y.sum() <= 3)

    for j in range(n):
        model.addGenConstrPWL(x[j], y[j], [-1, 0, 0, 0, 1], [2, 1, 0, 1, 2])

    # Optimize model
    model.optimize()

    for j in range(n):
        print('%s = %g' % (x[j].varName, x[j].x))

    print('Obj: %g' % model.objVal)

except gp.GurobiError as e:
    print('Error code ' + str(e.errno) + ": " + str(e))

except AttributeError:
    print('Encountered an attribute error')
