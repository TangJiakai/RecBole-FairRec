#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# Assign workers to shifts; each worker may or may not be available on a
# particular day.  The optimization problem is solved as a batch, and
# the schedule constructed only from the meta data available in the solution
# JSON.
#
# NOTE: You'll need a license file configured to use a Cluster Manager
#       for this example to run.

import time
import json
import sys
import gurobipy as gp
from gurobipy import GRB
from collections import OrderedDict, defaultdict


# For later pretty printing names for the shifts
shiftname = OrderedDict([
    ("Mon1",  "Monday 8:00"),
    ("Mon8",  "Monday 14:00"),
    ("Tue2",  "Tuesday 8:00"),
    ("Tue9",  "Tuesday 14:00"),
    ("Wed3",  "Wednesday 8:00"),
    ("Wed10", "Wednesday 14:00"),
    ("Thu4",  "Thursday 8:00"),
    ("Thu11", "Thursday 14:00"),
    ("Fri5",  "Friday 8:00"),
    ("Fri12", "Friday 14:00"),
    ("Sat6",  "Saturday 8:00"),
    ("Sat13", "Saturday 14:00"),
    ("Sun7",  "Sunday 9:00"),
    ("Sun14", "Sunday 12:00"),
    ])


# Build the assignment problem in a Model, and submit it for batch optimization
#
# Required input: A Cluster Manager environment setup for batch optimization
def submit_assigment_problem(env):
    # Number of workers required for each shift
    shifts, shiftRequirements = gp.multidict({
        "Mon1":  3,
        "Tue2":  2,
        "Wed3":  4,
        "Thu4":  4,
        "Fri5":  5,
        "Sat6":  5,
        "Sun7":  3,
        "Mon8":  2,
        "Tue9":  2,
        "Wed10": 3,
        "Thu11": 4,
        "Fri12": 5,
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
        ('Amy', 'Tue2'), ('Amy', 'Wed3'), ('Amy', 'Thu4'), ('Amy', 'Sun7'),
        ('Amy', 'Tue9'), ('Amy', 'Wed10'), ('Amy', 'Thu11'), ('Amy', 'Fri12'),
        ('Amy', 'Sat13'), ('Amy', 'Sun14'), ('Bob', 'Mon1'), ('Bob', 'Tue2'),
        ('Bob', 'Fri5'), ('Bob', 'Sat6'), ('Bob', 'Mon8'), ('Bob', 'Thu11'),
        ('Bob', 'Sat13'), ('Cathy', 'Wed3'), ('Cathy', 'Thu4'),
        ('Cathy', 'Fri5'), ('Cathy', 'Sun7'), ('Cathy', 'Mon8'),
        ('Cathy', 'Tue9'), ('Cathy', 'Wed10'), ('Cathy', 'Thu11'),
        ('Cathy', 'Fri12'), ('Cathy', 'Sat13'), ('Cathy', 'Sun14'),
        ('Dan', 'Tue2'), ('Dan', 'Thu4'), ('Dan', 'Fri5'), ('Dan', 'Sat6'),
        ('Dan', 'Mon8'), ('Dan', 'Tue9'), ('Dan', 'Wed10'), ('Dan', 'Thu11'),
        ('Dan', 'Fri12'), ('Dan', 'Sat13'), ('Dan', 'Sun14'), ('Ed', 'Mon1'),
        ('Ed', 'Tue2'), ('Ed', 'Wed3'), ('Ed', 'Thu4'), ('Ed', 'Fri5'),
        ('Ed', 'Sat6'), ('Ed', 'Mon8'), ('Ed', 'Tue9'), ('Ed', 'Thu11'),
        ('Ed', 'Sat13'), ('Ed', 'Sun14'), ('Fred', 'Mon1'), ('Fred', 'Tue2'),
        ('Fred', 'Wed3'), ('Fred', 'Sat6'), ('Fred', 'Mon8'), ('Fred', 'Tue9'),
        ('Fred', 'Fri12'), ('Fred', 'Sat13'), ('Fred', 'Sun14'),
        ('Gu', 'Mon1'), ('Gu', 'Tue2'), ('Gu', 'Wed3'), ('Gu', 'Fri5'),
        ('Gu', 'Sat6'), ('Gu', 'Sun7'), ('Gu', 'Mon8'), ('Gu', 'Tue9'),
        ('Gu', 'Wed10'), ('Gu', 'Thu11'), ('Gu', 'Fri12'), ('Gu', 'Sat13'),
        ('Gu', 'Sun14')
        ])

    # Start environment, get model in this environment
    with gp.Model("assignment", env=env) as m:
        # Assignment variables: x[w,s] == 1 if worker w is assigned to shift s.
        # Since an assignment model always produces integer solutions, we use
        # continuous variables and solve as an LP.
        x = m.addVars(availability, ub=1, name="x")

        # Set tags encoding the assignments for later retrieval of the schedule.
        # Each tag is a JSON string of the format
        #   {
        #     "Worker": "<Name of the worker>",
        #     "Shift":  "String representation of the shift"
        #   }
        #
        for k, v in x.items():
            name, timeslot = k
            d = {"Worker": name, "Shift": shiftname[timeslot]}
            v.VTag = json.dumps(d)

        # The objective is to minimize the total pay costs
        m.setObjective(gp.quicksum(pay[w]*x[w, s] for w, s in availability),
                       GRB.MINIMIZE)

        # Constraints: assign exactly shiftRequirements[s] workers to each shift
        reqCts = m.addConstrs((x.sum('*', s) == shiftRequirements[s]
                              for s in shifts), "_")

        # Submit this model for batch optimization to the cluster manager
        # and return its batch ID for later querying the solution
        batchID = m.optimizeBatch()

    return batchID


