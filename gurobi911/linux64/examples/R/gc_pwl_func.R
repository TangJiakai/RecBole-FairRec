# Copyright 2020, Gurobi Optimization, LLC
#
# This example considers the following nonconvex nonlinear problem
#
#  maximize    2 x    + y
#  subject to  exp(x) + 4 sqrt(y) <= 9
#              x, y >= 0
#
#  We show you two approaches to solve this:
#
#  1) Use a piecewise-linear approach to handle general function
#     constraints (such as exp and sqrt).
#     a) Add two variables
#        u = exp(x)
#        v = sqrt(y)
#     b) Compute points (x, u) of u = exp(x) for some step length (e.g., x
#        = 0, 1e-3, 2e-3, ..., xmax) and points (y, v) of v = sqrt(y) for
#        some step length (e.g., y = 0, 1e-3, 2e-3, ..., ymax). We need to
#        compute xmax and ymax (which is easy for this example, but this
#        does not hold in general).
#     c) Use the points to add two general constraints of type
#        piecewise-linear.
#
#  2) Use the Gurobis built-in general function constraints directly (EXP
#     and POW). Here, we do not need to compute the points and the maximal
#     possible values, which will be done internally by Gurobi.  In this
#     approach, we show how to "zoom in" on the optimal solution and
#     tighten tolerances to improve the solution quality.

library(gurobi)

printsol <- function(model, result) {
    print(sprintf('%s = %g, %s = %g',
                  model$varnames[1], result$x[1],
                  model$varnames[3], result$x[3]))
    print(sprintf('%s = %g, %s = %g',
                  model$varnames[2], result$x[2],
                  model$varnames[4], result$x[4]))
    print(sprintf('Obj = %g',  + result$objval))

    # Calculate violation of exp(x) + 4 sqrt(y) <= 9
    vio <- exp(result$x[1]) + 4 * sqrt(result$x[2]) - 9
    if (vio < 0.0)
           vio <- 0.0
    print(sprintf('Vio = %g', vio))
}

model <- list()

# Four nonneg. variables x, y, u, v, one linear constraint u + 4*v <= 9
model$varnames <- c('x', 'y', 'u', 'v')
model$lb       <- c(rep(0, 4))
model$ub       <- c(rep(Inf, 4))
model$A        <- matrix(c(0, 0, 1, 4), nrow = 1)
model$rhs      <- 9

# Objective
model$modelsense <- 'max'
model$obj        <- c(2, 1, 0, 0)

# First approach: PWL constraints
model$genconpwl <- list()

intv <- 1e-3

# Approximate u \approx exp(x), equispaced points in [0, xmax], xmax = log(9)
model$genconpwl[[1]]      <- list()
model$genconpwl[[1]]$xvar <- 1L
model$genconpwl[[1]]$yvar <- 3L

xmax <- log(9)
point <- 0
t     <- 0
while (t < xmax + intv) {
    point <- point + 1
    model$genconpwl[[1]]$xpts[point] <- t
    model$genconpwl[[1]]$ypts[point] <- exp(t)
    t <- t + intv
}

# Approximate v \approx sqrt(y), equispaced points in [0, ymax], ymax = (9/4)^2
model$genconpwl[[2]]      <- list()
model$genconpwl[[2]]$xvar <- 2L
model$genconpwl[[2]]$yvar <- 4L

ymax  <- (9/4)^2
point <- 0
t     <- 0
while (t < ymax + intv) {
    point <- point + 1
    model$genconpwl[[2]]$xpts[point] <- t
    model$genconpwl[[2]]$ypts[point] <- sqrt(t)
    t <- t + intv
}

# Solve and print solution
result = gurobi(model)
printsol(model, result)

# Second approach: General function constraint approach with auto PWL
#                  translation by Gurobi

# Delete explicit PWL approximations from model
model$genconpwl <-  NULL

# Set u \approx exp(x)
model$genconexp           <- list()
model$genconexp[[1]]      <- list()
model$genconexp[[1]]$xvar <- 1L
model$genconexp[[1]]$yvar <- 3L
model$genconexp[[1]]$name <- 'gcf1'

# Set v \approx sqrt(y) = y^0.5
model$genconpow           <- list()
model$genconpow[[1]]      <- list()
model$genconpow[[1]]$xvar <- 2L
model$genconpow[[1]]$yvar <- 4L
model$genconpow[[1]]$a    <- 0.5
model$genconpow[[1]]$name <- 'gcf2'

# Parameters for discretization: use equal piece length with length = 1e-3
params                 <- list()
params$FuncPieces      <- 1
params$FuncPieceLength <- 1e-3

# Solve and print solution
result = gurobi(model, params)
printsol(model, result)

# Zoom in, use optimal solution to reduce the ranges and use a smaller
# pclen=1-5 to resolve
model$lb[1] <- max(model$lb[1], result$x[1] - 0.01)
model$ub[1] <- min(model$ub[1], result$x[1] + 0.01)
model$lb[2] <- max(model$lb[2], result$x[2] - 0.01)
model$ub[2] <- min(model$ub[2], result$x[2] + 0.01)
params$FuncPieceLength <- 1e-5

# Solve and print solution
result = gurobi(model, params)
printsol(model, result)

# Clear space
rm(model, result)
