#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# Assign workers to shifts; each worker may or may not be available on a
# particular day. We use multi-objective optimization to solve the model.
# The highest-priority objective minimizes the sum of the slacks
# (i.e., the total number of uncovered shifts). The secondary objective
# minimizes the difference between the maximum and minimum number of
# shifts worked among all workers.  The second optimization is allowed
# to degrade the first objective by up to the smaller value of 10% and 2 */

import gurobipy as gp
from gurobipy import GRB
import sys

# Sample data
# Sets of days and workers
Shifts = [
    "Mon1", "Tue2", "Wed3", "Thu4", "Fri5", "Sat6", "Sun7", "Mon8", "Tue9",
    "Wed10", "Thu11", "Fri12", "Sat13", "Sun14"
    ]

Workers = ["Amy", "Bob", "Cathy", "Dan", "Ed", "Fred", "Gu", "Tobi"]

# Number of workers required for each shift
S = [3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5]
shiftRequirements = {s: S[i] for i, s in enumerate(Shifts)}

# Worker availability: 0 if the worker is unavailable for a shift
A = [
    [0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1],
    [1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0],
    [0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1],
    [0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1],
    [1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1],
    [0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1],
    [1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    ]

availability = {(w, s): A[j][i]
                for i, s in enumerate(Shifts)
                for j, w in enumerate(Workers)}

try:
    # Create model with a context manager. Upon exit from this block,
    # model.dispose is called automatically, and memory consumed by the model
    # is released.
    #
    # The model is created in the default environment, which will be created
    # automatically upon model construction.  For safe release of resources
    # tied to the default environment, disposeDefaultEnv is called below.
    with gp.Model("workforce5") as model:

        # Initialize assignment decision variables:
        # x[w][s] == 1 if worker w is assigned to shift s.
        # This is no longer a pure assignment model, so we must
        # use binary variables.
        x = model.addVars(availability.keys(), ub=availability,
                          vtype=GRB.BINARY, name='x')

        # Slack variables for each shift constraint so that the shifts can
        # be satisfied
        slacks = model.addVars(Shifts, name='Slack')

        # Variable to represent the total slack
        totSlack = model.addVar(name='totSlack')

        # Variables to count the total shifts worked by each worker
        totShifts = model.addVars(Workers, name='TotShifts')

        # Constraint: assign exactly shiftRequirements[s] workers
        # to each shift s, plus the slack
        model.addConstrs(
            (x.sum('*', s) + slacks[s] == shiftRequirements[s] for s in Shifts),
            name='shiftRequirement')

        # Constraint: set totSlack equal to the total slack
        model.addConstr(totSlack == slacks.sum(), name='totSlack')

        # Constraint: compute the total number of shifts for each worker
        model.addConstrs((totShifts[w] == x.sum(w, '*') for w in Workers),
                         name='totShifts')

        # Constraint: set minShift/maxShift variable to less/greater than the
        # number of shifts among all workers
        minShift = model.addVar(name='minShift')
        maxShift = model.addVar(name='maxShift')
        model.addGenConstrMin(minShift, totShifts, name='minShift')
        model.addGenConstrMax(maxShift, totShifts, name='maxShift')

        # Set global sense for ALL objectives
        model.ModelSense = GRB.MINIMIZE

        # Set up primary objective
        model.setObjectiveN(totSlack, index=0, priority=2, abstol=2.0,
                            reltol=0.1, name='TotalSlack')

        # Set up secondary objective
        model.setObjectiveN(maxShift - minShift, index=1, priority=1,
                            name='Fairness')

        # Save problem
        model.write('workforce5.lp')

        # Optimize
        model.optimize()

        status = model.Status

        if status in (GRB.INF_OR_UNBD, GRB.INFEASIBLE, GRB.UNBOUNDED):
            print('Model cannot be solved because it is infeasible or unbounded')
            sys.exit(0)

        if status != GRB.OPTIMAL:
            print('Optimization was stopped with status ' + str(status))
            sys.exit(0)

        # Print total slack and the number of shifts worked for each worker
        print('')
        print('Total slack required: ' + str(totSlack.X))
        for w in Workers:
            print(w + ' worked ' + str(totShifts[w].X) + ' shifts')
        print('')

except gp.GurobiError as e:
    print('Error code ' + str(e.errno) + ": " + str(e))
except AttributeError as e:
    print('Encountered an attribute error: ' + str(e))
finally:
    # Safely release memory and/or server side resources consumed by
    # the default environment.
    gp.disposeDefaultEnv()
