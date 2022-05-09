
' Copyright 2020, Gurobi Optimization, LLC

' In this example we show the use of general constraints for modeling
' some common expressions. We use as an example a SAT-problem where we
' want to see if it is possible to satisfy at least four (or all) clauses
' of the logical for
'
' L = (x0 or ~x1 or x2)  and (x1 or ~x2 or x3)  and
'     (x2 or ~x3 or x0)  and (x3 or ~x0 or x1)  and
'     (~x0 or ~x1 or x2) and (~x1 or ~x2 or x3) and
'     (~x2 or ~x3 or x0) and (~x3 or ~x0 or x1)
'
' We do this by introducing two variables for each literal (itself and its
' negated value), a variable for each clause, and then two
' variables for indicating if we can satisfy four, and another to identify
' the minimum of the clauses (so if it one, we can satisfy all clauses)
' and put these two variables in the objective.
' i.e. the Objective function will be
'
' maximize Obj0 + Obj1
'
'  Obj0 = MIN(Clause1, ... , Clause8)
'  Obj1 = 1 -> Clause1 + ... + Clause8 >= 4
'
' thus, the objective value will be two if and only if we can satisfy all
' clauses; one if and only of at least four clauses can be satisfied, and
' zero otherwise.
'

Imports Gurobi

Class genconstr_vb

    Public Const n As Integer = 4
    Public Const NLITERALS As Integer = 4  'same as n
    Public Const NCLAUSES As Integer = 8
    Public Const NOBJ As Integer = 2

    Shared Sub Main()

        Try

            ' Example data:
            ' e.g. {0, n+1, 2} means clause (x0 or ~x1 or x2)
            Dim Clauses As Integer(,) = New Integer(,) { _
                {    0, n + 1, 2}, {    1, n + 2, 3}, _
                {    2, n + 3, 0}, {    3, n + 0, 1}, _
                {n + 0, n + 1, 2}, {n + 1, n + 2, 3}, _
                {n + 2, n + 3, 0}, {n + 3, n + 0, 1}}

            Dim i As Integer, status As Integer

            ' Create environment
            Dim env As New GRBEnv("genconstr_vb.log")

            ' Create initial model
            Dim model As New GRBModel(env)
            model.ModelName = "genconstr_vb"

            ' Initialize decision variables and objective
            Dim Lit As GRBVar() = New GRBVar(NLITERALS - 1) {}
            Dim NotLit As GRBVar() = New GRBVar(NLITERALS - 1) {}
            For i = 0 To NLITERALS - 1
                Lit(i) = model.AddVar(0.0, 1.0, 0.0, GRB.BINARY, String.Format("X{0}", i))
                NotLit(i) = model.AddVar(0.0, 1.0, 0.0, GRB.BINARY, String.Format("notX{0}", i))
            Next

            Dim Cla As GRBVar() = New GRBVar(NCLAUSES - 1) {}
            For i = 0 To NCLAUSES - 1
                Cla(i) = model.AddVar(0.0, 1.0, 0.0, GRB.BINARY, String.Format("Clause{0}", i))
            Next

            Dim Obj As GRBVar() = New GRBVar(NOBJ - 1) {}
            For i = 0 To NOBJ - 1
                Obj(i) = model.AddVar(0.0, 1.0, 1.0, GRB.BINARY, String.Format("Obj{0}", i))
            Next

            ' Link Xi and notXi
            Dim lhs As GRBLinExpr
            For i = 0 To NLITERALS - 1
                lhs = New GRBLinExpr()
                lhs.AddTerm(1.0, Lit(i))
                lhs.AddTerm(1.0, NotLit(i))
                model.AddConstr(lhs, GRB.EQUAL, 1.0, String.Format("CNSTR_X{0}", i))
            Next

            ' Link clauses and literals
            For i = 0 To NCLAUSES - 1
                Dim clause As GRBVar() = New GRBVar(2) {}
                For j As Integer = 0 To 2
                    If Clauses(i, j) >= n Then
                        clause(j) = NotLit(Clauses(i, j) - n)
                    Else
                        clause(j) = Lit(Clauses(i, j))
                    End If
                Next
                model.AddGenConstrOr(Cla(i), clause, String.Format("CNSTR_Clause{0}", i))
            Next

            ' Link objs with clauses
            model.AddGenConstrMin(Obj(0), Cla, GRB.INFINITY, "CNSTR_Obj0")
            lhs = New GRBLinExpr()
            For i = 0 To NCLAUSES - 1
                lhs.AddTerm(1.0, Cla(i))
            Next
            model.AddGenConstrIndicator(Obj(1), 1, lhs, GRB.GREATER_EQUAL, 4.0, "CNSTR_Obj1")

            ' Set global objective sense
            model.ModelSense = GRB.MAXIMIZE

            ' Save problem
            model.Write("genconstr_vb.mps")
            model.Write("genconstr_vb.lp")

            ' Optimize
            model.Optimize()

            ' Status checking
            status = model.Status

            If status = GRB.Status.INF_OR_UNBD OrElse _
               status = GRB.Status.INFEASIBLE OrElse _
               status = GRB.Status.UNBOUNDED Then
                Console.WriteLine("The model cannot be solved " & _
                         "because it is infeasible or unbounded")
                Return
            End If

            If status <> GRB.Status.OPTIMAL Then
                Console.WriteLine("Optimization was stopped with status {0}", status)
                Return
            End If

            ' Print result
            Dim objval As Double = model.ObjVal

            If objval > 1.9 Then
                Console.WriteLine("Logical expression is satisfiable")
            ElseIf objval > 0.9 Then
                Console.WriteLine("At least four clauses can be satisfied")
            Else
                Console.WriteLine("Not even three clauses can be satisfied")
            End If

            ' Dispose of model and environment
            model.Dispose()
            env.Dispose()

        Catch e As GRBException
            Console.WriteLine("Error code: {0}. {1}", e.ErrorCode, e.Message)

        End Try
    End Sub
End Class
