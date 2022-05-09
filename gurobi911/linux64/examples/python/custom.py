#
# Copyright 2020, Gurobi Optimization, LLC
#
# Interactive shell customization example
#
# Define a set of customizations for the Gurobi shell.
# Type 'from custom import *' to import them into your shell.
#

from gurobipy import *


# custom read command -- change directory as appropriate

def myread(name):
    return read('/home/jones/models/' + name)


# Custom termination criterion: Quit optimization
# - after  5s if a high quality (1% gap) solution has been found, or
# - after 10s if a feasible solution has been found.

def mycallback(model, where):
    if where == GRB.Callback.MIP:
        time = model.cbGet(GRB.Callback.RUNTIME)
        best = model.cbGet(GRB.Callback.MIP_OBJBST)
        bound = model.cbGet(GRB.Callback.MIP_OBJBND)

        if best < GRB.INFINITY:
            # We have a feasible solution
            if time > 5 and abs(bound - best) < 0.01 * abs(bound):
                model.terminate()

            if time > 10:
                model.terminate()


# custom optimize() function that uses callback

def myopt(model):
    model.optimize(mycallback)


if __name__ == "__main__":
    # Use as customized command line tool
    import sys
    if len(sys.argv) != 2:
        print("Usage: python custom.py <model>")

    m = read(sys.argv[1])
    myopt(m)
