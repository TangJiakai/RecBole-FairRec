/* Copyright 2020, Gurobi Optimization, LLC */

/* This example formulates and solves the following simple bilinear model:

     maximize    x
     subject to  x + y + z <= 10
                 x * y <= 2          (bilinear inequality)
                 x * z + y * z == 1  (bilinear equality)
                 x, y, z non-negative (x integral in second version)
*/

import gurobi.*;

public class Bilinear {
  public static void main(String[] args) {
    try {
      GRBEnv    env   = new GRBEnv("bilinear.log");
      GRBModel  model = new GRBModel(env);

      // Create variables

      GRBVar x = model.addVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "x");
      GRBVar y = model.addVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "y");
      GRBVar z = model.addVar(0.0, GRB.INFINITY, 0.0, GRB.CONTINUOUS, "z");

      // Set objective

      GRBLinExpr obj = new GRBLinExpr();
      obj.addTerm(1.0, x);
      model.setObjective(obj, GRB.MAXIMIZE);

      // Add linear constraint: x + y + z <= 10

      GRBLinExpr expr = new GRBLinExpr();
      expr.addTerm(1.0, x); expr.addTerm(1.0, y); expr.addTerm(1.0, z);
      model.addConstr(expr, GRB.LESS_EQUAL, 10.0, "c0");

      // Add bilinear inequality: x * y <= 2

      GRBQuadExpr qexpr = new GRBQuadExpr();
      qexpr.addTerm(1.0, x, y);
      model.addQConstr(qexpr, GRB.LESS_EQUAL, 2.0, "bilinear0");

      // Add bilinear equality: x * z + y * z == 1

      qexpr = new GRBQuadExpr();
      qexpr.addTerm(1.0, x, z);
      qexpr.addTerm(1.0, y, z);
      model.addQConstr(qexpr, GRB.EQUAL, 1.0, "bilinear1");

      // First optimize() call will fail - need to set NonConvex to 2

      try {
        model.optimize();
        assert false;
      } catch (GRBException e) {
        System.out.println("Failed (as expected)");
      }

      // Change parameter and optimize again

      model.set(GRB.IntParam.NonConvex, 2);
      model.optimize();

      System.out.println(x.get(GRB.StringAttr.VarName)
                         + " " +x.get(GRB.DoubleAttr.X));
      System.out.println(y.get(GRB.StringAttr.VarName)
                         + " " +y.get(GRB.DoubleAttr.X));
      System.out.println(z.get(GRB.StringAttr.VarName)
                         + " " +z.get(GRB.DoubleAttr.X));

      System.out.println("Obj: " + model.get(GRB.DoubleAttr.ObjVal) + " " +
                         obj.getValue());
      System.out.println();

      // Constrain x to be integral and solve again

      x.set(GRB.CharAttr.VType, GRB.INTEGER);
      model.optimize();

      System.out.println(x.get(GRB.StringAttr.VarName)
                         + " " +x.get(GRB.DoubleAttr.X));
      System.out.println(y.get(GRB.StringAttr.VarName)
                         + " " +y.get(GRB.DoubleAttr.X));
      System.out.println(z.get(GRB.StringAttr.VarName)
                         + " " +z.get(GRB.DoubleAttr.X));

      System.out.println("Obj: " + model.get(GRB.DoubleAttr.ObjVal) + " " +
                         obj.getValue());
      System.out.println();

      // Dispose of model and environment

      model.dispose();
      env.dispose();

    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode() + ". " +
          e.getMessage());
    }
  }
}
