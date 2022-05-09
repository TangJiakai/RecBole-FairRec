# Copyright 2020, Gurobi Optimization, LLC
#
# This example reads an LP model from a file and solves it.
# If the model can be solved, then it finds the smallest positive variable,
# sets its upper bound to zero, and resultolves the model two ways:
# first with an advanced start, then without an advanced start
# (i.e. 'from scratch').

library(Matrix)
library(gurobi)

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
  stop('Usage: Rscript lpmod.R filename\n')
}

# Read model
cat('Reading model',args[1],'...')
model <- gurobi_read(args[1])
cat('... done\n')

# Determine whether it is an LP
if ('multiobj'  %in% names(model) ||
    'sos'       %in% names(model) ||
    'pwlobj'    %in% names(model) ||
    'cones'     %in% names(model) ||
    'quadcon'   %in% names(model) ||
    'genconstr' %in% names(model)   ) {
  stop('The model is not a linear program\n')
}

# Detect set of non-continuous variables
intvars    <- which(model$vtype != 'C')
numintvars <- length(intvars)
if (numintvars > 0) {
  stop('problem is a MIP, nothing to do\n')
}

# Optimize
result <- gurobi(model)
if (result$status != 'OPTIMAL') {
  cat('This model cannot be solved because its optimization status is',
      result$status, '\n')
  stop('Stop now\n')
}

# Recover number of variables in model
numvars   <- ncol(model$A)

# Ensure bounds array is initialized
if (is.null(model$lb)) {
  model$lb <- rep(0, numvars)
}
if (is.null(model$ub)) {
  model$ub <- rep(Inf, numvars)
}

# Find smallest (non-zero) variable value with zero lower bound
x      <- replace(result$x, result$x < 1e-4, Inf)
x      <- replace(x, model$lb > 1e-6, Inf)
minVar <- which.min(x)
minVal <- x[minVar]

# Get variable name
varname <- ''
if (is.null(model$varnames)) {
  varname <- sprintf('C%d',minVar)
} else {
  varname <- model$varnames[minVar]
}

cat('\n*** Setting', varname, 'from', minVal, 'to zero ***\n\n')
model$ub[minVar] <- 0

# Set advance start basis information
model$vbasis <- result$vbasis
model$cbasis <- result$cbasis

result2   <- gurobi(model)
warmCount <- result2$itercount
warmTime  <- result2$runtime

# Reset-advance start information
model$vbasis <- NULL
model$cbasis <- NULL

result2   <- gurobi(model)
coldCount <- result2$itercount
coldTime  <- result2$runtime

cat('\n*** Warm start:', warmCount, 'iterations,', warmTime, 'seconds\n')
cat('\n*** Cold start:', coldCount, 'iterations,', coldTime, 'seconds\n')

# Clear space
rm(model, result, result2)
