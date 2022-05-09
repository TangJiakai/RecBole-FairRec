
' Copyright 2020, Gurobi Optimization, LLC

' Want to cover three different sets but subject to a common budget of
' elements allowed to be used. However, the sets have different priorities to
' be covered; and we tackle this by using multi-objective optimization.

Imports Gurobi

Class multiobj_vb

    Shared Sub Main()

        Try
            ' Sample data
            Dim groundSetSize As Integer = 20
            Dim nSubsets As Integer = 4
            Dim Budget As Integer = 12

            Dim [Set] As Double(,) = New Double(,) { _
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, _
                {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}, _
                {0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0}, _
                {0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0}}

            Dim SetObjPriority As Integer() = New Integer() {3, 2, 2, 1}
            Dim SetObjWeight As Double() = New Double() {1.0, 0.25, 1.25, 1.0}
            Dim e As Integer, i As Integer, status As Integer, nSolutions As Integer

            ' Create environment
            Dim env As New GRBEnv("multiobj_vb.log")

            ' Create initial model
            Dim model As New GRBModel(env)
            model.ModelName = "multiobj_vb"

            ' Initialize decision variables for ground set:
            ' x[e] == 1 if element e is chosen for the covering.
            Dim Elem As GRBVar() = model.AddVars(groundSetSize, GRB.BINARY)
            For e = 0 To groundSetSize - 1
                Dim vname As String = "El" & e.ToString()
                Elem(e).VarName = vname
            Next

            ' Constraint: limit total number of elements to be picked to be at most
            ' Budget
            Dim lhs As New GRBLinExpr()
            For e = 0 To groundSetSize - 1
                lhs.AddTerm(1.0, Elem(e))
            Next
            model.AddConstr(lhs, GRB.LESS_EQUAL, Budget, "Budget")

            ' Set global sense for ALL objectives
            model.ModelSense = GRB.MAXIMIZE

            ' Limit how many solutions to collect
            model.Parameters.PoolSolutions = 100

            ' Set and configure i-th objective
            For i = 0 To nSubsets - 1
                Dim vname As String = String.Format("Set{0}", i)
                Dim objn As New GRBLinExpr()
                For e = 0 To groundSetSize - 1
                    objn.AddTerm([Set](i, e), Elem(e))
                Next

                model.SetObjectiveN(objn, i, SetObjPriority(i), SetObjWeight(i), _
                                    1.0 + i, 0.01, vname)
            Next

            ' Save problem
            model.Write("multiobj_vb.lp")

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

            ' Print best selected set
            Console.WriteLine("Selected elements in best solution:")
            Console.Write(vbTab)
            For e = 0 To groundSetSize - 1
                If Elem(e).X < 0.9 Then
                    Continue For
                End If
                Console.Write("El{0} ", e)
            Next
            Console.WriteLine()

            ' Print number of solutions stored
            nSolutions = model.SolCount
            Console.WriteLine("Number of solutions found: {0}", nSolutions)

            ' Print objective values of solutions
            If nSolutions > 10 Then
                nSolutions = 10
            End If
            Console.WriteLine("Objective values for first {0} solutions:", nSolutions)
            For i = 0 To nSubsets - 1
                model.Parameters.ObjNumber = i

                Console.Write(vbTab & "Set" & i)
                For e = 0 To nSolutions - 1
                    model.Parameters.SolutionNumber = e
                    Console.Write("{0,8}", model.ObjNVal)
                Next
                Console.WriteLine()
            Next

            model.Dispose()
            env.Dispose()

        Catch e As GRBException
            Console.WriteLine("Error code = {0}", e)
            Console.WriteLine(e.Message)

        End Try
    End Sub
End Class
