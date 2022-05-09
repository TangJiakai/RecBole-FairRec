# Copyright 2020, Gurobi Optimization, LLC
#
# We find alternative epsilon-optimal solutions to a given knapsack
# problem by using PoolSearchMode

library(Matrix)
library(gurobi)

# define primitive data
groundSetSize <- 10
objCoef       <- c(32, 32, 15, 15, 6, 6, 1, 1, 1, 1)
knapsackCoef  <- c(16, 16,  8,  8, 4, 4, 2, 2, 1, 1)
Budget        <- 33

# Initialize model
model             <- list()
model$modelsense  <- 'max'
model$modelname   <- 'poolsearch'

# Set variables
model$obj         <- objCoef
model$vtype       <- 'B'
model$lb          <- 0
model$ub          <- 1
model$varnames    <- sprintf('El%d', seq(1,groundSetSize))

# Build constraint matrix
model$A           <- spMatrix(1, groundSetSize,
                              i = rep(1,groundSetSize),
                              j = 1:groundSetSize,
                              x = knapsackCoef)
model$rhs         <- c(Budget)
model$sense       <- c('<')
model$constrnames <- c('Budget')

# Set poolsearch parameters
params                <- list()
params$PoolSolutions  <- 1024
params$PoolGap        <- 0.10
params$PoolSearchMode <- 2

# Save problem
gurobi_write(model, 'poolsearch_R.lp')

# Optimize
result <- gurobi(model, params)

# Capture solution information
if (result$status != 'OPTIMAL') {
  cat('Optimization finished with status', result$status, '\n')
  stop('Stop now\n')
}

# Print best solution
cat('Selected elements in best solution:\n')
cat(model$varnames[which(result$x>=0.9)],'\n')

# Print all solution objectives and best furth solution
if ('pool' %in% names(result)) {
  solcount <- length(result$pool)
  cat('Number of solutions found:', solcount, '\n')
  cat('Objective values for first', solcount, 'solutions:\n')
  for (k in 1:solcount) {
    cat(result$pool[[k]]$objval,' ',sep='')
  }
  cat('\n')
  if (solcount >= 4) {
    cat('Selected elements in fourth best solution:\n')
    cat(model$varnames[which(result$pool[[4]]$xn >= 0.9)], '\n')
  }
} else {
  solcount <- 1
  cat('Number of solutions found:', solcount, '\n')
  cat('Solution 1 has objective:', result$objval, '\n')
}

# Clean up
rm(model, params, result)
