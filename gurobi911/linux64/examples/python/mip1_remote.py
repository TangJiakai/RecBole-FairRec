import gurobipy as gp
from gurobipy import GRB

# Variation of mip1.py, with a focus on remote services
#
# When remote resources are tied to the optimization process, such as a token
# server, compute server, or Instant Cloud, extra care should be taken to
# ensure that such resources are released once they are no longer needed.
# Technically, such resources are managed by a gurobipy.Env object
# ("environment").  This example shows best practices for acquiring and
# releasing such shared resources via Env objects.
#
# See also https://www.gurobi.com/documentation/9.1/refman/environments.html

def populate_and_solve(m):
    # This function formulates and solves the following MIP model (see mip1.py):
    #  maximize
    #        x +   y + 2 z
    #  subject to
    #        x + 2 y + 3 z <= 4
    #        x +   y       >= 1
    #        x, y, z binary

    # Create variables
    x = m.addVar(vtype=GRB.BINARY, name="x")
    y = m.addVar(vtype=GRB.BINARY, name="y")
    z = m.addVar(vtype=GRB.BINARY, name="z")

    # Set objective
    m.setObjective(x + y + 2 * z, GRB.MAXIMIZE)

    # Add constraint: x + 2 y + 3 z <= 4
    m.addConstr(x + 2 * y + 3 * z <= 4, "c0")

    # Add constraint: x + y >= 1
    m.addConstr(x + y >= 1, "c1")

    # Optimize model
    m.optimize()

    for v in m.getVars():
        print('%s %g' % (v.varName, v.x))

    print('Obj: %g' % m.objVal)

# Put any connection parameters for Gurobi Compute Server, Gurobi Cluster
# Manager or Gurobi Token server here, unless they are set already
# through the license file.

connection_params = {
# For Compute Server you need at least this
#       "ComputeServer": "<server name>",
#       "Username": "<user name>",
#       "ServerPassword": "<password>",

# For Instant cloud you need at least this
#       "CloudAccessID": "<access id>",
#       "CloudSecretKey": "<secret>",
        }

with gp.Env(params=connection_params) as env:
    # 'env' is now set up according to the connection parameters.
    # The environment is disposed of automatically through the context manager
    # upon leaving this block.
    with gp.Model(env=env) as model:
        # 'model' is now an instance tied to the enclosing Env object 'env'.
        # The model is disposed of automatically through the context manager
        # upon leaving this block.
        try:
            populate_and_solve(model)
        except:
            # Add appropriate error handling here.
            raise
