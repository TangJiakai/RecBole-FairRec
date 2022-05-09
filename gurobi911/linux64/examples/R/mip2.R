# Copyright 2020, Gurobi Optimization, LLC
#
# This example reads a MIP model from a file, solves it and
# prints the objective values from all feasible solutions
# generated while solving the MIP. Then it creates the fixed
# model and solves that model.

library(Matrix)
library(gurobi)

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
  stop('Usage: Rscript mip2.R filename\n')
}

# Read model
cat('Reading model',args[1],'...')
model <- gurobi_read(args[1])
cat('... done\n')

# Detect set of non-continous variables
numvars    <- dim(model$A)[[2]]
intvars    <- which(model$vtype != 'C')
numintvars <- length(intvars)
if (numintvars < 1) {
  stop('All model\'s variables are continuous, nothing to do\n')
}

# Optimize
params               <- list()
params$poolsolutions <- 20
result               <- gurobi(model, params)

# Capture solution information
if (result$status != 'OPTIMAL') {
  cat('Optimization finished with status', result$status, '\n')
  stop('Stop now\n')
}

# Iterate over the solutions
if ('pool' %in% names(result)) {
  solcount <- length(result$pool)
  for (k in 1:solcount) {
    cat('Solution', k, 'has objective:', result$pool[[k]]$objval, '\n')
  }
} else {
  solcount <- 1
  cat('Solution 1 has objective:', result$objval, '\n')
}

# Convert to fixed model
for (j in 1:numvars) {
  if (model$vtype[j] != 'C') {
    t <- floor(result$x[j]+0.5)
    model$lb[j] <- t
    model$ub[j] <- t
  }
}

# Solve the fixed model
result2 <- gurobi(model, params)
if (result2$status != 'OPTIMAL') {
  stop('Error: fixed model isn\'t optimal\n')
}

if (abs(result$objval - result2$objval) > 1e-6 * (1 + abs(result$objval))) {
  stop('Error: Objective values differ\n')
}

# Print values of non-zero variables
for (j in 1:numvars) {
  if (abs(result2$x[j]) < 1e-6) next
  varnames <- ''
  if ('varnames' %in% names(model)) {
    varnames <- model$varnames[j]
  } else {
    varnames <- sprintf('X%d', j)
  }
  cat(format(varnames, justify='left', width=10),':',
      format(result2$x[j], justify='right', digits=2, width=10), '\n')
}

# Clear space
rm(model, params, result, result2)
