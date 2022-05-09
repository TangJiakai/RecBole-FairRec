#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# This example considers the following nonconvex nonlinear problem
#
#  maximize    2 x    + y
#  subject to  exp(x) + 4 sqrt(y) <= 9
#              x, y >= 0
#
#  We show you two approaches to solve this:
#
#  1) Use a piecewise-linear approach to handle general function
#     constraints (such as exp and sqrt).
#     a) Add two variables
#        u = exp(x)
#        v = sqrt(y)
#     b) Compute points (x, u) of u = exp(x) for some step length (e.g., x
#        = 0, 1e-3, 2e-3, ..., xmax) and points (y, v) of v = sqrt(y) for
#        some step length (e.g., y = 0, 1e-3, 2e-3, ..., ymax). We need to
#        compute xmax and ymax (which is easy for this example, but this
#        does not hold in general).
#     c) Use the points to add two general constraints of type
#        piecewise-linear.
#
#  2) Use the Gurobis built-in general function constraints directly (EXP
#     and POW). Here, we do not need to compute the points and the maximal
#     possible values, which will be done internally by Gurobi.  In this
#     approach, we show how to "zoom in" on the optimal solution and
#     tighten tolerances to improve the solution quality.
#

import math
import gurobipy as gp
from gurobipy import GRB


def printsol(m, x, y, u, v):
    print('x = ' + str(x.x) + ', u = ' + str(u.x))
    print('y = ' + str(y.x) + ', v = ' + str(v.x))
    print('Obj = ' + str(m.objVal))

    # Calculate violation of exp(x) + 4 sqrt(y) <= 9
    vio = math.exp(x.x) + 4 * math.sqrt(y.x) - 9
    if vio < 0:
        vio = 0
    print('Vio = ' + str(vio))


try:

    # Create a new model
    m = gp.Model()

    # Create variables
    x = m.addVar(name='x')
    y = m.addVar(name='y')
    u = m.addVar(name='u')
    v = m.addVar(name='v')

    # Set objective
    m.setObjective(2*x + y, GRB.MAXIMIZE)

    # Add constraints
    lc = m.addConstr(u + 4*v <= 9)

    # Approach 1) PWL constraint approach

    xpts = []
    ypts = []
    upts = []
    vpts = []

    intv = 1e-3

    xmax = math.log(9)
    t = 0.0
    while t < xmax + intv:
        xpts.append(t)
        upts.append(math.exp(t))
        t += intv

    ymax = (9.0/4)*(9.0/4)
    t = 0.0
    while t < ymax + intv:
        ypts.append(t)
        vpts.append(math.sqrt(t))
        t += intv

    gc1 = m.addGenConstrPWL(x, u, xpts, upts, "gc1")
    gc2 = m.addGenConstrPWL(y, v, ypts, vpts, "gc2")

    # Optimize the model
    m.optimize()

    printsol(m, x, y, u, v)

    # Approach 2) General function constraint approach with auto PWL
    #             translation by Gurobi

    # restore unsolved state and get rid of PWL constraints
    m.reset()
    m.remove(gc1)
    m.remove(gc2)
    m.update()

    # u = exp(x)
    gcf1 = m.addGenConstrExp(x, u, name="gcf1")
    # v = x^(0.5)
    gcf2 = m.addGenConstrPow(y, v, 0.5, name="gcf2")

    # Use the equal piece length approach with the length = 1e-3
    m.params.FuncPieces = 1
    m.params.FuncPieceLength = 1e-3

    # Optimize the model
    m.optimize()

    printsol(m, x, y, u, v)

    # Zoom in, use optimal solution to reduce the ranges and use a smaller
    # pclen=1-5 to solve it

    x.lb = max(x.lb, x.x-0.01)
    x.ub = min(x.ub, x.x+0.01)
    y.lb = max(y.lb, y.x-0.01)
    y.ub = min(y.ub, y.x+0.01)
    m.update()
    m.reset()

    m.params.FuncPieceLength = 1e-5

    # Optimize the model
    m.optimize()

    printsol(m, x, y, u, v)

except gp.GurobiError as e:
    print('Error code ' + str(e.errno) + ": " + str(e))

except AttributeError:
    print('Encountered an attribute error')
