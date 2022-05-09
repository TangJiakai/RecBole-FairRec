/* Copyright 2020, Gurobi Optimization, LLC

This example considers the following nonconvex nonlinear problem

 maximize    2 x    + y
 subject to  exp(x) + 4 sqrt(y) <= 9
             x, y >= 0

 We show you two approaches to solve this:

 1) Use a piecewise-linear approach to handle general function
    constraints (such as exp and sqrt).
    a) Add two variables
       u = exp(x)
       v = sqrt(y)
    b) Compute points (x, u) of u = exp(x) for some step length (e.g., x
       = 0, 1e-3, 2e-3, ..., xmax) and points (y, v) of v = sqrt(y) for
       some step length (e.g., y = 0, 1e-3, 2e-3, ..., ymax). We need to
       compute xmax and ymax (which is easy for this example, but this
       does not hold in general).
    c) Use the points to add two general constraints of type
       piecewise-linear.

 2) Use the Gurobis built-in general function constraints directly (EXP
    and POW). Here, we do not need to compute the points and the maximal
    possible values, which will be done internally by Gurobi.  In this
    approach, we show how to "zoom in" on the optimal solution and
    tighten tolerances to improve the solution quality.
*/

import gurobi.*;

public class GCPWLFunc {

   private static double f(double u) { return Math.exp(u); }
   private static double g(double u) { return Math.sqrt(u); }

   private static void printsol(GRBModel m, GRBVar x, GRBVar y, GRBVar u, GRBVar v)
     throws GRBException {

     assert(m.get(GRB.IntAttr.Status) == GRB.OPTIMAL);
     System.out.println("x = " + x.get(GRB.DoubleAttr.X) + ", u = " + u.get(GRB.DoubleAttr.X));
     System.out.println("y = " + y.get(GRB.DoubleAttr.X) + ", v = " + v.get(GRB.DoubleAttr.X));
     System.out.println("Obj = " + m.get(GRB.DoubleAttr.ObjVal));

     // Calculate violation of exp(x) + 4 sqrt(y) <= 9
     double vio = f(x.get(GRB.DoubleAttr.X)) + 4 * g(y.get(GRB.DoubleAttr.X)) - 9;
     if (vio < 0.0) vio = 0.0;
     System.out.println("Vio = " + vio);
   }

   public static void main(String[] args) {
     try {

      // Create environment

      GRBEnv env = new GRBEnv();

      // Create a new m

      GRBModel m = new GRBModel(env);

      double lb = 0.0, ub = GRB.INFINITY;

      GRBVar x = m.addVar(lb, ub, 0.0, GRB.CONTINUOUS, "x");
      GRBVar y = m.addVar(lb, ub, 0.0, GRB.CONTINUOUS, "y");
      GRBVar u = m.addVar(lb, ub, 0.0, GRB.CONTINUOUS, "u");
      GRBVar v = m.addVar(lb, ub, 0.0, GRB.CONTINUOUS, "v");

      // Set objective

      GRBLinExpr obj = new GRBLinExpr();
      obj.addTerm(2.0, x); obj.addTerm(1.0, y);
      m.setObjective(obj, GRB.MAXIMIZE);

      // Add linear constraint

      GRBLinExpr expr = new GRBLinExpr();
      expr.addTerm(1.0, u); expr.addTerm(4.0, v);
      m.addConstr(expr, GRB.LESS_EQUAL, 9.0, "l1");

   // Approach 1) PWL constraint approach

      double intv = 1e-3;
      double xmax = Math.log(9.0);
      int len = (int) Math.ceil(xmax/intv) + 1;
      double[] xpts = new double[len];
      double[] upts = new double[len];
      for (int i = 0; i < len; i++) {
        xpts[i] = i*intv;
        upts[i] = f(i*intv);
      }
      GRBGenConstr gc1 = m.addGenConstrPWL(x, u, xpts, upts, "gc1");

      double ymax = (9.0/4.0)*(9.0/4.0);
      len = (int) Math.ceil(ymax/intv) + 1;
      double[] ypts = new double[len];
      double[] vpts = new double[len];
      for (int i = 0; i < len; i++) {
        ypts[i] = i*intv;
        vpts[i] = g(i*intv);
      }
      GRBGenConstr gc2  = m.addGenConstrPWL(y, v, ypts, vpts, "gc2");

      // Optimize the model and print solution

      m.optimize();
      printsol(m, x, y, u, v);

   // Approach 2) General function constraint approach with auto PWL
   //             translation by Gurobi

      // restore unsolved state and get rid of PWL constraints
      m.reset();
      m.remove(gc1);
      m.remove(gc2);
      m.update();

      GRBGenConstr gcf1 = m.addGenConstrExp(x, u, "gcf1", null);
      GRBGenConstr gcf2 = m.addGenConstrPow(y, v, 0.5, "gcf2", "");

      m.set(GRB.DoubleParam.FuncPieceLength, 1e-3);

      // Optimize the model and print solution

      m.optimize();
      printsol(m, x, y, u, v);

      // Zoom in, use optimal solution to reduce the ranges and use a smaller
      // pclen=1e-5 to solve it

      double xval = x.get(GRB.DoubleAttr.X);
      double yval = y.get(GRB.DoubleAttr.X);

      x.set(GRB.DoubleAttr.LB, Math.max(x.get(GRB.DoubleAttr.LB), xval-0.01));
      x.set(GRB.DoubleAttr.UB, Math.min(x.get(GRB.DoubleAttr.UB), xval+0.01));
      y.set(GRB.DoubleAttr.LB, Math.max(y.get(GRB.DoubleAttr.LB), yval-0.01));
      y.set(GRB.DoubleAttr.UB, Math.min(y.get(GRB.DoubleAttr.UB), yval+0.01));
      m.update();
      m.reset();

      m.set(GRB.DoubleParam.FuncPieceLength, 1e-5);

      // Optimize the model and print solution

      m.optimize();
      printsol(m, x, y, u, v);

      // Dispose of model and environment

      m.dispose();
      env.dispose();

    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode() + ". " +
          e.getMessage());
    }
  }
}
