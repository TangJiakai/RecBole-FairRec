' Copyright 2020, Gurobi Optimization, LLC
'
' This example considers the following nonconvex nonlinear problem
'
'  maximize    2 x    + y
'  subject to  exp(x) + 4 sqrt(y) <= 9
'              x, y >= 0
'
'  We show you two approaches to solve this:
'
'  1) Use a piecewise-linear approach to handle general function
'     constraints (such as exp and sqrt).
'     a) Add two variables
'        u = exp(x)
'        v = sqrt(y)
'     b) Compute points (x, u) of u = exp(x) for some step length (e.g., x
'        = 0, 1e-3, 2e-3, ..., xmax) and points (y, v) of v = sqrt(y) for
'        some step length (e.g., y = 0, 1e-3, 2e-3, ..., ymax). We need to
'        compute xmax and ymax (which is easy for this example, but this
'        does not hold in general).
'     c) Use the points to add two general constraints of type
'        piecewise-linear.
'
'  2) Use the Gurobis built-in general function constraints directly (EXP
'     and POW). Here, we do not need to compute the points and the maximal
'     possible values, which will be done internally by Gurobi.  In this
'     approach, we show how to "zoom in" on the optimal solution and
'     tighten tolerances to improve the solution quality.

Imports System
Imports Gurobi

Class gc_pwl_func_vb

    Shared Function f(u As Double) As Double
        Return Math.Exp(u)
    End Function
    Shared Function g(u As Double) As Double
        Return Math.Sqrt(u)
    End Function

    Shared Sub printsol(m As GRBModel, x As GRBVar, _
                        y As GRBVar, u As GRBVar, v As GRBVar)
        Console.WriteLine("x = " & x.X & ", u = " & u.X)
        Console.WriteLine("y = " & y.X & ", v = " & v.X)
        Console.WriteLine("Obj = " & m.ObjVal)

        ' Calculate violation of exp(x) + 4 sqrt(y) <= 9
        Dim vio As Double = f(x.X) + 4 * g(y.X) - 9
        If vio < 0.0 Then
            vio = 0.0
        End If
        Console.WriteLine("Vio = " & vio)
    End Sub

    Shared Sub Main()
        Try

            ' Create environment

            Dim env As New GRBEnv()

            ' Create a new m

            Dim m As New GRBModel(env)

            Dim lb As Double = 0.0
            Dim ub As Double = GRB.INFINITY

            Dim x As GRBVar = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "x")
            Dim y As GRBVar = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "y")
            Dim u As GRBVar = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "u")
            Dim v As GRBVar = m.AddVar(lb, ub, 0.0, GRB.CONTINUOUS, "v")

            ' Set objective

            m.SetObjective(2*x + y, GRB.MAXIMIZE)

            ' Add linear constraint

            m.AddConstr(u + 4*v <= 9, "l1")

        ' PWL constraint approach

            Dim intv As Double = 1e-3
            Dim xmax As Double = Math.Log(9.0)
            Dim npts As Integer = Math.Ceiling(xmax/intv) + 1
            Dim xpts As Double() = new Double(npts -1) {}
            Dim upts As Double() = new Double(npts -1) {}

            For i As Integer = 0 To npts - 1
                xpts(i) = i*intv
                upts(i) = f(i*intv)
            Next

            Dim gc1 As GRBGenConstr = m.AddGenConstrPWL(x, u, xpts, upts, "gc1")

            Dim ymax As Double = (9.0/4.0)*(9.0/4.0)
            npts = Math.Ceiling(ymax/intv) + 1
            Dim ypts As Double() = new Double(npts -1) {}
            Dim vpts As Double() = new Double(npts -1) {}

            For i As Integer = 0 To npts - 1
                ypts(i) = i*intv
                vpts(i) = g(i*intv)
            Next

            Dim gc2 As GRBGenConstr = m.AddGenConstrPWL(y, v, ypts, vpts, "gc2")

            ' Optimize the model and print solution

            m.Optimize()
            printsol(m, x, y, u, v)

        ' General function approach with auto PWL translation by Gurobi

            m.Reset()
            m.Remove(gc1)
            m.Remove(gc2)
            m.Update()

            Dim gcf1 As GRBGenConstr = m.AddGenConstrExp(x, u, "gcf1", "")
            Dim gcf2 As GRBGenConstr = m.AddGenConstrPow(y, v, 0.5, "gcf2", "")

            m.Parameters.FuncPieceLength = 1e-3

            ' Optimize the model and print solution

            m.Optimize()
            printsol(m, x, y, u, v)

            ' Use optimal solution to reduce the ranges and use smaller pclen to solve

            x.LB = Math.Max(x.LB, x.X-0.01)
            x.UB = Math.Min(x.UB, x.X+0.01)
            y.LB = Math.Max(y.LB, y.X-0.01)
            y.UB = Math.Min(y.UB, y.X+0.01)
            m.Update()
            m.Reset()

            m.Parameters.FuncPieceLength = 1e-5

            ' Optimize the model and print solution

            m.Optimize()
            printsol(m, x, y, u, v)

            ' Dispose of model and environment

            m.Dispose()
            env.Dispose()
        Catch e As GRBException
            Console.WriteLine("Error code: " + e.ErrorCode + ". " + e.Message)
        End Try
    End Sub
End Class
