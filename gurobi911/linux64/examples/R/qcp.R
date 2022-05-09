# Copyright 2020, Gurobi Optimization, LLC
#
# This example formulates and solves the following simple QCP model:
#  maximize
#        x
#  subject to
#        x + y + z   =  1
#        x^2 + y^2 <= z^2  (second-order cone)
#        x^2 <= yz         (rotated second-order cone)
#        x, y, z non-negative

library(gurobi)
library(Matrix)

model <- list()

model$A          <- matrix(c(1,1,1), nrow=1, byrow=T)
model$modelsense <- 'max'
model$obj        <- c(1,0,0)
model$rhs        <- c(1)
model$sense      <- c('=')

# First quadratic constraint: x^2 + y^2 - z^2 <= 0
qc1 <- list()
qc1$Qc <- spMatrix(3, 3, c(1, 2, 3), c(1, 2, 3), c(1.0, 1.0, -1.0))
qc1$rhs <- 0.0

# Second quadratic constraint: x^2 - yz <= 0
qc2 <- list()
qc2$Qc <- spMatrix(3, 3, c(1, 2), c(1, 3), c(1.0, -1.0))
qc2$rhs <- 0.0

model$quadcon <- list(qc1, qc2)

result <- gurobi(model)

print(result$objval)
print(result$x)

# Clear space
rm(model, result)
