' Copyright 2020, Gurobi Optimization, LLC
'
' This example formulates and solves the following simple model
' with PWL constraints:
'
'  maximize
'        sum c[j] * x[j]
'  subject to
'        sum A[i,j] * x[j] <= 0,  for i = 0, ..., m-1
'        sum y[j] <= 3
'        y[j] = pwl(x[j]),        for j = 0, ..., n-1
'        x[j] free, y[j] >= 0,    for j = 0, ..., n-1
'  where pwl(x) = 0,     if x  = 0
'               = 1+|x|, if x != 0
'
'  Note
'   1. sum pwl(x[j]) <= b is to bound x vector and also to favor sparse x vector.
'      Here b = 3 means that at most two x[j] can be nonzero and if two, then
'      sum x[j] <= 1
'   2. pwl(x) jumps from 1 to 0 and from 0 to 1, if x moves from negatie 0 to 0,
'      then to positive 0, so we need three points at x = 0. x has infinite bounds
'      on both sides, the piece defined with two points (-1, 2) and (0, 1) can
'      extend x to -infinite. Overall we can use five points (-1, 2), (0, 1),
'      (0, 0), (0, 1) and (1, 2) to define y = pwl(x)

Imports System
Imports Gurobi

Class gc_pwl_vb
    Shared Sub Main()
        Try
            Dim n As Integer = 5
            Dim m As Integer = 5
            Dim c As Double() = New Double() {0.5, 0.8, 0.5, 0.1, -1}
            Dim A As Double(,) = New Double(,) {{0, 0, 0, 1, -1}, _
                                                 {0, 0, 1, 1, -1}, _
                                                 {1, 1, 0, 0, -1}, _
                                                 {1, 0, 1, 0, -1}, _
                                                 {1, 0, 0, 1, -1}}
            Dim xpts As Double() = New Double() {-1, 0, 0, 0, 1}
            Dim ypts As Double() = New Double() {2, 1, 0, 1, 2}

            ' Env and model
            Dim env As GRBEnv = New GRBEnv()
            Dim model As GRBModel = New GRBModel(env)
            model.ModelName = "gc_pwl_cs"

            ' Add variables, set bounds and obj coefficients
            Dim x As GRBVar() = model.AddVars(n, GRB.CONTINUOUS)
            For i As Integer = 0 To n - 1
                x(i).LB = -GRB.INFINITY
                x(i).Obj = c(i)
            Next

            Dim y As GRBVar() = model.AddVars(n, GRB.CONTINUOUS)

            ' Set objective to maximize
            model.ModelSense = GRB.MAXIMIZE

            ' Add linear constraints
            For i As Integer = 0 To m - 1
                Dim le As GRBLinExpr = 0.0
                For j As Integer = 0 To n - 1
                    le.AddTerm(A(i, j), x(j))
                Next
                model.AddConstr(le, GRB.LESS_EQUAL, 0, "cx" & i)
            Next

            Dim le1 As GRBLinExpr = 0.0
            For j As Integer = 0 To n - 1
                le1.AddTerm(1.0, y(j))
            Next
            model.AddConstr(le1, GRB.LESS_EQUAL, 3, "cy")

            ' Add piecewise constraints
            For j As Integer = 0 To n - 1
                model.AddGenConstrPWL(x(j), y(j), xpts, ypts, "pwl" & j)
            Next

            ' Optimize model
            model.Optimize()

            For j As Integer = 0 To n - 1
                Console.WriteLine("x[" & j & "] = " & x(j).X)
            Next
            Console.WriteLine("Obj: " & model.ObjVal)

            ' Dispose of model and environment
            model.Dispose()
            env.Dispose()

        Catch e As GRBException
            Console.WriteLine("Error code: " & e.ErrorCode & ". " & e.Message)
        End Try
    End Sub
End Class
