#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# Use parameters that are associated with a model.
#
# A MIP is solved for a few seconds with different sets of parameters.
# The one with the smallest MIP gap is selected, and the optimization
# is resumed until the optimal solution is found.

import sys
import gurobipy as gp


if len(sys.argv) < 2:
    print('Usage: params.py filename')
    sys.exit(0)


# Read model and verify that it is a MIP
m = gp.read(sys.argv[1])
if m.isMIP == 0:
    print('The model is not an integer program')
    sys.exit(1)

# Set a 2 second time limit
m.Params.timeLimit = 2

# Now solve the model with different values of MIPFocus
bestModel = m.copy()
bestModel.optimize()
for i in range(1, 4):
    m.reset()
    m.Params.MIPFocus = i
    m.optimize()
    if bestModel.MIPGap > m.MIPGap:
        bestModel, m = m, bestModel  # swap models

# Finally, delete the extra model, reset the time limit and
# continue to solve the best model to optimality
del m
bestModel.Params.timeLimit = "default"
bestModel.optimize()
print('Solved with MIPFocus: %d' % bestModel.Params.MIPFocus)
