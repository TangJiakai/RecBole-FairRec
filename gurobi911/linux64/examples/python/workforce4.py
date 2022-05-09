#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# Assign workers to shifts; each worker may or may not be available on a
# particular day. We use lexicographic optimization to solve the model:
# first, we minimize the linear sum of the slacks. Then, we constrain
# the sum of the slacks, and we minimize a quadratic objective that
# tries to balance the workload among the workers.

import gurobipy as gp
from gurobipy import GRB
import sys

# Number of workers required for each shift
shifts, shiftRequirements = gp.multidict({
    "Mon1":  3,
    "Tue2":  2,
    "Wed3":  4,
    "Thu4":  4,
    "Fri5":  5,
    "Sat6":  6,
    "Sun7":  5,
    "Mon8":  2,
    "Tue9":  2,
    "Wed10": 3,
    "Thu11": 4,
    "Fri12": 6,
    "Sat13": 7,
    "Sun14": 5,
    })

# Amount each worker is paid to work one shift
workers, pay = gp.multidict({
    "Amy":   10,
    "Bob":   12,
    "Cathy": 10,
    "Dan":   8,
    "Ed":    8,
    "Fred":  9,
    "Gu":    11,
    })

# Worker availability
availability = gp.tuplelist([
    ('Amy', 'Tue2'), ('Amy', 'Wed3'), ('Amy', 'Fri5'), ('Amy', 'Sun7'),
    ('Amy', 'Tue9'), ('Amy', 'Wed10'), ('Amy', 'Thu11'), ('Amy', 'Fri12'),
    ('Amy', 'Sat13'), ('Amy', 'Sun14'), ('Bob', 'Mon1'), ('Bob', 'Tue2'),
    ('Bob', 'Fri5'), ('Bob', 'Sat6'), ('Bob', 'Mon8'), ('Bob', 'Thu11'),
    ('Bob', 'Sat13'), ('Cathy', 'Wed3'), ('Cathy', 'Thu4'), ('Cathy', 'Fri5'),
    ('Cathy', 'Sun7'), ('Cathy', 'Mon8'), ('Cathy', 'Tue9'),
    ('Cathy', 'Wed10'), ('Cathy', 'Thu11'), ('Cathy', 'Fri12'),
    ('Cathy', 'Sat13'), ('Cathy', 'Sun14'), ('Dan', 'Tue2'), ('Dan', 'Wed3'),
    ('Dan', 'Fri5'), ('Dan', 'Sat6'), ('Dan', 'Mon8'), ('Dan', 'Tue9'),
    ('Dan', 'Wed10'), ('Dan', 'Thu11'), ('Dan', 'Fri12'), ('Dan', 'Sat13'),
    ('Dan', 'Sun14'), ('Ed', 'Mon1'), ('Ed', 'Tue2'), ('Ed', 'Wed3'),
    ('Ed', 'Thu4'), ('Ed', 'Fri5'), ('Ed', 'Sun7'), ('Ed', 'Mon8'),
    ('Ed', 'Tue9'), ('Ed', 'Thu11'), ('Ed', 'Sat13'), ('Ed', 'Sun14'),
    ('Fred', 'Mon1'), ('Fred', 'Tue2'), ('Fred', 'Wed3'), ('Fred', 'Sat6'),
    ('Fred', 'Mon8'), ('Fred', 'Tue9'), ('Fred', 'Fri12'), ('Fred', 'Sat13'),
    ('Fred', 'Sun14'), ('Gu', 'Mon1'), ('Gu', 'Tue2'), ('Gu', 'Wed3'),
    ('Gu', 'Fri5'), ('Gu', 'Sat6'), ('Gu', 'Sun7'), ('Gu', 'Mon8'),
    ('Gu', 'Tue9'), ('Gu', 'Wed10'), ('Gu', 'Thu11'), ('Gu', 'Fri12'),
    ('Gu', 'Sat13'), ('Gu', 'Sun14')
    ])

# Model
m = gp.Model("assignment")

# Assignment variables: x[w,s] == 1 if worker w is assigned to shift s.
# This is no longer a pure assignment model, so we must use binary variables.
x = m.addVars(availability, vtype=GRB.BINARY, name="x")

# Slack variables for each shift constraint so that the shifts can
# be satisfied
slacks = m.addVars(shifts, name="Slack")

# Variable to represent the total slack
totSlack = m.addVar(name="totSlack")

# Variables to count the total shifts worked by each worker
totShifts = m.addVars(workers, name="TotShifts")

# Constraint: assign exactly shiftRequirements[s] workers to each shift s,
# plus the slack
reqCts = m.addConstrs((slacks[s] + x.sum('*', s) == shiftRequirements[s]
                      for s in shifts), "_")

# Constraint: set totSlack equal to the total slack
m.addConstr(totSlack == slacks.sum(), "totSlack")

# Constraint: compute the total number of shifts for each worker
m.addConstrs((totShifts[w] == x.sum(w) for w in workers), "totShifts")

# Objective: minimize the total slack
# Note that this replaces the previous 'pay' objective coefficients
m.setObjective(totSlack)


# Optimize
def solveAndPrint():
    m.optimize()
    status = m.status
    if status in (GRB.INF_OR_UNBD, GRB.INFEASIBLE, GRB.UNBOUNDED):
        print('The model cannot be solved because it is infeasible or \
               unbounded')
        sys.exit(1)

    if status != GRB.OPTIMAL:
        print('Optimization was stopped with status %d' % status)
        sys.exit(0)

    # Print total slack and the number of shifts worked for each worker
    print('')
    print('Total slack required: %g' % totSlack.x)
    for w in workers:
        print('%s worked %g shifts' % (w, totShifts[w].x))
    print('')


solveAndPrint()

# Constrain the slack by setting its upper and lower bounds
totSlack.ub = totSlack.x
totSlack.lb = totSlack.x

# Variable to count the average number of shifts worked
avgShifts = m.addVar(name="avgShifts")

# Variables to count the difference from average for each worker;
# note that these variables can take negative values.
diffShifts = m.addVars(workers, lb=-GRB.INFINITY, name="Diff")

# Constraint: compute the average number of shifts worked
m.addConstr(len(workers) * avgShifts == totShifts.sum(), "avgShifts")

# Constraint: compute the difference from the average number of shifts
m.addConstrs((diffShifts[w] == totShifts[w] - avgShifts for w in workers),
             "Diff")

# Objective: minimize the sum of the square of the difference from the
# average number of shifts worked
m.setObjective(gp.quicksum(diffShifts[w]*diffShifts[w] for w in workers))

# Optimize
solveAndPrint()
