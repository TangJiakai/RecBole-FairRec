/* Copyright 2020, Gurobi Optimization, LLC */

/* This example formulates and solves the following simple QCP model:

     maximize    x
     subject to  x + y + z = 1
                 x^2 + y^2 <= z^2 (second-order cone)
                 x^2 <= yz        (rotated second-order cone)
                 x, y, z non-negative
*/

using System;
using Gurobi;

class qcp_cs
{
  static void Main()
  {
    try {
      GRBEnv    env   = new GRBEnv("qcp.log");
      GRBModel  model = new GRBModel(env);

      // Create variables

      GRBVar x = model.AddVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "x");
      GRBVar y = model.AddVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "y");
      GRBVar z = model.AddVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "z");

      // Set objective

      GRBLinExpr obj = x;
      model.SetObjective(obj, GRB.MAXIMIZE);

      // Add linear constraint: x + y + z = 1

      model.AddConstr(x + y + z == 1.0, "c0");

      // Add second-order cone: x^2 + y^2 <= z^2

      model.AddQConstr(x*x + y*y <= z*z, "qc0");

      // Add rotated cone: x^2 <= yz

      model.AddQConstr(x*x <= y*z, "qc1");

      // Optimize model

      model.Optimize();

      Console.WriteLine(x.VarName + " " + x.X);
      Console.WriteLine(y.VarName + " " + y.X);
      Console.WriteLine(z.VarName + " " + z.X);

      Console.WriteLine("Obj: " + model.ObjVal + " " + obj.Value);

      // Dispose of model and env

      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " + e.Message);
    }
  }
}
