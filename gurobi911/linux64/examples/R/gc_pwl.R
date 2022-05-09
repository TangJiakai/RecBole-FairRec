# Copyright 2020, Gurobi Optimization, LLC
#
# This example formulates and solves the following simple model
# with PWL constraints:
#
#  maximize
#        sum c(j) * x(j)
#  subject to
#        sum A(i,j) * x(j) <= 0,  for i = 1, ..., m
#        sum y(j) <= 3
#        y(j) = pwl(x(j)),        for j = 1, ..., n
#        x(j) free, y(j) >= 0,    for j = 1, ..., n
#
#  where pwl(x) = 0,     if x  = 0
#               = 1+|x|, if x != 0
#
#  Note
#   1. sum pwl(x(j)) <= b is to bound x vector and also to favor sparse x vector.
#      Here b = 3 means that at most two x(j) can be nonzero and if two, then
#      sum x(j) <= 1
#   2. pwl(x) jumps from 1 to 0 and from 0 to 1, if x moves from negative 0 to 0,
#      then to positive 0, so we need three points at x = 0. x has infinite bounds
#      on both sides, the piece defined with two points (-1, 2) and (0, 1) can
#      extend x to -infinite. Overall we can use five points (-1, 2), (0, 1),
#      (0, 0), (0, 1) and (1, 2) to define y = pwl(x)

library(gurobi)
library(Matrix)

n = 5

# A x <= 0
A <- rbind(c(0, 0, 0, 1, -1),
            c(0, 0, 1, 1, -1),
            c(1, 1, 0, 0, -1),
            c(1, 0, 1, 0, -1),
            c(1, 0, 0, 1, -1))

# sum y(j) <= 3
y <- rbind(c(1, 1, 1, 1, 1))

# Initialize model
model <- list()

# Constraint matrix
model$A <- bdiag(A, y)

# Right-hand-side coefficient vector
model$rhs <- c(rep(0, n), 3)

# Objective function (x coefficients arbitrarily chosen)
model$obj <- c(0.5, 0.8, 0.5, 0.1, -1, rep(0, n))

# It's a maximization model
model$modelsense <- "max"

# Lower bounds for x and y
model$lb <- c(rep(-Inf, n), rep(0, n))

# PWL constraints
model$genconpwl <- list()
for (k in 1:n) {
    model$genconpwl[[k]]      <- list()
    model$genconpwl[[k]]$xvar <- k
    model$genconpwl[[k]]$yvar <- n + k
    model$genconpwl[[k]]$xpts <- c(-1, 0, 0, 0, 1)
    model$genconpwl[[k]]$ypts <- c(2, 1, 0, 1, 2)
}

# Solve the model and collect the results
result <- gurobi(model)

# Display solution values for x
for (k in 1:n)
    print(sprintf('x(%d) = %g', k, result$x[k]))

print(sprintf('Objective value: %g', result$objval))

# Clear space
rm(model, result)
