# Copyright 2020, Gurobi Optimization, LLC
#
# This example formulates and solves the following simple bilinear model:
#  maximize   
#        x
#  subject to
#        x + y + z <= 10
#        x * y <= 2         (bilinear inequality)
#        x * z + y * z = 1  (bilinear equality)
#        x, y, z non-negative (x integral in second version)

library(gurobi)
library(Matrix)

model <- list()

# Linear constraint matrix
model$A <- matrix(c(1, 1, 1), nrow=1, byrow=T)
model$rhs <- c(10.0)
model$sense <- c('<')

# Variable names
model$varnames <- c('x', 'y', 'z')

# Objective function max 1.0 * x
model$obj <- c(1, 0, 0)
model$modelsense <- 'max'

# Bilinear inequality constraint: x * y <= 2
qc1 <- list()
qc1$Qc <- spMatrix(3, 3, c(1), c(2), c(1.0))
qc1$rhs <- 2.0
qc1$sense <- c('<')
qc1$name <- 'bilinear0'

# Bilinear equality constraint: x * z + y * z == 1
qc2 <- list()
qc2$Qc <- spMatrix(3, 3, c(1, 2), c(3, 3), c(1.0, 1.0))
qc2$rhs <- 1.0
qc2$sense <- c('=')
qc2$name <- 'bilinear1'

model$quadcon <- list(qc1, qc2)

# Solve bilinear model, display solution.  The problem is non-convex,
# we need to set the parameter 'NonConvex' in order to solve it.
params <- list()
params$NonConvex <- 2
result <- gurobi(model, params)
print(result$x)

# Constrain 'x' to be integral and solve again
model$vtype <- c('I', 'C', 'C')
result <- gurobi(model, params)
print(result$x)
