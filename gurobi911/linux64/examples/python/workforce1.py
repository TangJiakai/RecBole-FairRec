#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# Assign workers to shifts; each worker may or may not be available on a
# particular day. If the problem cannot be solved, use IIS to find a set of
# conflicting constraints. Note that there may be additional conflicts besides
# what is reported via IIS.

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
# Since an assignment model always produces integer solutions, we use
# continuous variables and solve as an LP.
x = m.addVars(availability, ub=1, name="x")

# The objective is to minimize the total pay costs
m.setObjective(gp.quicksum(pay[w]*x[w, s] for w, s in availability), GRB.MINIMIZE)

# Constraints: assign exactly shiftRequirements[s] workers to each shift s
reqCts = m.addConstrs((x.sum('*', s) == shiftRequirements[s]
                      for s in shifts), "_")

# Using Python looping constructs, the preceding statement would be...
#
# reqCts = {}
# for s in shifts:
#   reqCts[s] = m.addConstr(
#        gp.quicksum(x[w,s] for w,s in availability.select('*', s)) ==
#        shiftRequirements[s], s)

# Save model
m.write('workforce1.lp')

# Optimize
m.optimize()
status = m.status
if status == GRB.UNBOUNDED:
    print('The model cannot be solved because it is unbounded')
    sys.exit(0)
if status == GRB.OPTIMAL:
    print('The optimal objective is %g' % m.objVal)
    sys.exit(0)
if status != GRB.INF_OR_UNBD and status != GRB.INFEASIBLE:
    print('Optimization was stopped with status %d' % status)
    sys.exit(0)

# do IIS
print('The model is infeasible; computing IIS')
m.computeIIS()
if m.IISMinimal:
    print('IIS is minimal\n')
else:
    print('IIS is not minimal\n')
print('\nThe following constraint(s) cannot be satisfied:')
for c in m.getConstrs():
    if c.IISConstr:
        print('%s' % c.constrName)
