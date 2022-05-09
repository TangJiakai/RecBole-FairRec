# Copyright 2020, Gurobi Optimization, LLC
#
# Implement a simple MIP heuristic.  Relax the model,
# sort variables based on fractionality, and fix the 25% of
# the fractional variables that are closest to integer variables.
# Repeat until either the relaxation is integer feasible or
# linearly infeasible.

library(Matrix)
library(gurobi)

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
  stop('Usage: Rscript fixanddive.R filename\n')
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

# create lb and ub if they do not exists, and set them to default values
if (!('lb' %in% model)) {
  model$lb <- numeric(numvars)
}
if (!('ub' %in% model)) {
  model$ub <- Inf + numeric(numvars)
}

# set all variables to continuous
ovtype                 <- model$vtype
model$vtype[1:numvars] <- 'C'

# parameters
params            <- list()
params$OutputFlag <- 0

result <- gurobi(model, params)

# Perform multiple iterations. In each iteration, identify the first
# quartile of integer variables that are closest to an integer value
# in the relaxation, fix them to the nearest integer, and repeat.
for (iter in 1:1000) {
  # See if status is optimal
  if (result$status != 'OPTIMAL') {
    cat('Model status is', result$status,'\n')
    cat('Cannot keep fixing variables\n')
    break
  }
  # collect fractionality of integer variables
  fractional  <- abs(result$x - floor(result$x+0.5))
  fractional  <- replace(fractional, fractional < 1e-5, 1)
  fractional  <- replace(fractional, ovtype == 'C', 1)
  fractional  <- replace(fractional, ovtype == 'S', 1)
  nfractional <- length(which(fractional<0.51))

  cat('Iteration:', iter, 'Obj:', result$objval,
      'Fractional:', nfractional, '\n')
  if (nfractional == 0) {
    cat('Found feasible solution - objective', result$objval, '\n')
    break
  }

  # order the set of fractional index
  select <- order(fractional, na.last = TRUE, decreasing = FALSE)

  # fix 25% of variables
  nfix <- as.integer(ceiling(nfractional  / 4))
  # cat('Will fix', nfix, 'variables, out of', numvars, '\n')
  if (nfix < 10)
    cat('Fixing ')
  else
    cat('Fixing',nfix,'variables, fractionality threshold:',fractional[select[nfix]],'\n')
  for (k in 1:nfix) {
    j   <- select[k]
    val <- floor(result$x[j] + 0.5)
    model$lb[j] <- val
    model$ub[j] <- val
    if (nfix < 10)
      cat(model$varname[j],'x*=',result$x[j],'to',val,' ')
  }
  if (nfix < 10)
    cat('\n')

  # reoptimize
  result <- gurobi(model, params)
}

# Clear space
rm(model, params, result)
