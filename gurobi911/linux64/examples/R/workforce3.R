# Copyright 2020, Gurobi Optimization, LLC
# 
# Assign workers to shifts; each worker may or may not be available on a
# particular day. If the problem cannot be solved, relax the model
# to determine which constraints cannot be satisfied, and how much
# they need to be relaxed.

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
env$logfile <- 'workforce3.log'

# Build model
model            <- list()
model$modelname  <- 'workforce3'
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
gurobi_write(model,'workforce3.lp', env)

# Optimize
result <- gurobi(model, env = env)

# Display results
if (result$status == 'OPTIMAL') {
# The code may enter here if you change some of the data... otherwise
# this will never be executed.
  printsolution(result);
} else if (result$status == 'INFEASIBLE') {
# Use gurobi_feasrelax to find out which copnstraints should be relaxed
# and by how much to make the problem feasible.
  penalties     <- list()
  penalties$lb  <- Inf
  penalties$ub  <- Inf
  penalties$rhs <- rep(1,length(model$rhs))
  feasrelax     <- gurobi_feasrelax(model, 0, FALSE, penalties, env = env)
  result        <- gurobi(feasrelax$model, env = env)
  if (result$status == 'OPTIMAL') {
    printsolution(result)
    cat('Slack values:\n')
    for (j in (nVars+1):length(result$x)) {
      if(result$x[j] > 0.1)
        cat('\t',feasrelax$model$varnames[j],result$x[j],'\n')
    }
  } else {
    cat('Unexpected status',result$status,'\nEnding now\n')
  }
  rm(penalties, feasrelax)
} else {
# Just to handle user interruptions or other problems
  cat('Unexpected status',result$status,'\nEnding now\n')
}

#Clear space
rm(model, env, availability, Shifts, Workers, pay, shiftRequirements, result)
