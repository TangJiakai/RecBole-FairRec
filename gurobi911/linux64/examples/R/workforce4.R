# Copyright 2020, Gurobi Optimization, LLC
# 
# Assign workers to shifts; each worker may or may not be available on a
# particular day.  We use Pareto optimization to solve the model:
# first, we minimize the linear sum of the slacks. Then, we constrain
# the sum of the slacks, and we minimize a quadratic objective that
# tries to balance the workload among the workers.

library(Matrix)
library(gurobi)

# define data
nShifts       <- 14
nWorkers      <-  7
nVars         <- (nShifts + 1) * (nWorkers + 1) + nWorkers + 1
varIdx        <- function(w,s) {s+(w-1)*nShifts}
shiftSlackIdx <- function(s) {s+nShifts*nWorkers}
totShiftIdx   <- function(w) {w + nShifts * (nWorkers+1)}
avgShiftIdx   <- ((nShifts+1)*(nWorkers+1))
diffShiftIdx  <- function(w) {w + avgShiftIdx}
totalSlackIdx <- nVars


Shifts  <- c('Mon1', 'Tue2', 'Wed3', 'Thu4', 'Fri5', 'Sat6', 'Sun7',
             'Mon8', 'Tue9', 'Wed10', 'Thu11', 'Fri12', 'Sat13', 'Sun14')
Workers <- c( 'Amy', 'Bob', 'Cathy', 'Dan', 'Ed', 'Fred', 'Gu' )

shiftRequirements <- c(3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5 )

availability <- list( c( 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1 ),
                      c( 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0 ),
                      c( 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 ),
                      c( 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 ),
                      c( 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1 ),
                      c( 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1 ),
                      c( 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ) )

# Function to display results
solveandprint <- function(model, env) {
  result <- gurobi(model, env = env)
  if(result$status == 'OPTIMAL') {
    cat('The optimal objective is',result$objval,'\n')
    cat('Schedule:\n')
    for (s in 1:nShifts) {
      cat('\t',Shifts[s],':')
      for (w in 1:nWorkers) {
        if (result$x[varIdx(w,s)] > 0.9) cat(Workers[w],' ')
      }
      cat('\n')
    }
    cat('Workload:\n')
    for (w in 1:nWorkers) {
      cat('\t',Workers[w],':',result$x[totShiftIdx(w)],'\n')
    }
  } else {
    cat('Optimization finished with status',result$status)
  }
  result
}

# Set-up environment
env <- list()
env$logfile <- 'workforce4.log'

# Build model
model            <- list()
model$modelname  <- 'workforce4'
model$modelsense <- 'min'

# Initialize assignment decision variables:
#    x[w][s] == 1 if worker w is assigned to shift s.
#    This is no longer a pure assignment model, so we must
#    use binary variables.
model$vtype    <- rep('C', nVars)
model$lb       <- rep(0, nVars)
model$ub       <- rep(1, nVars)
model$obj      <- rep(0, nVars)
model$varnames <- rep('',nVars)
for (w in 1:nWorkers) {
  for (s in 1:nShifts) {
    model$vtype[varIdx(w,s)]    = 'B'
    model$varnames[varIdx(w,s)] = paste0(Workers[w],'.',Shifts[s])
    if (availability[[w]][s] == 0) model$ub[varIdx(w,s)] = 0
  }
}

# Initialize shift slack variables
for (s in 1:nShifts) {
  model$varnames[shiftSlackIdx(s)] = paste0('ShiftSlack',Shifts[s])
  model$ub[shiftSlackIdx(s)] = Inf
}

# Initialize worker slack and diff variables
for (w in 1:nWorkers) {
  model$varnames[totShiftIdx(w)] = paste0('TotalShifts',Workers[w])
  model$ub[totShiftIdx(w)]       = Inf
  model$varnames[diffShiftIdx(w)]  = paste0('DiffShifts',Workers[w])
  model$ub[diffShiftIdx(w)]        = Inf
  model$lb[diffShiftIdx(w)]        = -Inf
}

