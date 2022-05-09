/* Copyright 2020, Gurobi Optimization, LLC */

/* This example formulates and solves the following simple bilinear model:

     maximize    x
     subject to  x + y + z <= 10
                 x * y <= 2          (bilinear inequality)
                 x * z + y * z == 1  (bilinear equality)
                 x, y, z non-negative (x integral in second version)
*/

using System;
using Gurobi;

class bilinear_cs
{
  static void Main()
  {
    try {
      GRBEnv    env   = new GRBEnv("bilinear.log");
      GRBModel  model = new GRBModel(env);

      // Create variables

      GRBVar x = model.AddVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "x");
      GRBVar y = model.AddVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "y");
      GRBVar z = model.AddVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "z");

      // Set objective

      GRBLinExpr obj = x;
      model.SetObjective(obj, GRB.MAXIMIZE);

      // Add linear constraint: x + y + z <= 10

      model.AddConstr(x + y + z <= 10, "c0");

      // Add bilinear inequality: x * y <= 2

      model.AddQConstr(x*y <= 2, "bilinear0");

      // Add bilinear equality: x * z + y * z == 1

      model.AddQConstr(x*z + y*z == 1, "bilinear1");

      // Optimize model

      try {
        model.Optimize();
      } catch (GRBException e) {
        Console.WriteLine("Failed (as expected) " + e.ErrorCode + ". " + e.Message);
      }

      model.Set(GRB.IntParam.NonConvex, 2);
      model.Optimize();

      Console.WriteLine(x.VarName + " " + x.X);
      Console.WriteLine(y.VarName + " " + y.X);
      Console.WriteLine(z.VarName + " " + z.X);

      Console.WriteLine("Obj: " + model.ObjVal + " " + obj.Value);

      x.Set(GRB.CharAttr.VType, GRB.INTEGER);
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