# Wait for the final status of the batch.
# Initially the status of a batch is "submitted"; the status will change
# once the batch has been processed (by a compute server).
def waitforfinalbatchstatus(batch):
    # Wait no longer than ten seconds
    maxwaittime = 10

    starttime = time.time()
    while batch.BatchStatus == GRB.BATCH_SUBMITTED:

        # Abort this batch if it is taking too long
        curtime = time.time()
        if curtime - starttime > maxwaittime:
            batch.abort()
            break

        # Wait for one second
        time.sleep(1)

        # Update the resident attribute cache of the Batch object with the
        # latest values from the cluster manager.
        batch.update()


# Print the schedule according to the solution in the given dict
def print_shift_schedule(soldict):
    schedule = defaultdict(list)

    # Iterate over the variables that take a non-zero value (i.e.,
    # an assignment), and collect them per day
    for v in soldict['Vars']:
        # There is only one VTag, the JSON dict of an assignment we passed
        # in as the VTag
        assignment = json.loads(v["VTag"][0])
        schedule[assignment["Shift"]].append(assignment["Worker"])

    # Print the schedule
    for k in shiftname.values():
        day, time = k.split()
        workers = ", ".join(schedule[k])
        print(" - {:10} {:>5}: {}".format(day, time, workers))


if __name__ == '__main__':
    # Create Cluster Manager environment in batch mode.
    env = gp.Env(empty=True)
    env.setParam('CSBatchMode', 1)

    # env is a context manager; upon leaving, Env.dispose() is called
    with env.start():
        # Submit the assignment problem to the cluster manager, get batch ID
        batchID = submit_assigment_problem(env)

        # Create a batch object, wait for batch to complete, query solution JSON
        with gp.Batch(batchID, env) as batch:
            waitforfinalbatchstatus(batch)

            if batch.BatchStatus != GRB.BATCH_COMPLETED:
                print("Batch request couldn't be completed")
                sys.exit(0)

            jsonsol = batch.getJSONSolution()

    # Dump JSON solution string into a dict
    soldict = json.loads(jsonsol)

    # Has the assignment problem been solved as expected?
    if soldict['SolutionInfo']['Status'] != GRB.OPTIMAL:
        # Shouldn't happen...
        print("Assignment problem could  not be solved to optimality")
        sys.exit(0)

    # Print shift schedule from solution JSON
    print_shift_schedule(soldict)
