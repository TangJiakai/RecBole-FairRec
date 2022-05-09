#!/usr/bin/env python3.7

# Copyright 2020, Gurobi Optimization, LLC

# This example reads a MIP model from a file, solves it in batch mode,
# and prints the JSON solution string.
#
# You will need a Cluster Manager license for this example to work.

import sys
import time
import json
import gurobipy as gp
from gurobipy import GRB


# Set up the environment for batch mode optimization.
#
# The function creates an empty environment, sets all necessary parameters,
# and returns the ready-to-be-started Env object to caller.   It is the
# caller's responsibility to dispose of this environment when it's no
# longer needed.
def setupbatchenv():

    env = gp.Env(empty=True)
    env.setParam('LogFile',        'batchmode.log')
    env.setParam('CSManager',      'http://localhost:61080')
    env.setParam('UserName',       'gurobi')
    env.setParam('ServerPassword', 'pass')
    env.setParam('CSBatchMode',    1)

    # No network communication happened up to this point.  This will happen
    # once the caller invokes the start() method of the returned Env object.

    return env


# Print batch job error information, if any
def printbatcherrorinfo(batch):

    if batch is None or batch.BatchErrorCode == 0:
        return

    print("Batch ID {}: Error code {} ({})".format(
        batch.BatchID, batch.BatchErrorCode, batch.BatchErrorMessage))


# Create a batch request for given problem file
def newbatchrequest(filename):

    # Start environment, create Model object from file
    #
    # By using the context handlers for env and model, it is ensured that
    # model.dispose() and env.dispose() are called automatically
    with setupbatchenv().start() as env, gp.read(filename, env=env) as model:
        # Set some parameters
        model.Params.MIPGap = 0.01
        model.Params.JSONSolDetail = 1

        # Define tags for some variables in order to access their values later
        for count, v in enumerate(model.getVars()):
            v.VTag = "Variable{}".format(count)
            if count >= 10:
                break

        # Submit batch request
        batchID = model.optimizeBatch()

    return batchID


# Wait for the final status of the batch.
# Initially the status of a batch is "submitted"; the status will change
# once the batch has been processed (by a compute server).
def waitforfinalstatus(batchID):
    # Wait no longer than one hour
    maxwaittime = 3600

    # Setup and start environment, create local Batch handle object
    with setupbatchenv().start() as env, gp.Batch(batchID, env) as batch:

        starttime = time.time()
        while batch.BatchStatus == GRB.BATCH_SUBMITTED:
            # Abort this batch if it is taking too long
            curtime = time.time()
            if curtime - starttime > maxwaittime:
                batch.abort()
                break

            # Wait for two seconds
            time.sleep(2)

            # Update the resident attribute cache of the Batch object with the
            # latest values from the cluster manager.
            batch.update()

            # If the batch failed, we retry it
            if batch.BatchStatus == GRB.BATCH_FAILED:
                batch.retry()

        # Print information about error status of the job that processed the batch
        printbatcherrorinfo(batch)


def printfinalreport(batchID):
    # Setup and start environment, create local Batch handle object
    with setupbatchenv().start() as env, gp.Batch(batchID, env) as batch:
        if batch.BatchStatus == GRB.BATCH_CREATED:
            print("Batch status is 'CREATED'")
        elif batch.BatchStatus == GRB.BATCH_SUBMITTED:
            print("Batch is 'SUBMITTED")
        elif batch.BatchStatus == GRB.BATCH_ABORTED:
            print("Batch is 'ABORTED'")
        elif batch.BatchStatus == GRB.BATCH_FAILED:
            print("Batch is 'FAILED'")
        elif batch.BatchStatus == GRB.BATCH_COMPLETED:
            print("Batch is 'COMPLETED'")
            print("JSON solution:")
            # Get JSON solution as string, create dict from it
            sol = json.loads(batch.getJSONSolution())

            # Pretty printing the general solution information
            print(json.dumps(sol["SolutionInfo"], indent=4))

            # Write the full JSON solution string to a file
            batch.writeJSONSolution('batch-sol.json.gz')
        else:
            # Should not happen
            print("Batch has unknown BatchStatus")

        printbatcherrorinfo(batch)


# Instruct the cluster manager to discard all data relating to this BatchID
def batchdiscard(batchID):
    # Setup and start environment, create local Batch handle object
    with setupbatchenv().start() as env, gp.Batch(batchID, env) as batch:
        # Remove batch request from manager
        batch.discard()


# Solve a given model using batch optimization
if __name__ == '__main__':

    # Ensure we have an input file
    if len(sys.argv) < 2:
        print("Usage: {} filename".format(sys.argv[0]))
        sys.exit(0)

    # Submit new batch request
    batchID = newbatchrequest(sys.argv[1])

    # Wait for final status
    waitforfinalstatus(batchID)

    # Report final status info
    printfinalreport(batchID)

    # Remove batch request from manager
    batchdiscard(batchID)

    print('Batch optimization OK')
