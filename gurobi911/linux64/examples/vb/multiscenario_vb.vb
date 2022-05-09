' Copyright 2020, Gurobi Optimization, LLC

' Facility location: a company currently ships its product from 5 plants
' to 4 warehouses. It is considering closing some plants to reduce
' costs. What plant(s) should the company close, in order to minimize
' transportation and fixed costs?
'
' Since the plant fixed costs and the warehouse demands are uncertain, a
' scenario approach is chosen.
'
' Note that this example is similar to the facility_vb.vb example. Here we
' added scenarios in order to illustrate the multi-scenario feature.
'
' Based on an example from Frontline Systems:
' http://www.solver.com/disfacility.htm
' Used with permission.

Imports System
Imports Gurobi

Class multiscenario_vb
   Shared Sub Main()
   Try

   ' Warehouse demand in thousands of units
   Dim Demand As Double() = New Double() {15, 18, 14, 20}

   ' Plant capacity in thousands of units
   Dim Capacity As Double() = New Double() {20, 22, 17, 19, 18}

   ' Fixed costs for each plant
   Dim FixedCosts As Double() = New Double() {12000, 15000, 17000, 13000, 16000}

   ' Transportation costs per thousand units
   Dim TransCosts As Double(,) = New Double(,) { {4000, 2000, 3000, 2500, 4500}, _
                                                 {2500, 2600, 3400, 3000, 4000}, _
                                                 {1200, 1800, 2600, 4100, 3000}, _
                                                 {2200, 2600, 3100, 3700, 3200}}

   ' Number of plants and warehouses
   Dim nPlants As Integer = Capacity.Length
   Dim nWarehouses As Integer = Demand.Length

   Dim maxFixed As Double = -GRB.INFINITY
   Dim minFixed As Double = GRB.INFINITY
   For p As Integer = 0 To nPlants - 1
      If FixedCosts(p) > maxFixed Then maxFixed = FixedCosts(p)
      If FixedCosts(p) < minFixed Then minFixed = FixedCosts(p)
   Next

   ' Model
   Dim env As GRBEnv = New GRBEnv()
   Dim model As GRBModel = New GRBModel(env)

   model.ModelName = "multiscenario"

   ' Plant open decision variables: open(p) == 1 if plant p is open.
   Dim open As GRBVar() = New GRBVar(nPlants - 1) {}
   For p As Integer = 0 To nPlants - 1
      open(p) = model.AddVar(0, 1, FixedCosts(p), GRB.BINARY, "Open" & p)
   Next

   ' Transportation decision variables: how much to transport from a plant
   ' p to a warehouse w
   Dim transport As GRBVar(,) = New GRBVar(nWarehouses - 1, nPlants - 1) {}
   For w As Integer = 0 To nWarehouses - 1
      For p As Integer = 0 To nPlants - 1
         transport(w, p) = model.AddVar(0, GRB.INFINITY, TransCosts(w, p), _
                                        GRB.CONTINUOUS, "Trans" & p & "." & w)
      Next
   Next

   ' The objective is to minimize the total fixed and variable costs
   model.ModelSense = GRB.MINIMIZE

   ' Production constraints
   ' Note that the right-hand limit sets the production to zero if
   ' the plant is closed
   For p As Integer = 0 To nPlants - 1
      Dim ptot As GRBLinExpr = 0.0

      For w As Integer = 0 To nWarehouses - 1
         ptot.AddTerm(1.0, transport(w, p))
      Next

      model.AddConstr(ptot <= Capacity(p) * open(p), "Capacity" & p)
   Next

   ' Demand constraints
   Dim demandConstr As GRBConstr() = New GRBConstr(nWarehouses - 1) {}
   For w As Integer = 0 To nWarehouses - 1
      Dim dtot As GRBLinExpr = 0.0

      For p As Integer = 0 To nPlants - 1
         dtot.AddTerm(1.0, transport(w, p))
      Next

      demandConstr(w) = model.AddConstr(dtot = Demand(w), "Demand" & w)
   Next

   ' We constructed the base model, now we add 7 scenarios
   '
   ' Scenario 0: Represents the base model, hence, no manipulations.
   ' Scenario 1: Manipulate the warehouses demands slightly (constraint right
   '             hand sides).
   ' Scenario 2: Double the warehouses demands (constraint right hand sides).
   ' Scenario 3: Manipulate the plant fixed costs (objective coefficients).
   ' Scenario 4: Manipulate the warehouses demands and fixed costs.
   ' Scenario 5: Force the plant with the largest fixed cost to stay open
   '             (variable bounds).
   ' Scenario 6: Force the plant with the smallest fixed cost to be closed
   '             (variable bounds).

   model.NumScenarios = 7

   ' Scenario 0: Base model, hence, nothing to do except giving the
   '             scenario a name
   model.Parameters.ScenarioNumber = 0
   model.ScenNName = "Base model"

   ' Scenario 1: Increase the warehouse demands by 10%
   model.Parameters.ScenarioNumber = 1
   model.ScenNName = "Increased warehouse demands"

   For w As Integer = 0 To nWarehouses - 1
      demandConstr(w).ScenNRHS = Demand(w) * 1.1
   Next

   ' Scenario 2: Double the warehouse demands
   model.Parameters.ScenarioNumber = 2
   model.ScenNName = "Double the warehouse demands"

   For w As Integer = 0 To nWarehouses - 1
      demandConstr(w).ScenNRHS = Demand(w) * 2.0
   Next

   ' Scenario 3: Decrease the plant fixed costs by 5%
   model.Parameters.ScenarioNumber = 3
   model.ScenNName = "Decreased plant fixed costs"

   For p As Integer = 0 To nPlants - 1
      open(p).ScenNObj = FixedCosts(p) * 0.95
   Next

   ' Scenario 4: Combine scenario 1 and scenario 3 */
   model.Parameters.ScenarioNumber = 4
   model.ScenNName = "Increased warehouse demands and decreased plant fixed costs"

   For w As Integer = 0 To nWarehouses - 1
      demandConstr(w).ScenNRHS = Demand(w) * 1.1
   Next

   For p As Integer = 0 To nPlants - 1
      open(p).ScenNObj = FixedCosts(p) * 0.95
   Next

   ' Scenario 5: Force the plant with the largest fixed cost to stay
   '             open
   model.Parameters.ScenarioNumber = 5
   model.ScenNName = "Force plant with largest fixed cost to stay open"

   For p As Integer = 0 To nPlants - 1

      If FixedCosts(p) = maxFixed Then
         open(p).ScenNLB = 1.0
         Exit For
      End If
   Next

   ' Scenario 6: Force the plant with the smallest fixed cost to be
   '             closed
   model.Parameters.ScenarioNumber = 6
   model.ScenNName = "Force plant with smallest fixed cost to be closed"

   For p As Integer = 0 To nPlants - 1

      If FixedCosts(p) = minFixed Then
         open(p).ScenNUB = 0.0
         Exit For
      End If
   Next

   ' Guess at the starting point: close the plant with the highest fixed
   ' costs; open all others

   ' First, open all plants
   For p As Integer = 0 To nPlants - 1
      open(p).Start = 1.0
   Next

   ' Now close the plant with the highest fixed cost
   Console.WriteLine("Initial guess:")
   For p As Integer = 0 To nPlants - 1

      If FixedCosts(p) = maxFixed Then
         open(p).Start = 0.0
         Console.WriteLine("Closing plant " & p & vbLf)
         Exit For
      End If
   Next

   ' Use barrier to solve root relaxation
   model.Parameters.Method = GRB.METHOD_BARRIER

   ' Solve multi-scenario model
   model.Optimize()
   Dim nScenarios As Integer = model.NumScenarios

   For s As Integer = 0 To nScenarios - 1
      Dim modelSense As Integer = GRB.MINIMIZE

      ' Set the scenario number to query the information for this scenario
      model.Parameters.ScenarioNumber = s

      ' collect result for the scenario
      Dim scenNObjBound As Double = model.ScenNObjBound
      Dim scenNObjVal As Double = model.ScenNObjVal

      Console.WriteLine(vbLf & vbLf & "------ Scenario " & s & " (" & model.ScenNName & ")")

      ' Check if we found a feasible solution for this scenario
      If scenNObjVal >= modelSense * GRB.INFINITY Then
         If scenNObjBound >= modelSense * GRB.INFINITY Then
            ' Scenario was proven to be infeasible
            Console.WriteLine(vbLf & "INFEASIBLE")
         Else
            ' We did not find any feasible solution - should not happen in
            ' this case, because we did not set any limit (like a time
            ' limit) on the optimization process
            Console.WriteLine(vbLf & "NO SOLUTION")
         End If
      Else
         Console.WriteLine(vbLf & "TOTAL COSTS: " & scenNObjVal)
         Console.WriteLine("SOLUTION:")

         For p As Integer = 0 To nPlants - 1
            Dim scenNX As Double = open(p).ScenNX

            If scenNX > 0.5 Then
               Console.WriteLine("Plant " & p & " open")

               For w As Integer = 0 To nWarehouses - 1
                  scenNX = transport(w, p).ScenNX
                  If scenNX > 0.0001 Then Console.WriteLine("  Transport " & scenNX & " units to warehouse " & w)
               Next
            Else
               Console.WriteLine("Plant " & p & " closed!")
            End If
         Next
      End If
   Next

   ' Print a summary table: for each scenario we add a single summary line
   Console.WriteLine(vbLf & vbLf & "Summary: Closed plants depending on scenario" & vbLf)
   Console.WriteLine("{0,8} | {1,17} {2,13}", "", "Plant", "|")

   Console.Write("{0,8} |", "Scenario")
   For p As Integer = 0 To nPlants - 1
      Console.Write("{0,6}", p)
   Next

   Console.WriteLine(" | {0,6}  Name", "Costs")

   For s As Integer = 0 To nScenarios - 1
      Dim modelSense As Integer = GRB.MINIMIZE

      ' Set the scenario number to query the information for this scenario
      model.Parameters.ScenarioNumber = s

      ' Collect result for the scenario
      Dim scenNObjBound As Double = model.ScenNObjBound
      Dim scenNObjVal As Double = model.ScenNObjVal

      Console.Write("{0,-8} |", s)

      ' Check if we found a feasible solution for this scenario
      If scenNObjVal >= modelSense * GRB.INFINITY Then
         If scenNObjBound >= modelSense * GRB.INFINITY Then
            ' Scenario was proven to be infeasible
            Console.WriteLine(" {0,-30}| {1,6}  " & model.ScenNName, "infeasible", "-")
         Else
            ' We did not find any feasible solution - should not happen in
            ' this case, because we did not set any limit (like a Time
            ' limit) on the optimization process
            Console.WriteLine(" {0,-30}| {1,6}  " & model.ScenNName, "no solution found", "-")
         End If
      Else

         For p As Integer = 0 To nPlants - 1
            Dim scenNX As Double = open(p).ScenNX

            If scenNX > 0.5 Then
               Console.Write("{0,6}", " ")
            Else
               Console.Write("{0,6}", "x")
            End If
         Next

         Console.WriteLine(" | {0,6}  " & model.ScenNName, scenNObjVal)
      End If
   Next

   model.Dispose()
   env.Dispose()
   Catch e As GRBException
   Console.WriteLine("Error code: " & e.ErrorCode & ". " + e.Message)
   End Try
End Sub
End Class
