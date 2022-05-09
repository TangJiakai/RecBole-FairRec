' Copyright 2020, Gurobi Optimization, LLC
'
' This example reads a MIP model from a file, solves it in batch mode,
' and prints the JSON solution string.
'
'   You will need a Cluster Manager license for this example to work. */

Imports System
Imports Gurobi

Class batchmode_vb

    ' Set-up the environment for batch mode optimization.
    '
    ' The function creates an empty environment, sets all neccessary
    ' parameters, and returns the ready-to-be-started Env object to caller.
    ' It is the caller's responsibility to dispose of this environment when
    ' it's no longer needed.
    Private Shared Function setupbatchenv() As GRBEnv
        Dim env As GRBEnv = New GRBEnv(True)
        env.CSBatchMode = 1
        env.CSManager = "http://localhost:61080"
        env.LogFile = "batchmode.log"
        env.ServerPassword = "pass"
        env.UserName = "gurobi"

        ' No network communication happened up to this point. This will happen
        ' once the caller invokes the start() method of the returned Env
        ' Object.

        Return env
    End Function

    ' Print batch job error information, if any
    Private Shared Sub printbatcherrorinfo(ByRef batch As GRBBatch)
        If batch.BatchErrorCode = 0 Then Return
        Console.WriteLine("Batch ID: " & batch.BatchID & ", Error code: " + batch.BatchErrorCode & "(" + batch.BatchErrorMessage & ")")
    End Sub

    ' Create a batch request for given problem file
    Private Shared Function newbatchrequest(ByVal filename As String) As String
        Dim batchID As String = ""

        ' Start environment, create Model object from file
        Dim env As GRBEnv = setupbatchenv()
        env.Start()
        Dim model As GRBModel = New GRBModel(env, filename)

        Try
            ' Set some parameters
            model.[Set](GRB.DoubleParam.MIPGap, 0.01)
            model.[Set](GRB.IntParam.JSONSolDetail, 1)

            ' Define tags for some variables in order to access their values later
            Dim count As Integer = 0
            For Each v As GRBVar In model.GetVars()
                v.VTag = "Variable" & count
                count += 1
                If count >= 10 Then Exit For
            Next

            ' submit batch request
            batchID = model.OptimizeBatch()
        Finally
            model.Dispose()
            env.Dispose()
        End Try

        Return batchID
    End Function

    ' Wait for the final status of the batch.
    ' Initially the status of a batch is "submitted"; the status will change
    ' once the batch has been processed (by a compute server).
    Private Shared Sub waitforfinalstatus(ByVal batchID As String)
        ' Wait no longer than one hour
        Dim maxwaittime As Double = 3600
        Dim start As DateTime = DateTime.Now

        ' Setup and start environment, create local Batch handle object
        Dim env As GRBEnv = setupbatchenv()
        env.Start()
        Dim batch As GRBBatch = New GRBBatch(env, batchID)

        Try

            While batch.BatchStatus = GRB.BatchStatus.SUBMITTED
                ' Abort this batch if it is taking too long
                Dim interval As TimeSpan = DateTime.Now - start
                If interval.TotalSeconds > maxwaittime Then
                    batch.Abort()
                    Exit While
                End If

                ' Wait for two seconds
                System.Threading.Thread.Sleep(2000)

                ' Update the resident attribute cache of the Batch object with the
                ' latest values from the cluster manager.
                batch.Update()

                ' If the batch failed, we retry it
                If batch.BatchStatus = GRB.BatchStatus.FAILED Then
                    batch.Retry()
                    System.Threading.Thread.Sleep(2000)
                    batch.Update()
                End If
            End While

        Finally
           ' Print information about error status of the job that
           ' processed the batch
            printbatcherrorinfo(batch)
            batch.Dispose()
            env.Dispose()
        End Try
    End Sub

    Private Shared Sub printfinalreport(ByVal batchID As String)
        ' Setup and start environment, create local Batch handle object
        Dim env As GRBEnv = setupbatchenv()
        env.Start()
        Dim batch As GRBBatch = New GRBBatch(env, batchID)

        Select Case batch.BatchStatus
            Case GRB.BatchStatus.CREATED
                Console.WriteLine("Batch status is 'CREATED'" & vbLf)
            Case GRB.BatchStatus.SUBMITTED
                Console.WriteLine("Batch is 'SUBMITTED" & vbLf)
            Case GRB.BatchStatus.ABORTED
                Console.WriteLine("Batch is 'ABORTED'" & vbLf)
            Case GRB.BatchStatus.FAILED
                Console.WriteLine("Batch is 'FAILED'" & vbLf)
            Case GRB.BatchStatus.COMPLETED
                Console.WriteLine("Batch is 'COMPLETED'" & vbLf)
                ' Pretty printing the general solution information
                Console.WriteLine("JSON solution:" & batch.GetJSONSolution())

                ' Write the full JSON solution string to a file
                batch.WriteJSONSolution("batch-sol.json.gz")
            Case Else
                ' Should not happen
                Console.WriteLine("Unknown BatchStatus" & batch.BatchStatus)
                Environment.[Exit](1)
        End Select

        batch.Dispose()
        env.Dispose()
    End Sub

    ' Instruct cluster manager to discard all data relating to this BatchID
    Private Shared Sub batchdiscard(ByVal batchID As String)
        ' Setup and start environment, create local Batch handle object
        Dim env As GRBEnv = setupbatchenv()
        env.Start()
        Dim batch As GRBBatch = New GRBBatch(env, batchID)

        ' Remove batch request from manager
        batch.Discard()
        batch.Dispose()
        env.Dispose()
    End Sub

    ' Solve a given model using batch optimization
    Shared Sub Main(ByVal args As String())
        ' Ensure we have an input file
        If args.Length < 1 Then
            Console.Out.WriteLine("Usage: batchmode_vb filename")
            Return
        End If

        Try
            ' Submit new batch request
            Dim batchID As String = newbatchrequest(args(0))

            ' Wait for final status
            waitforfinalstatus(batchID)

            ' Report final status info
            printfinalreport(batchID)

            ' Remove batch request from manager
            batchdiscard(batchID)

            Console.WriteLine("Batch optimization OK")
        Catch e As GRBException
            Console.WriteLine("Error code: " & e.ErrorCode & ". " + e.Message)
        End Try
    End Sub
End Class
