# Copyright 2020, Gurobi Optimization, LLC
#
# This example formulates and solves the following simple SOS model:
#  maximize
#        2 x + y + z
#  subject to
#        x1 = 0 or x2 = 0 (SOS1 constraint)
#        x1 = 0 or x3 = 0 (SOS1 constraint)
#        x1 <= 1, x2 <= 1, x3 <= 2

library(gurobi)

model <- list()

model$A          <- matrix(c(0,0,0), nrow=1, byrow=T)
model$obj        <- c(2,1,1)
model$modelsense <- 'max'
model$ub         <- c(1,1,2)
model$rhs        <- c(0)
model$sense      <- c('=')

# First SOS: x1 = 0 or x2 = 0
sos1 <- list()
sos1$type <- 1
sos1$index <- c(1, 2)
sos1$weight <- c(1, 2)

# Second SOS: x1 = 0 or x3 = 0
sos2 <- list()
sos2$type <- 1
sos2$index <- c(1, 3)
sos2$weight <- c(1, 2)

model$sos <- list(sos1, sos2)

result <- gurobi(model)

print(result$objval)
print(result$x)

# Clear space
rm(model, result)
