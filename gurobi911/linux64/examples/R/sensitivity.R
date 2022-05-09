# Copyright 2020, Gurobi Optimization, LLC
#
# A simple sensitivity analysis example which reads a MIP model
# from a file and solves it. Then each binary variable is set
# to 1-X, where X is its value in the optimal solution, and
# the impact on the objective function value is reported.

library(Matrix)
library(gurobi)

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
  stop('Usage: Rscript sensitivity.R filename\n')
}

# Read model
cat('Reading model',args[1],'...')
model <- gurobi_read(args[1])
cat('... done\n')

# Detect set of non-continous variables
numvars    <- ncol(model$A)
intvars    <- which(model$vtype != 'C')
numintvars <- length(intvars)
if (numintvars < 1) {
  stop('All model\'s variables are continuous, nothing to do\n')
}
maxanalyze <- 10

# Optimize
result <- gurobi(model)

# Capture solution information
if (result$status != 'OPTIMAL') {
  cat('Optimization finished with status', result$status, '\n')
  stop('Stop now\n')
}
origx       <- result$x
origobjval  <- result$objval

# create lb and ub if they do not exists, and set them to default values
if (!('lb' %in% names(model))) {
  model$lb <- numeric(numvars)
}
if (!('ub' %in% names(model))) {
  # This line is not needed, as we must have ub defined
  model$ub <- Inf + numeric(numvars)
}

# Disable output for subsequent solves
params            <- list()
params$OutputFlag <- 0

# We limit the sensitivity analysis to a maximum number of variables
numanalyze <- 0

# Iterate through unfixed binary variables in the model
for (j in 1:numvars) {
  if (model$vtype[j] != 'B' &&
      model$vtype[j] != 'I'   ) next
  if (model$vtype[j] == 'I') {
    if (model$lb[j] != 0.0)     next
    if (model$ub[j] != 1.0)     next
  } else {
    if (model$lb[j] > 0.0)      next
    if (model$ub[j] < 1.0)      next
  }

  # Update MIP start for all variables
  model$start <- origx

  # Set variable to 1-X, where X is its value in optimal solution
  if (origx[j] < 0.5) {
    model$start[j] <- 1
    model$lb[j]    <- 1
  } else {
    model$start[j] <- 0
    model$ub[j]    <- 0
  }

  # Optimize
  result <- gurobi(model, params)

  # Display result
  varnames <- ''
  if ('varnames' %in% names(model)) {
    varnames <- model$varnames[j]
  } else {
    varnames <- sprintf('%s%d', model$vtype[j], j)
  }
  gap <- 0
  if (result$status != 'OPTIMAL') {
    gap <- Inf
  } else {
    gap <- result$objval - origobjval
  }
  cat('Objective sensitivity for variable', varnames, 'is', gap, '\n')

  # Restore original bounds
  model$lb[j] <- 0
  model$ub[j] <- 1

  numanalyze <- numanalyze + 1

  # Stop when we reached the maximum number of sensitivity analysis steps
  if (numanalyze >= maxanalyze) {
      cat('Limit sensitivity analysis to the first', maxanalyze, 'variables\n')
      break
  }
}

# Clear space
rm(model, params, result, origx)
