# Copyright 2020, Gurobi Optimization, LLC
# 
# Assign workers to shifts; each worker may or may not be available on a
# particular day. If the problem cannot be solved, use IIS to find a set of
# conflicting constraints. Note that there may be additional conflicts
# besides what is reported via IIS.

library(Matrix)
library(gurobi)

# define data
nShifts  <- 14
nWorkers <-  7
nVars    <- nShifts * nWorkers
varIdx   <- function(w,s) {s+(w-1)*nShifts}

Shifts  <- c('Mon1', 'Tue2', 'Wed3', 'Thu4', 'Fri5', 'Sat6', 'Sun7',
             'Mon8', 'Tue9', 'Wed10', 'Thu11', 'Fri12', 'Sat13', 'Sun14')
Workers <- c( 'Amy', 'Bob', 'Cathy', 'Dan', 'Ed', 'Fred', 'Gu' )

pay     <- c(10, 12, 10, 8, 8, 9, 11 )

shiftRequirements <- c(3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5 )

availability <- list( c( 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1 ),
                      c( 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0 ),
                      c( 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 ),
                      c( 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 ),
                      c( 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1 ),
                      c( 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1 ),
                      c( 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ) )

# Set-up environment
env <- list()
env$logfile <- 'workforce1.log'

# Build model
model            <- list()
model$modelname  <- 'workforce1'
model$modelsense <- 'min'

# Initialize assignment decision variables:
#    x[w][s] == 1 if worker w is assigned
#    to shift s. Since an assignment model always produces integer
#    solutions, we use continuous variables and solve as an LP.
model$lb       <- 0
model$ub       <- rep(1, nVars)
model$obj      <- rep(0, nVars)
model$varnames <- rep('',nVars)
for (w in 1:nWorkers) {
  for (s in 1:nShifts) {
    model$varnames[varIdx(w,s)] = paste0(Workers[w],'.',Shifts[s])
    model$obj[varIdx(w,s)]      = pay[w]
    if (availability[[w]][s] == 0) model$ub[varIdx(w,s)] = 0
  }
}

# Set-up shift-requirements constraints
model$A           <- spMatrix(nShifts,nVars,
                      i = c(mapply(rep,1:nShifts,nWorkers)),
                      j = mapply(varIdx,1:nWorkers,
                                 mapply(rep,1:nShifts,nWorkers)),
                      x = rep(1,nShifts * nWorkers))
model$sense       <- rep('=',nShifts)
model$rhs         <- shiftRequirements
model$constrnames <- Shifts

# Save model
gurobi_write(model,'workforce1.lp', env)

# Optimize
result <- gurobi(model, env = env)

# Display results
if (result$status == 'OPTIMAL') {
# The code may enter here if you change some of the data... otherwise
# this will never be executed.
  cat('The optimal objective is',result$objval,'\n')
  cat('Schedule:\n')
  for (s in 1:nShifts) {
    cat('\t',Shifts[s],':')
    for (w in 1:nWorkers) {
      if (result$x[varIdx(w,s)] > 0.9) cat(Workers[w],' ')
    }
    cat('\n')
  }
} else if (result$status == 'INFEASIBLE') {
# Find ONE IIS
  cat('Problem is infeasible.... computing IIS\n')
  iis <- gurobi_iis(model, env = env)
  if (iis$minimal) cat('IIS is minimal\n')
  else             cat('IIS is not minimal\n')
  cat('Rows in IIS: ', model$constrnames[iis$Arows])
  cat('\nLB in IIS: ', model$varnames[iis$lb])
  cat('\nUB in IIS: ', model$varnames[iis$ub])
  cat('\n')
  rm(iis)
} else {
# Just to handle user interruptions or other problems
  cat('Unexpected status',result$status,'\nEnding now\n')
}

#Clear space
rm(model, env, availability, Shifts, Workers, pay, shiftRequirements, result)
