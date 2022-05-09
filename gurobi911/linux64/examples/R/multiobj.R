# Copyright 2020, Gurobi Optimization, LLC
#
# Want to cover three different sets but subject to a common budget of
# elements allowed to be used. However, the sets have different priorities to
# be covered; and we tackle this by using multi-objective optimization.

library(Matrix)
library(gurobi)

# define primitive data
groundSetSize     <- 20
nSubSets          <- 4
Budget            <- 12
Set               <- list(
    c( 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ),
    c( 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 ),
    c( 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0 ),
    c( 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0 ) )
SetObjPriority    <- c(3, 2, 2, 1)
SetObjWeight      <- c(1.0, 0.25, 1.25, 1.0)

# Initialize model
model             <- list()
model$modelsense  <- 'max'
model$modelname   <- 'multiobj'

# Set variables, all of them are binary, with 0,1 bounds.
model$vtype       <- 'B'
model$lb          <- 0
model$ub          <- 1
model$varnames    <- paste(rep('El', groundSetSize), 1:groundSetSize, sep='')

# Build constraint matrix
model$A           <- spMatrix(1, groundSetSize,
                              i = rep(1,groundSetSize),
                              j = 1:groundSetSize,
                              x = rep(1,groundSetSize))
model$rhs         <- c(Budget)
model$sense       <- c('<')
model$constrnames <- c('Budget')

# Set multi-objectives
model$multiobj          <- list()
for (m in 1:nSubSets) {
  model$multiobj[[m]]          <- list()
  model$multiobj[[m]]$objn     <- Set[[m]]
  model$multiobj[[m]]$priority <- SetObjPriority[m]
  model$multiobj[[m]]$weight   <- SetObjWeight[m]
  model$multiobj[[m]]$abstol   <- m
  model$multiobj[[m]]$reltol   <- 0.01
  model$multiobj[[m]]$name     <- sprintf('Set%d', m)
  model$multiobj[[m]]$con      <- 0.0
}

# Save model
gurobi_write(model,'multiobj_R.lp')

# Set parameters
params               <- list()
params$PoolSolutions <- 100

# Optimize
result <- gurobi(model, params)

# Capture solution information
if (result$status != 'OPTIMAL') {
  cat('Optimization finished with status', result$status, '\n')
  stop('Stop now\n')
}

# Print best solution
cat('Selected elements in best solution:\n')
for (e in 1:groundSetSize) {
  if(result$x[e] < 0.9) next
  cat(' El',e,sep='')
}
cat('\n')

# Iterate over the best 10 solutions
if ('pool' %in% names(result)) {
  solcount <- length(result$pool)
  cat('Number of solutions found:', solcount, '\n')
  if (solcount > 10) {
    solcount <- 10
  }
  cat('Objective values for first', solcount, 'solutions:\n')
  for (k in 1:solcount) {
    cat('Solution', k, 'has objective:', result$pool[[k]]$objval[1], '\n')
  }
} else {
  solcount <- 1
  cat('Number of solutions found:', solcount, '\n')
  cat('Solution 1 has objective:', result$objval, '\n')
}

# Clean up
rm(model, params, result)
