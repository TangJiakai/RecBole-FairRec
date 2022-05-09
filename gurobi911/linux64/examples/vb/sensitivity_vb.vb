' Copyright 2020, Gurobi Optimization, LLC

' A simple sensitivity analysis example which reads a MIP model from a
' file and solves it. Then uses the scenario feature to analyze the impact
' w.r.t. the objective function of each binary variable if it is set to
' 1-X, where X is its value in the optimal solution.
'
' Usage:
'     sensitivity_cs <model filename>

Imports System
Imports Gurobi

Class sensitivity_vb
   Shared Sub Main(args As String())

   If args.Length < 1 Then
      Console.Out.WriteLine("Usage: sensitivity_vb filename")
      Return
   End If

   Try
      ' Maximum number of scenarios to be considered
      Dim MAXSCENARIOS as Integer = 100

      ' Create environment
      Dim env As New GRBEnv()

      ' Read model
      Dim model As New GRBModel(env, args(0))

      Dim scenarios As Integer

      If model.IsMIP = 0 Then
         Console.WriteLine("Model is not a MIP")
         Return
      End If

      ' Solve model
      model.Optimize()

      If model.Status <> GRB.Status.OPTIMAL Then
         Console.WriteLine("Optimization ended with status " & _
                           model.Status)
         Return
      End If

      ' Store the optimal solution
      Dim origObjVal As Double = model.ObjVal
      Dim vars As GRBVar()     = model.GetVars()
      Dim origX As Double()    = model.Get(GRB.DoubleAttr.X, vars)

      scenarios = 0

      ' Count number of unfixed, binary variables in model. For each we
      ' create a scenario.
      For i As Integer = 0 To vars.Length - 1
         Dim v As GRBVar   = vars(i)
         Dim vType As Char = v.VType

         If v.LB = 0.0                                      AndAlso _
            v.UB = 1.0                                      AndAlso _
            (vType = GRB.BINARY OrElse vType = GRB.INTEGER)           Then
            scenarios += 1

            If scenarios >= MAXSCENARIOS Then
               Exit For
            End If
         End If
      Next

      Console.WriteLine("###  construct multi-scenario model with " _
                        & scenarios & " scenarios")

      ' Set the number of scenarios in the model */
      model.NumScenarios = scenarios

      scenarios = 0

      ' Create a (single) scenario model by iterating through unfixed
      ' binary variables in the model and create for each of these
      ' variables a scenario by fixing the variable to 1-X, where X is its
      ' value in the computed optimal solution
      For i As Integer = 0 To vars.Length - 1
         Dim v As GRBVar   = vars(i)
         Dim vType As Char = v.VType

         If v.LB = 0.0                                      AndAlso _
            v.UB = 1.0                                      AndAlso _
            (vType = GRB.BINARY OrElse vType = GRB.INTEGER) AndAlso _
            scenarios < MAXSCENARIOS                                  Then

            ' Set ScenarioNumber parameter to select the corresponding
            ' scenario for adjustments
            model.Parameters.ScenarioNumber = scenarios

            ' Set variable to 1-X, where X is its value in the optimal solution */
            If origX(i) < 0.5 Then
               v.ScenNLB = 1.0
            Else
               v.ScenNUB = 0.0
            End If

            scenarios += 1
         Else
            ' Add MIP start for all other variables using the optimal solution
            ' of the base model
            v.Start =  origX(i)
         End If
      Next

      ' Solve multi-scenario model
      model.Optimize()

      ' In case we solved the scenario model to optimality capture the
      ' sensitivity information
      If model.Status = GRB.Status.OPTIMAL Then
         Dim modelSense As Integer = model.ModelSense

         scenarios = 0

         For i As Integer = 0 To vars.Length - 1
            Dim v As GRBVar   = vars(i)
            Dim vType As Char = v.VType

            If v.LB = 0.0                                      AndAlso _
               v.UB = 1.0                                      AndAlso _
               (vType = GRB.BINARY OrElse vType = GRB.INTEGER)           Then

               ' Set scenario parameter to collect the objective value of the
               ' corresponding scenario
               model.Parameters.ScenarioNumber = scenarios

               ' Collect objective value and bound for the scenario
               Dim scenarioObjVal As Double = model.ScenNObjVal
               Dim scenarioObjBound As Double = model.ScenNObjBound

               Console.Write("Objective sensitivity for variable " _
                             & v.VarName & " is ")

               ' Check if we found a feasible solution for this scenario
               If scenarioObjVal >= modelSense * GRB.INFINITY Then
                  ' Check if the scenario is infeasible
                  If scenarioObjBound >= modelSense * GRB.INFINITY Then
                     Console.WriteLine("infeasible")
                  Else
                     Console.WriteLine("unknown (no solution available)")
                  End If
               Else
                  ' Scenario is feasible and a solution is available
                  Console.WriteLine(modelSense * (scenarioObjVal - origObjVal))
               End If

               scenarios += 1

               If scenarios >= MAXSCENARIOS Then
                  Exit For

               End If
            End If
         Next
      End If

      ' Dispose of model and environment
      model.Dispose()
      env.Dispose()

   Catch e As GRBException
      Console.WriteLine("Error code: " + e.ErrorCode)
      Console.WriteLine(e.Message)
      Console.WriteLine(e.StackTrace)
   End Try
End Sub
End Class
