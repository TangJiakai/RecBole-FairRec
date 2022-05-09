/* Copyright 2020, Gurobi Optimization, LLC */

/* In this example we show the use of general constraints for modeling
   some common expressions. We use as an example a SAT-problem where we
   want to see if it is possible to satisfy at least four (or all) clauses
   of the logical for

   L = (x0 or ~x1 or x2)  and (x1 or ~x2 or x3)  and
       (x2 or ~x3 or x0)  and (x3 or ~x0 or x1)  and
       (~x0 or ~x1 or x2) and (~x1 or ~x2 or x3) and
       (~x2 or ~x3 or x0) and (~x3 or ~x0 or x1)

   We do this by introducing two variables for each literal (itself and its
   negated value), a variable for each clause, and then two
   variables for indicating if we can satisfy four, and another to identify
   the minimum of the clauses (so if it one, we can satisfy all clauses)
   and put these two variables in the objective.
   i.e. the Objective function will be

   maximize Obj0 + Obj1

    Obj0 = MIN(Clause1, ... , Clause8)
    Obj1 = 1 -> Clause1 + ... + Clause8 >= 4

   thus, the objective value will be two if and only if we can satisfy all
   clauses; one if and only if at least four clauses can be satisfied, and
   zero otherwise.
 */

using System;
using Gurobi;

class genconstr_cs {

  public const int n = 4;
  public const int NLITERALS = 4;  // same as n
  public const int NCLAUSES = 8;
  public const int NOBJ = 2;

  static void Main() {

    try {
      // Example data:
      //   e.g. {0, n+1, 2} means clause (x0 or ~x1 or x2)
      int[,] Clauses = new int[,]
                        {{  0, n+1, 2}, {  1, n+2, 3},
                         {  2, n+3, 0}, {  3, n+0, 1},
                         {n+0, n+1, 2}, {n+1, n+2, 3},
                         {n+2, n+3, 0}, {n+3, n+0, 1}};

      int i, status;

      // Create environment
      GRBEnv env = new GRBEnv("genconstr_cs.log");

      // Create initial model
      GRBModel model = new GRBModel(env);
      model.ModelName = "genconstr_cs";

      // Initialize decision variables and objective

      GRBVar[] Lit     = new GRBVar[NLITERALS];
      GRBVar[] NotLit  = new GRBVar[NLITERALS];
      for (i = 0; i < NLITERALS; i++) {
        Lit[i]    = model.AddVar(0.0, 1.0, 0.0, GRB.BINARY, string.Format("X{0}", i));
        NotLit[i] = model.AddVar(0.0, 1.0, 0.0, GRB.BINARY, string.Format("notX{0}", i));
      }

      GRBVar[] Cla = new GRBVar[NCLAUSES];
      for (i = 0; i < NCLAUSES; i++) {
        Cla[i] = model.AddVar(0.0, 1.0, 0.0, GRB.BINARY, string.Format("Clause{0}", i));
      }

      GRBVar[] Obj = new GRBVar[NOBJ];
      for (i = 0; i < NOBJ; i++) {
        Obj[i] = model.AddVar(0.0, 1.0, 1.0, GRB.BINARY, string.Format("Obj{0}", i));
      }

      // Link Xi and notXi
      GRBLinExpr lhs;
      for (i = 0; i < NLITERALS; i++) {
        lhs = new GRBLinExpr();
        lhs.AddTerm(1.0, Lit[i]);
        lhs.AddTerm(1.0, NotLit[i]);
        model.AddConstr(lhs, GRB.EQUAL, 1.0, string.Format("CNSTR_X{0}", i));
      }

      // Link clauses and literals
      for (i = 0; i < NCLAUSES; i++) {
        GRBVar[] clause = new GRBVar[3];
        for (int j = 0; j < 3; j++) {
          if (Clauses[i,j] >= n) clause[j] = NotLit[Clauses[i,j]-n];
          else                   clause[j] = Lit[Clauses[i,j]];
        }
        model.AddGenConstrOr(Cla[i], clause, string.Format("CNSTR_Clause{0}", i));
      }

      // Link objs with clauses
      model.AddGenConstrMin(Obj[0], Cla, GRB.INFINITY, "CNSTR_Obj0");
      lhs = new GRBLinExpr();
      for (i = 0; i < NCLAUSES; i++) {
        lhs.AddTerm(1.0, Cla[i]);
      }
      model.AddGenConstrIndicator(Obj[1], 1, lhs, GRB.GREATER_EQUAL, 4.0, "CNSTR_Obj1");

      // Set global objective sense
      model.ModelSense = GRB.MAXIMIZE;

      // Save problem
      model.Write("genconstr_cs.mps");
      model.Write("genconstr_cs.lp");

      // Optimize
      model.Optimize();

      // Status checking
      status = model.Status;

      if (status == GRB.Status.INF_OR_UNBD ||
          status == GRB.Status.INFEASIBLE  ||
          status == GRB.Status.UNBOUNDED     ) {
        Console.WriteLine("The model cannot be solved " +
               "because it is infeasible or unbounded");
        return;
      }
      if (status != GRB.Status.OPTIMAL) {
        Console.WriteLine("Optimization was stopped with status {0}", status);
        return;
      }

      // Print result
      double objval = model.ObjVal;

      if (objval > 1.9)
        Console.WriteLine("Logical expression is satisfiable");
      else if (objval > 0.9)
        Console.WriteLine("At least four clauses can be satisfied");
      else
        Console.WriteLine("Not even three clauses can be satisfied");

      // Dispose of model and environment
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: {0}. {1}", e.ErrorCode, e.Message);
    }
  }
}
