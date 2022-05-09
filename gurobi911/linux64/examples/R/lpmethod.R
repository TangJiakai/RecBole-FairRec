# Copyright 2020, Gurobi Optimization, LLC
#
# Solve a model with different values of the Method parameter;
# show which value gives the shortest solve time.

library(Matrix)
library(gurobi)

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
  stop('Usage: Rscript lpmethod.R filename\n')
}

# Read model
cat('Reading model',args[1],'...')
model <- gurobi_read(args[1])
cat('... done\n')

# Solve the model with different values of Method
params     <- list()
bestTime   <- Inf
bestMethod <- -1
for (i in 0:4) {
  params$method <- i
  res <- gurobi(model, params)
  if (res$status == 'OPTIMAL') {
    bestMethod       <- i
    bestTime         <- res$runtime
    params$TimeLimit <- bestTime
  }
}

# Report which method was fastest
if (bestMethod == -1) {
  cat('Unable to solve this model\n')
} else {
  cat('Solved in', bestTime, 'seconds with Method:', bestMethod, '\n')
}

rm(params, model)
