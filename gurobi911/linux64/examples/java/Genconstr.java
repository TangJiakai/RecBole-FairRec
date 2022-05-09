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


import gurobi.*;

public class Genconstr {

  public static final int n = 4;
  public static final int NLITERALS = 4;  // same as n
  public static final int NCLAUSES = 8;
  public static final int NOBJ = 2;

  public static void main(String[] args) {

    try {
      // Example data:
      //   e.g. {0, n+1, 2} means clause (x0 or ~x1 or x2)
      int Clauses[][] = new int[][]
                        {{  0, n+1, 2}, {  1, n+2, 3},
                         {  2, n+3, 0}, {  3, n+0, 1},
                         {n+0, n+1, 2}, {n+1, n+2, 3},
                         {n+2, n+3, 0}, {n+3, n+0, 1}};

      int i, status, nSolutions;

      // Create environment
      GRBEnv env = new GRBEnv("Genconstr.log");

      // Create initial model
      GRBModel model = new GRBModel(env);
      model.set(GRB.StringAttr.ModelName, "Genconstr");

      // Initialize decision variables and objective

      GRBVar[] Lit     = new GRBVar[NLITERALS];
      GRBVar[] NotLit  = new GRBVar[NLITERALS];
      for (i = 0; i < NLITERALS; i++) {
        Lit[i]    = model.addVar(0.0, 1.0, 0.0, GRB.BINARY, "X" + String.valueOf(i));
        NotLit[i] = model.addVar(0.0, 1.0, 0.0, GRB.BINARY, "notX" + String.valueOf(i));
      }

      GRBVar[] Cla = new GRBVar[NCLAUSES];
      for (i = 0; i < NCLAUSES; i++) {
        Cla[i] = model.addVar(0.0, 1.0, 0.0, GRB.BINARY, "Clause" + String.valueOf(i));
      }

      GRBVar[] Obj = new GRBVar[NOBJ];
      for (i = 0; i < NOBJ; i++) {
        Obj[i] = model.addVar(0.0, 1.0, 1.0, GRB.BINARY, "Obj" + String.valueOf(i));
      }

      // Link Xi and notXi
      GRBLinExpr lhs;
      for (i = 0; i < NLITERALS; i++) {
        lhs = new GRBLinExpr();
        lhs.addTerm(1.0, Lit[i]);
        lhs.addTerm(1.0, NotLit[i]);
        model.addConstr(lhs, GRB.EQUAL, 1.0, "CNSTR_X" + String.valueOf(i));
      }

      // Link clauses and literals
      for (i = 0; i < NCLAUSES; i++) {
        GRBVar[] clause = new GRBVar[3];
        for (int j = 0; j < 3; j++) {
          if (Clauses[i][j] >= n) clause[j] = NotLit[Clauses[i][j]-n];
          else                    clause[j] = Lit[Clauses[i][j]];
        }
        model.addGenConstrOr(Cla[i], clause, "CNSTR_Clause" + String.valueOf(i));
      }

      // Link objs with clauses
      model.addGenConstrMin(Obj[0], Cla, GRB.INFINITY, "CNSTR_Obj0");
      lhs = new GRBLinExpr();
      for (i = 0; i < NCLAUSES; i++) {
        lhs.addTerm(1.0, Cla[i]);
      }
      model.addGenConstrIndicator(Obj[1], 1, lhs, GRB.GREATER_EQUAL, 4.0, "CNSTR_Obj1");

      // Set global objective sense
      model.set(GRB.IntAttr.ModelSense, GRB.MAXIMIZE);

      // Save problem
      model.write("Genconstr.mps");
      model.write("Genconstr.lp");

      // Optimize
      model.optimize();

      // Status checking
      status = model.get(GRB.IntAttr.Status);

      if (status == GRB.INF_OR_UNBD ||
          status == GRB.INFEASIBLE  ||
          status == GRB.UNBOUNDED     ) {
        System.out.println("The model cannot be solved " +
               "because it is infeasible or unbounded");
        System.exit(1);
      }
      if (status != GRB.OPTIMAL) {
        System.out.println("Optimization was stopped with status " + status);
        System.exit(1);
      }

      // Print result
      double objval = model.get(GRB.DoubleAttr.ObjVal);

      if (objval > 1.9)
        System.out.println("Logical expression is satisfiable");
      else if (objval > 0.9)
        System.out.println("At least four clauses can be satisfied");
      else
        System.out.println("Not even three clauses can be satisfied");

      // Dispose of model and environment
      model.dispose();
      env.dispose();

    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode() + ". " +
          e.getMessage());
    }
  }
}
