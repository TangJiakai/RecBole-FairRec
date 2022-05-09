#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# This example formulates and solves the following simple QCP model:
#  maximize    x
#  subject to  x + y + z = 1
#              x^2 + y^2 <= z^2 (second-order cone)
#              x^2 <= yz        (rotated second-order cone)
#              x, y, z non-negative

import gurobipy as gp
from gurobipy import GRB

# Create a new model
m = gp.Model("qcp")

# Create variables
x = m.addVar(name="x")
y = m.addVar(name="y")
z = m.addVar(name="z")

# Set objective: x
obj = 1.0*x
m.setObjective(obj, GRB.MAXIMIZE)

# Add constraint: x + y + z = 1
m.addConstr(x + y + z == 1, "c0")

# Add second-order cone: x^2 + y^2 <= z^2
m.addConstr(x**2 + y**2 <= z**2, "qc0")

# Add rotated cone: x^2 <= yz
m.addConstr(x**2 <= y*z, "qc1")

m.optimize()

for v in m.getVars():
    print('%s %g' % (v.varName, v.x))

print('Obj: %g' % obj.getValue())
