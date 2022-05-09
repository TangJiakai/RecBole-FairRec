# Copyright 2020, Gurobi Optimization, LLC
#
# Use parameters that are associated with a model.
#
# A MIP is solved for a few seconds with different sets of parameters.
# The one with the smallest MIP gap is selected, and the optimization
# is resumed until the optimal solution is found.


library(Matrix)
library(gurobi)

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
  stop('Usage: Rscript params.R filename\n')
}

# Read model
cat('Reading model',args[1],'...')
model <- gurobi_read(args[1])
cat('... done\n')

# Detect set of non-continuous variables
intvars    <- which(model$vtype != 'C')
numintvars <- length(intvars)
if (numintvars < 1) {
  stop('All model\'s variables are continuous, nothing to do\n')
}

# Set a 2 second time limit
params <- list()
params$TimeLimit <- 2
# Now solve the model with different values of MIPFocus
params$MIPFocus <- 0
result          <- gurobi(model, params)
bestgap         <- result$mipgap
bestparams      <- params
for (i in 1:3) {
  params$MIPFocus <- i
  result          <- gurobi(model, params)
  if (result$mipgap < bestgap) {
    bestparams <- params
    bestgap    <- result$mipgap
  }
}

# Finally, reset the time limit and Re-solve model to optimality
bestparams$TimeLimit <- Inf
result <- gurobi(model, bestparams)
cat('Solved with MIPFocus:', bestparams$MIPFocus, '\n')

# Clear space
rm(model, params, result, bestparams)
