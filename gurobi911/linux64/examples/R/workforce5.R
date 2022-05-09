# Copyright 2020, Gurobi Optimization, LLC
# 
# Assign workers to shifts; each worker may or may not be available on a
# particular day. We use multi-objective optimization to solve the model.
# The highest-priority objective minimizes the sum of the slacks
# (i.e., the total number of uncovered shifts). The secondary objective
# minimizes the difference between the maximum and minimum number of
# shifts worked among all workers.  The second optimization is allowed
# to degrade the first objective by up to the smaller value of 10% and 2

library('Matrix')
library('gurobi')

# define data
nShifts       <- 14
nWorkers      <-  8
nVars         <- (nShifts + 1) * (nWorkers + 1) + 2
varIdx        <- function(w,s) {s+(w-1)*nShifts}
shiftSlackIdx <- function(s) {s+nShifts*nWorkers}
totShiftIdx   <- function(w) {w + nShifts * (nWorkers+1)}
minShiftIdx   <- ((nShifts+1)*(nWorkers+1))
maxShiftIdx   <- (minShiftIdx+1)
totalSlackIdx <- nVars


Shifts  <- c('Mon1', 'Tue2', 'Wed3', 'Thu4', 'Fri5', 'Sat6', 'Sun7',
             'Mon8', 'Tue9', 'Wed10', 'Thu11', 'Fri12', 'Sat13', 'Sun14')
Workers <- c( 'Amy', 'Bob', 'Cathy', 'Dan', 'Ed', 'Fred', 'Gu', 'Tobi' )

shiftRequirements <- c(3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5 )

availability <- list( c( 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1 ),
                      c( 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0 ),
                      c( 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 ),
                      c( 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 ),
                      c( 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1 ),
                      c( 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1 ),
                      c( 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1 ),
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
env$logfile <- 'workforce5.log'

# Build model
model            <- list()
model$modelname  <- 'workforce5'
model$modelsense <- 'min'

# Initialize assignment decision variables:
#    x[w][s] == 1 if worker w is assigned to shift s.
#    This is no longer a pure assignment model, so we must
#    use binary variables.
model$vtype    <- rep('C', nVars)
model$lb       <- rep(0, nVars)
model$ub       <- rep(1, nVars)
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
}

#Initialize min/max shift variables
model$ub[minShiftIdx]       = Inf
model$varnames[minShiftIdx] = 'MinShift'
model$ub[maxShiftIdx]       = Inf
model$varnames[maxShiftIdx] = 'MaxShift'

#Initialize total slack variable
model$ub[totalSlackIdx]      = Inf
model$varnames[totalSlackIdx] = 'TotalSlack'

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

# Set minShift / maxShift general constraints
model$genconmin <- list(list(resvar = minShiftIdx,
                             vars   = c(totShiftIdx(1:nWorkers)),
                             name   = 'MinShift'))
model$genconmax <- list(list(resvar = maxShiftIdx,
                             vars   = c(totShiftIdx(1:nWorkers)),
                             name   = 'MaxShift'))

# Set multiobjective
model$multiobj <- list(1:2)
model$multiobj[[1]]          <- list()
model$multiobj[[1]]$objn     <- c(rep(0,nVars))
model$multiobj[[1]]$objn[totalSlackIdx] = 1
model$multiobj[[1]]$priority <- 2
model$multiobj[[1]]$weight   <- 1
model$multiobj[[1]]$abstol   <- 2
model$multiobj[[1]]$reltol   <- 0.1
model$multiobj[[1]]$name     <- 'TotalSlack'
model$multiobj[[1]]$con      <- 0.0
model$multiobj[[2]]          <- list()
model$multiobj[[2]]$objn     <- c(rep(0,nVars))
model$multiobj[[2]]$objn[minShiftIdx] = -1
model$multiobj[[2]]$objn[maxShiftIdx] =  1
model$multiobj[[2]]$priority <- 1
model$multiobj[[2]]$weight   <- 1
model$multiobj[[2]]$abstol   <- 0
model$multiobj[[2]]$reltol   <- 0
model$multiobj[[2]]$name     <- 'Fairness'
model$multiobj[[2]]$con      <- 0.0


# Save initial model
gurobi_write(model,'workforce5.lp', env)

# Optimize
result <- solveandprint(model, env)
if (result$status != 'OPTIMAL') stop('Stop now\n')

#Clear space
rm(model, env, availability, Shifts, Workers, shiftRequirements, result)
