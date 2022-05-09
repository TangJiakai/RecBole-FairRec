
' Copyright 2020, Gurobi Optimization, LLC
'
' Assign workers to shifts; each worker may or may not be available on a
' particular day. We use multi-objective optimization to solve the model.
' The highest-priority objective minimizes the sum of the slacks
' (i.e., the total number of uncovered shifts). The secondary objective
' minimizes the difference between the maximum and minimum number of
' shifts worked among all workers.  The second optimization is allowed
' to degrade the first objective by up to the smaller value of 10% and 2 */

Imports System
Imports Gurobi

Class workforce5_vb
    Shared Sub Main()

        Try

            ' Sample data
            ' Sets of days and workers
            Dim Shifts As String() = New String() { _
                "Mon1", "Tue2", "Wed3", "Thu4", "Fri5", "Sat6", "Sun7", _
                "Mon8", "Tue9", "Wed10", "Thu11", "Fri12", "Sat13", "Sun14"}

            Dim Workers As String() = New String() { _
                "Amy", "Bob", "Cathy", "Dan", "Ed", "Fred", "Gu", "Tobi"}

            Dim nShifts As Integer = Shifts.Length
            Dim nWorkers As Integer = Workers.Length

            ' Number of workers required for each shift
            Dim shiftRequirements As Double() = New Double() { _
                3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5}

            ' Worker availability: 0 if the worker is unavailable for a shift
            Dim availability As Double(,) = New Double(,) { _
                {0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1}, _
                {1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0}, _
                {0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, _
                {0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1}, _
                {1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1}, _
                {1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1}, _
                {0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1}, _
                {1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}

            ' Create environment
            Dim env As New GRBEnv()

            ' Create initial model
            Dim model As New GRBModel(env)
            model.ModelName = "workforce5_vb"

            ' Initialize assignment decision variables:
            ' x[w][s] == 1 if worker w is assigned to shift s.
            ' This is no longer a pure assignment model, so we must
            ' use binary variables.
            Dim x As GRBVar(,) = New GRBVar(nWorkers - 1, nShifts - 1) {}
            For w As Integer = 0 To nWorkers - 1
                For s As Integer = 0 To nShifts - 1
                    x(w, s) = model.AddVar(0, availability(w, s), 0, GRB.BINARY, _
                                           String.Format("{0}.{1}", Workers(w), Shifts(s)))
                Next
            Next

            ' Slack variables for each shift constraint so that the shifts can
            ' be satisfied
            Dim slacks As GRBVar() = New GRBVar(nShifts - 1) {}
            For s As Integer = 0 To nShifts - 1
                slacks(s) = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, _
                                         String.Format("{0}Slack", Shifts(s)))
            Next

            ' Variable to represent the total slack
            Dim totSlack As GRBVar = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, "totSlack")

            ' Variables to count the total shifts worked by each worker
            Dim totShifts As GRBVar() = New GRBVar(nWorkers - 1) {}
            For w As Integer = 0 To nWorkers - 1
                totShifts(w) = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, _
                                            String.Format("{0}TotShifts", Workers(w)))
            Next

            Dim lhs As GRBLinExpr

            ' Constraint: assign exactly shiftRequirements[s] workers
            ' to each shift s, plus the slack
            For s As Integer = 0 To nShifts - 1
                lhs = New GRBLinExpr()
                lhs.AddTerm(1.0, slacks(s))
                For w As Integer = 0 To nWorkers - 1
                    lhs.AddTerm(1.0, x(w, s))
                Next
                model.AddConstr(lhs, GRB.EQUAL, shiftRequirements(s), Shifts(s))
            Next

            ' Constraint: set totSlack equal to the total slack
            lhs = New GRBLinExpr()
            lhs.AddTerm(-1.0, totSlack)
            For s As Integer = 0 To nShifts - 1
                lhs.AddTerm(1.0, slacks(s))
            Next
            model.AddConstr(lhs, GRB.EQUAL, 0, "totSlack")

            ' Constraint: compute the total number of shifts for each worker
            For w As Integer = 0 To nWorkers - 1
                lhs = New GRBLinExpr()
                lhs.AddTerm(-1.0, totShifts(w))
                For s As Integer = 0 To nShifts - 1
                    lhs.AddTerm(1.0, x(w, s))
                Next
                model.AddConstr(lhs, GRB.EQUAL, 0, String.Format("totShifts{0}", Workers(w)))
            Next

            ' Constraint: set minShift/maxShift variable to less <=/>= to the
            ' number of shifts among all workers
            Dim minShift As GRBVar = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, "minShift")
            Dim maxShift As GRBVar = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, "maxShift")
            model.AddGenConstrMin(minShift, totShifts, GRB.INFINITY, "minShift")
            model.AddGenConstrMax(maxShift, totShifts, -GRB.INFINITY, "maxShift")

            ' Set global sense for ALL objectives
            model.ModelSense = GRB.MINIMIZE

            ' Set primary objective
            model.SetObjectiveN(totSlack, 0, 2, 1.0, 2.0, 0.1, "TotalSlack")

            ' Set secondary objective
            model.SetObjectiveN(maxShift - minShift, 1, 1, 1.0, 0, 0, "Fairness")

            ' Save problem
            model.Write("workforce5_vb.lp")

            ' Optimize
            Dim status As Integer = _
                solveAndPrint(model, totSlack, nWorkers, Workers, totShifts)

            If status <> GRB.Status.OPTIMAL Then
                Return
            End If

            ' Dispose of model and environment
            model.Dispose()

            env.Dispose()
        Catch e As GRBException
            Console.WriteLine("Error code: {0}. {1}", e.ErrorCode, e.Message)
        End Try
    End Sub

    Private Shared Function solveAndPrint(ByVal model As GRBModel, _
                                          ByVal totSlack As GRBVar, _
                                          ByVal nWorkers As Integer, _
                                          ByVal Workers As String(), _
                                          ByVal totShifts As GRBVar()) As Integer

        model.Optimize()
        Dim status As Integer = model.Status
        If status = GRB.Status.INF_OR_UNBD OrElse _
           status = GRB.Status.INFEASIBLE OrElse _
           status = GRB.Status.UNBOUNDED Then
            Console.WriteLine("The model cannot be solved " & _
                     "because it is infeasible or unbounded")
            Return status
        End If
        If status <> GRB.Status.OPTIMAL Then
            Console.WriteLine("Optimization was stopped with status {0}", status)
            Return status
        End If

        ' Print total slack and the number of shifts worked for each worker
        Console.WriteLine(vbLf & "Total slack required: {0}", totSlack.X)
        For w As Integer = 0 To nWorkers - 1
            Console.WriteLine("{0} worked {1} shifts", Workers(w), totShifts(w).X)
        Next
        Console.WriteLine(vbLf)
        Return status
    End Function

End Class
