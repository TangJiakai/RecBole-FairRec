#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# This example uses the Python matrix API to formulate the n-queens
# problem; it maximizes the number queens placed on an n x n
# chessboard without threatening each other.
#
# This example demonstrates NumPy slicing.

import numpy as np
import scipy.sparse as sp
import gurobipy as gp
from gurobipy import GRB


# Size of the n x n chess board
n = 8

try:
    # Create a new model
    m = gp.Model("matrix2")

    # Create a 2-D array of binary variables
    # x[i,j]=1 means that a queen is placed at square (i,j)
    x = m.addMVar((n, n), vtype=GRB.BINARY, name="x")

    # Set objective - maximize number of queens
    m.setObjective(x.sum(), GRB.MAXIMIZE)

    # Add row and column constraints
    for i in range(n):

        # At most one queen per row
        m.addConstr(x[i, :].sum() <= 1, name="row"+str(i))

        # At most one queen per column
        m.addConstr(x[:, i].sum() <= 1, name="col"+str(i))

    # Add diagonal constraints
    for i in range(1, 2*n):

        # At most one queen per diagonal
        diagn = (range(max(0, i-n), min(n, i)), range(min(n, i)-1, max(0, i-n)-1, -1))
        m.addConstr(x[diagn].sum() <= 1, name="diag"+str(i))

        # At most one queen per anti-diagonal
        adiagn = (range(max(0, i-n), min(n, i)), range(max(0, n-i), min(n, 2*n-i)))
        m.addConstr(x[adiagn].sum() <= 1, name="adiag"+str(i))

    # Optimize model
    m.optimize()

    print(x.X)
    print('Queens placed: %g' % m.objVal)

except gp.GurobiError as e:
    print('Error code ' + str(e.errno) + ": " + str(e))

except AttributeError:
    print('Encountered an attribute error')
