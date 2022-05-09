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

using System;
using Gurobi;

class gc_pwl_func_cs {

   private static double f(double u) { return Math.Exp(u); }
   private static double g(double u) { return Math.Sqrt(u); }

   private static void printsol(GRBModel m, GRBVar x, GRBVar y, GRBVar u, GRBVar v) {
     Console.WriteLine("x = " + x.X + ", u = " + u.X);
     Console.WriteLine("y = " + y.X + ", v = " + v.X);
     Console.WriteLine("Obj = " + m.ObjVal);

     // Calculate violation of exp(x) + 4 sqrt(y) <= 9
     double vio = f(x.X) + 4 * g(y.X) - 9;
     if (vio < 0.0) vio = 0.0;
     Console.WriteLine("Vio = " + vio);
   }

   static void Main() {
     try {

      // Create environment

      GRBEnv env = new GRBEnv();

      // Create a new m

      GRBModel m = new GRBModel(env);

      double lb = 0.0, ub = GRB.INFINITY;

      GRBVar x = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "x");
      GRBVar y = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "y");
      GRBVar u = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "u");
      GRBVar v = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "v");

      // Set objective

      m.SetObjective(2*x + y, GRB.MAXIMIZE);

      // Add linear constraint

      m.AddConstr(u + 4*v <= 9, "l1");

   // Approach 1) PWL constraint approach

      double intv = 1e-3;
      double xmax = Math.Log(9.0);
      int len = (int) Math.Ceiling(xmax/intv) + 1;
      double[] xpts = new double[len];
      double[] upts = new double[len];
      for (int i = 0; i < len; i++) {
        xpts[i] = i*intv;
        upts[i] = f(i*intv);
      }
      GRBGenConstr gc1 = m.AddGenConstrPWL(x, u, xpts, upts, "gc1");

      double ymax = (9.0/4.0)*(9.0/4.0);
      len = (int) Math.Ceiling(ymax/intv) + 1;
      double[] ypts = new double[len];
      double[] vpts = new double[len];
      for (int i = 0; i < len; i++) {
        ypts[i] = i*intv;
        vpts[i] = g(i*intv);
      }
      GRBGenConstr gc2  = m.AddGenConstrPWL(y, v, ypts, vpts, "gc2");

      // Optimize the model and print solution

      m.Optimize();
      printsol(m, x, y, u, v);

   // Approach 2) General function constraint approach with auto PWL
   //             translation by Gurobi

      // restore unsolved state and get rid of PWL constraints
      m.Reset();
      m.Remove(gc1);
      m.Remove(gc2);
      m.Update();

      GRBGenConstr gcf1 = m.AddGenConstrExp(x, u, "gcf1", "");
      GRBGenConstr gcf2 = m.AddGenConstrPow(y, v, 0.5, "gcf2", "");

      m.Parameters.FuncPieceLength = 1e-3;

      // Optimize the model and print solution

      m.Optimize();
      printsol(m, x, y, u, v);

      // Zoom in, use optimal solution to reduce the ranges and use a smaller
      // pclen=1e-5 to solve it

      x.LB = Math.Max(x.LB, x.X-0.01);
      x.UB = Math.Min(x.UB, x.X+0.01);
      y.LB = Math.Max(y.LB, y.X-0.01);
      y.UB = Math.Min(y.UB, y.X+0.01);
      m.Update();
      m.Reset();

      m.Parameters.FuncPieceLength = 1e-5;

      // Optimize the model and print solution

      m.Optimize();
      printsol(m, x, y, u, v);

      // Dispose of model and environment

      m.Dispose();
      env.Dispose();
    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " + e.Message);
    }
  }
}
