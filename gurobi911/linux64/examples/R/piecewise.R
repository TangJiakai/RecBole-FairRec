# Copyright 2020, Gurobi Optimization, LLC
#
# This example considers the following separable, convex problem:
#
#  minimize
#        f(x) - y + g(z)
#  subject to
#        x + 2 y + 3 z   <= 4
#        x +   y         >= 1
#        x,    y,    z   <= 1
#
# where f(u) = exp(-u) and g(u) = 2 u^2 - 4u, for all real u.  It
# formulates and solves a simpler LP model by approximating f and
# g with piecewise-linear functions.  Then it transforms the model
# into a MIP by negating the approximation for f, which gives
# a non-convex piecewise-linear function, and solves it again.

library(gurobi)

model <- list()

model$A     <- matrix(c(1,2,3,1,1,0), nrow=2, byrow=T)
model$obj   <- c(0,-1,0)
model$ub    <- c(1,1,1)
model$rhs   <- c(4,1)
model$sense <- c('<', '>')

# Uniformly spaced points in [0.0, 1.0]
u <- seq(from=0, to=1, by=0.01)

# First piecewise-linear function: f(x) = exp(-x)
pwl1     <- list()
pwl1$var <- 1
pwl1$x   <- u
pwl1$y   <- sapply(u, function(x) exp(-x))

# Second piecewise-linear function: g(z) = 2 z^2 - 4 z
pwl2     <- list()
pwl2$var <- 3
pwl2$x   <- u
pwl2$y   <- sapply(u, function(z) 2 * z * z - 4 * z)

model$pwlobj <- list(pwl1, pwl2)

result <- gurobi(model)

print(result$objval)
print(result$x)


# Negate piecewise-linear function on x, making it non-convex

model$pwlobj[[1]]$y <- sapply(u, function(x) -exp(-x))

result <- gurobi(model)
gurobi_write(model, "pwl.lp")

print(result$objval)
print(result$x)

# Clear space
rm(model, pwl1, pwl2, result)
