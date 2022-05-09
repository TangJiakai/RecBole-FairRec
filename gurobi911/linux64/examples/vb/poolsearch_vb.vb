
' Copyright 2020, Gurobi Optimization, LLC

' We find alternative epsilon-optimal solutions to a given knapsack
' problem by using PoolSearchMode

Imports Gurobi

Class poolsearch_vb

    Shared Sub Main()

        Try

            'Sample data
            Dim groundSetSize As Integer = 10

            Dim objCoef As Double() = New Double() { _
                32, 32, 15, 15, 6, 6, 1, 1, 1, 1}

            Dim knapsackCoef As Double() = New Double() { _
                16, 16, 8, 8, 4, 4, 2, 2, 1, 1}

            Dim Budget As Double = 33
            Dim e As Integer, status As Integer, nSolutions As Integer

            ' Create environment
            Dim env As New GRBEnv("poolsearch_vb.log")

            ' Create initial model
            Dim model As New GRBModel(env)
            model.ModelName = "poolsearch_vb"

            ' Initialize decision variables for ground set:
            ' x[e] == k if element e is chosen k-times.
            Dim Elem As GRBVar() = model.AddVars(groundSetSize, GRB.BINARY)
            model.[Set](GRB.DoubleAttr.Obj, Elem, objCoef, 0, groundSetSize)

            For e = 0 To groundSetSize - 1
                Elem(e).VarName = String.Format("El{0}", e)
            Next

            ' Constraint: limit total number of elements to be picked to be at most Budget
            Dim lhs As New GRBLinExpr()
            For e = 0 To groundSetSize - 1
                lhs.AddTerm(knapsackCoef(e), Elem(e))
            Next
            model.AddConstr(lhs, GRB.LESS_EQUAL, Budget, "Budget")

            ' set global sense for ALL objectives
            model.ModelSense = GRB.MAXIMIZE

            ' Limit how many solutions to collect
            model.Parameters.PoolSolutions = 1024

            ' Limit how many solutions to collect
            model.Parameters.PoolGap = 0.1

            ' Limit how many solutions to collect
            model.Parameters.PoolSearchMode = 2

            ' save problem
            model.Write("poolsearch_vb.lp")

            ' Optimize
            model.Optimize()

            ' Status checking
            status = model.Status

            If status = GRB.Status.INF_OR_UNBD OrElse _
               status = GRB.Status.INFEASIBLE OrElse _
               status = GRB.Status.UNBOUNDED Then
                Console.WriteLine("The model cannot be solved because it is infeasible or unbounded")
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
            Console.WriteLine("Number of solutions found: ", nSolutions)

            ' Print objective values of solutions
            For e = 0 To nSolutions - 1
                model.Parameters.SolutionNumber = e
                Console.Write("{0} ", model.PoolObjVal)
                If e Mod 15 = 14 Then
                    Console.WriteLine()
                End If
            Next
            Console.WriteLine()

            model.Dispose()
            env.Dispose()

        Catch e As GRBException
            Console.WriteLine("Error code: {0}. {1}", e.ErrorCode, e.Message)

        End Try
    End Sub

End Class
