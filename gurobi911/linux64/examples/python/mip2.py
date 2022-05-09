#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# This example reads a MIP model from a file, solves it and prints
# the objective values from all feasible solutions generated while
# solving the MIP. Then it creates the associated fixed model and
# solves that model.

import sys
import gurobipy as gp
from gurobipy import GRB

if len(sys.argv) < 2:
    print('Usage: mip2.py filename')
    sys.exit(0)

# Read and solve model

model = gp.read(sys.argv[1])

if model.isMIP == 0:
    print('Model is not a MIP')
    sys.exit(0)

model.optimize()

if model.status == GRB.OPTIMAL:
    print('Optimal objective: %g' % model.objVal)
elif model.status == GRB.INF_OR_UNBD:
    print('Model is infeasible or unbounded')
    sys.exit(0)
elif model.status == GRB.INFEASIBLE:
    print('Model is infeasible')
    sys.exit(0)
elif model.status == GRB.UNBOUNDED:
    print('Model is unbounded')
    sys.exit(0)
else:
    print('Optimization ended with status %d' % model.status)
    sys.exit(0)

# Iterate over the solutions and compute the objectives
model.Params.outputFlag = 0
print('')
for k in range(model.solCount):
    model.Params.solutionNumber = k
    print('Solution %d has objective %g' % (k, model.poolObjVal))
print('')
model.Params.outputFlag = 1

fixed = model.fixed()
fixed.Params.presolve = 0
fixed.optimize()

if fixed.status != GRB.OPTIMAL:
    print("Error: fixed model isn't optimal")
    sys.exit(1)

diff = model.objVal - fixed.objVal

if abs(diff) > 1e-6 * (1.0 + abs(model.objVal)):
    print('Error: objective values are different')
    sys.exit(1)

# Print values of nonzero variables
for v in fixed.getVars():
    if v.x != 0:
        print('%s %g' % (v.varName, v.x))
