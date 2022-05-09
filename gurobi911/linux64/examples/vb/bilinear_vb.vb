' Copyright 2020, Gurobi Optimization, LLC */

' This example formulates and solves the following simple bilinear model:
'
'     maximize    x
'     subject to  x + y + z <= 10
'                 x * y <= 2          (bilinear inequality)
'                 x * z + y * z == 1  (bilinear equality)
'                 x, y, z non-negative (x integral in second version)

Imports Gurobi

Class bilinear_vb
    Shared Sub Main()
        Try
            Dim env As New GRBEnv("bilinear.log")
            Dim model As New GRBModel(env)

            ' Create variables

            Dim x As GRBVar = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, "x")
            Dim y As GRBVar = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, "y")
            Dim z As GRBVar = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, "z")

            ' Set objective
            Dim obj As GRBLinExpr = x
            model.SetObjective(obj, GRB.MAXIMIZE)

            ' Add linear constraint: x + y + z <= 10

            model.AddConstr(x + y + z <= 10, "c0")

            ' Add bilinear inequality: x * y <= 2

            model.AddQConstr(x * y <= 2, "bilinear0")

            ' Add bilinear equality: x * z + y * z == 1

            model.AddQConstr(x * z + y * z = 1, "bilinear1")

            ' Optimize model

            Try
                model.Optimize()
            Catch e As GRBException
                Console.WriteLine("Failed (as expected)")
            End Try

            model.Set(GRB.IntParam.NonConvex, 2)
            model.Optimize()

            Console.WriteLine(x.VarName & " " & x.X)
            Console.WriteLine(y.VarName & " " & y.X)
            Console.WriteLine(z.VarName & " " & z.X)

            Console.WriteLine("Obj: " & model.ObjVal & " " & obj.Value)

            x.Set(GRB.CharAttr.VType, GRB.INTEGER)
            model.Optimize()

            Console.WriteLine(x.VarName & " " & x.X)
            Console.WriteLine(y.VarName & " " & y.X)
            Console.WriteLine(z.VarName & " " & z.X)

            Console.WriteLine("Obj: " & model.ObjVal & " " & obj.Value)

            ' Dispose of model and env

            model.Dispose()

            env.Dispose()
        Catch e As GRBException
            Console.WriteLine("Error code: " & e.ErrorCode & ". " & e.Message)
        End Try

    End Sub
End Class
