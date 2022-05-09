# Copyright 2020, Gurobi Optimization, LLC
#
# This example formulates and solves the following simple QP model:
#  minimize
#        x^2 + x*y + y^2 + y*z + z^2 + 2 x
#  subject to
#        x + 2 y + 3z >= 4
#        x +   y      >= 1
#        x, y, z non-negative

library(gurobi)

model <- list()

model$A     <- matrix(c(1,2,3,1,1,0), nrow=2, byrow=T)
model$Q     <- matrix(c(1,0.5,0,0.5,1,0.5,0,0.5,1), nrow=3, byrow=T)
model$obj   <- c(2,0,0)
model$rhs   <- c(4,1)
model$sense <- c('>', '>')

result <- gurobi(model)

print(result$objval)
print(result$x)

model$vtype <- c('I', 'I', 'I')

result <- gurobi(model)

print(result$objval)
print(result$x)

# Clear space
rm(model, result)
