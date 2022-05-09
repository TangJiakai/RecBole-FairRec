' Copyright 2020, Gurobi Optimization, LLC
'
' This example reads a MIP model from a file, solves it and
' prints the objective values from all feasible solutions
' generated while solving the MIP. Then it creates the fixed
' model and solves that model.


Imports System
Imports Gurobi

Class mip2_vb
    Shared Sub Main(ByVal args As String())

        If args.Length < 1 Then
            Console.WriteLine("Usage: mip2_vb filename")
            Return
        End If

        Try
            Dim env As GRBEnv = New GRBEnv("lp1.log")
            Dim model As GRBModel = New GRBModel(env, args(0))

            If model.IsMIP = 0 Then
                Console.WriteLine("Model is not a MIP")
                Return
            End If

            model.Optimize()

            Dim optimstatus As Integer = model.Status

            If optimstatus = GRB.Status.INF_OR_UNBD Then
                model.Parameters.Presolve = 0
                model.Optimize()
                optimstatus = model.Status
            End If

            Dim objval As Double

            If optimstatus = GRB.Status.OPTIMAL Then
                objval = model.ObjVal
                Console.WriteLine("Optimal objective: " & objval)
            ElseIf optimstatus = GRB.Status.INFEASIBLE Then
                Console.WriteLine("Model is infeasible")
                model.ComputeIIS()
                model.Write("model.ilp")
                Return
            ElseIf optimstatus = GRB.Status.UNBOUNDED Then
                Console.WriteLine("Model is unbounded")
                Return
            Else
                Console.WriteLine("Optimization was stopped with status = " & _
                                  optimstatus)
                Return
            End If

            ' Iterate over the solutions and compute the objectives
            model.Parameters.OutputFlag = 0

            Console.WriteLine()
            For k As Integer = 0 To model.SolCount - 1
                model.Parameters.SolutionNumber = k
                Dim objn As Double = model.PoolObjVal

                Console.WriteLine("Solution " & k & " has objective: " & objn)
            Next
            Console.WriteLine()
            model.Parameters.OutputFlag = 1

            ' Solve fixed model
            Dim fixedmodel As GRBModel = model.FixedModel()
            fixedmodel.Parameters.Presolve = 0
            fixedmodel.Optimize()

            Dim foptimstatus As Integer = fixedmodel.Status
            If foptimstatus <> GRB.Status.OPTIMAL Then
                Console.WriteLine("Error: fixed model isn't optimal")
                Return
            End If

            Dim fobjval As Double = fixedmodel.ObjVal

            If Math.Abs(fobjval - objval) > 0.000001 * (1.0 + Math.Abs(objval)) Then
            End If

            Dim fvars() As GRBVar = fixedmodel.GetVars()
            Dim x() As Double = fixedmodel.Get(GRB.DoubleAttr.X, fvars)
            Dim vnames() As String = fixedmodel.Get(GRB.StringAttr.VarName, fvars)

            For j As Integer = 0 To fvars.Length - 1
                If x(j) <> 0 Then
                    Console.WriteLine(vnames(j) & " " & x(j))
                End If
            Next

            ' Dispose of models and env
            fixedmodel.Dispose()
            model.Dispose()
            env.Dispose()

        Catch e As GRBException
            Console.WriteLine("Error code: " & e.ErrorCode & ". " & e.Message)
        End Try
    End Sub
End Class
