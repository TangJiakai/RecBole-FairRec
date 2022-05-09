/* Copyright 2020, Gurobi Optimization, LLC */

/* This example formulates and solves the following simple bilinear model:

     maximize    x
     subject to  x + y + z <= 10
                 x * y <= 2          (bilinear inequality)
                 x * z + y * z == 1  (bilinear equality)
                 x, y, z non-negative (x integral in second version)
*/
#include <cassert>
#include "gurobi_c++.h"
using namespace std;

int
main(int   argc,
     char *argv[])
{
  try {
    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    // Create variables

    GRBVar x = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "x");
    GRBVar y = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "y");
    GRBVar z = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "z");

    // Set objective

    GRBLinExpr obj = x;
    model.setObjective(obj, GRB_MAXIMIZE);

    // Add linear constraint: x + y + z <= 10

    model.addConstr(x + y + z <= 10, "c0");

    // Add bilinear inequality constraint: x * y <= 2

    model.addQConstr(x*y <= 2, "bilinear0");

    // Add bilinear equality constraint: y * z == 1

    model.addQConstr(x*z + y*z == 1, "bilinear1");

    // First optimize() call will fail - need to set NonConvex to 2

    try {
      model.optimize();
      assert(0);
    } catch (GRBException e) {
      cout << "Failed (as expected)" << endl;
    }

    model.set(GRB_IntParam_NonConvex, 2);
    model.optimize();

    cout << x.get(GRB_StringAttr_VarName) << " "
         << x.get(GRB_DoubleAttr_X) << endl;
    cout << y.get(GRB_StringAttr_VarName) << " "
         << y.get(GRB_DoubleAttr_X) << endl;
    cout << z.get(GRB_StringAttr_VarName) << " "
         << z.get(GRB_DoubleAttr_X) << endl;

    // Constrain x to be integral and solve again
    x.set(GRB_CharAttr_VType, GRB_INTEGER);
    model.optimize();

    cout << x.get(GRB_StringAttr_VarName) << " "
         << x.get(GRB_DoubleAttr_X) << endl;
    cout << y.get(GRB_StringAttr_VarName) << " "
         << y.get(GRB_DoubleAttr_X) << endl;
    cout << z.get(GRB_StringAttr_VarName) << " "
         << z.get(GRB_DoubleAttr_X) << endl;

    cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }

  return 0;
}