#Initialize average shift variable
model$ub[avgShiftIdx]      = Inf
model$varnames[avgShiftIdx] = 'AvgShift'

#Initialize total slack variable
model$ub[totalSlackIdx]      = Inf
model$varnames[totalSlackIdx] = 'TotalSlack'
model$obj[totalSlackIdx]     = 1

# Set-up shift-requirements constraints
model$A           <- spMatrix(nShifts,nVars,
                      i = c(c(mapply(rep,1:nShifts,nWorkers)),
                            c(1:nShifts)),
                      j = c(mapply(varIdx,1:nWorkers,
                                 mapply(rep,1:nShifts,nWorkers)),
                            shiftSlackIdx(1:nShifts)),
                      x = rep(1,nShifts * (nWorkers+1)))
model$sense       <- rep('=',nShifts)
model$rhs         <- shiftRequirements
model$constrnames <- Shifts

# Set TotalSlack equal to the sum of each shift slack
B <- spMatrix(1, nVars,
        i = rep(1,nShifts+1),
        j = c(shiftSlackIdx(1:nShifts),totalSlackIdx),
        x = c(rep(1,nShifts),-1))
model$A           <- rbind(model$A, B)
model$rhs         <- c(model$rhs,0)
model$sense       <- c(model$sense,'=')
model$constrnames <- c(model$constrnames, 'TotalSlack')

# Set total number of shifts for each worker
B <- spMatrix(nWorkers, nVars,
          i = c(mapply(rep,1:nWorkers,nShifts),
                1:nWorkers),
          j = c(mapply(varIdx,c(mapply(rep,1:nWorkers,nShifts)),1:nShifts),
                totShiftIdx(1:nWorkers)),
          x = c(rep(1,nShifts*nWorkers),rep(-1,nWorkers)))
model$A           <- rbind(model$A, B)
model$rhs         <- c(model$rhs,rep(0,nWorkers))
model$sense       <- c(model$sense,rep('=',nWorkers))
model$constrnames <- c(model$constrnames, sprintf('TotalShifts%s',Workers[1:nWorkers]))

# Save initial model
gurobi_write(model,'workforce4.lp', env)

# Optimize
result <- solveandprint(model, env)
if (result$status != 'OPTIMAL') stop('Stop now\n')

# Constraint the slack by setting its upper and lower bounds
totalSlack <- result$x[totalSlackIdx]
model$lb[totalSlackIdx] = totalSlack
model$ub[totalSlackIdx] = totalSlack

# Link average number of shifts worked and difference with average
B <- spMatrix(nWorkers+1, nVars,
        i = c(1:nWorkers,
              1:nWorkers,
              1:nWorkers,
              rep(nWorkers+1,nWorkers+1)),
        j = c(totShiftIdx(1:nWorkers),
              diffShiftIdx(1:nWorkers),
              rep(avgShiftIdx,nWorkers),
              totShiftIdx(1:nWorkers),avgShiftIdx),
        x = c(rep(1, nWorkers),
              rep(-1,nWorkers),
              rep(-1,nWorkers),
              rep(1,nWorkers),-nWorkers))
model$A           <- rbind(model$A, B)
model$rhs         <- c(model$rhs,rep(0,nWorkers+1))
model$sense       <- c(model$sense,rep('=',nWorkers+1))
model$constrnames <- c(model$constrnames,
                       sprintf('DiffShifts%s',Workers[1:nWorkers]),
                       'AvgShift')

# Objective: minimize the sum of the square of the difference from the
# average number of shifts worked
model$obj <- 0
model$Q   <- spMatrix(nVars,nVars,
                i = c(diffShiftIdx(1:nWorkers)),
                j = c(diffShiftIdx(1:nWorkers)),
                x = rep(1,nWorkers))

# Save modified model
gurobi_write(model,'workforce4b.lp', env)

# Optimize
result <- solveandprint(model, env)
if (result$status != 'OPTIMAL') stop('Stop now\n')

#Clear space
rm(model, env, availability, Shifts, Workers, shiftRequirements, result)
