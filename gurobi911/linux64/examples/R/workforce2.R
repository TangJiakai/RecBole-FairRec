# Copyright 2020, Gurobi Optimization, LLC
# 
# Assign workers to shifts; each worker may or may not be available on a
# particular day. If the problem cannot be solved, use IIS iteratively to
# find all conflicting constraints.

library(Matrix)
library(gurobi)

# Function to display results
printsolution <- function(result) {
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
  }
}

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
env$logfile <- 'workforce2.log'

# Build model
model            <- list()
model$modelname  <- 'workforce2'
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
gurobi_write(model,'workforce2.lp', env)

# Optimize
result <- gurobi(model, env = env)

# Display results
if (result$status == 'OPTIMAL') {
# The code may enter here if you change some of the data... otherwise
# this will never be executed.
  printsolution(result);
} else if (result$status == 'INFEASIBLE') {
# We will loop until we reduce a model that can be solved
  numremoved <- 0 
  while(result$status == 'INFEASIBLE') {
    iis               <- gurobi_iis(model, env = env)
    keep              <- (!iis$Arows)
    cat('Removing rows',model$constrnames[iis$Arows],'...\n')
    model$A           <- model$A[keep,,drop = FALSE]
    model$sense       <- model$sense[keep]
    model$rhs         <- model$rhs[keep]
    model$constrnames <- model$constrnames[keep]
    numremoved        <- numremoved + 1
    gurobi_write(model, paste0('workforce2-',numremoved,'.lp'), env)
    result            <- gurobi(model, env = env)
  }
  printsolution(result)
  rm(iis)
} else {
# Just to handle user interruptions or other problems
  cat('Unexpected status',result$status,'\nEnding now\n')
}

#Clear space
rm(model, env, availability, Shifts, Workers, pay, shiftRequirements, result)
