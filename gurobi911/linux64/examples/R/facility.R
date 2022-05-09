# Copyright 2020, Gurobi Optimization, LLC
#
# Facility location: a company currently ships its product from 5 plants
# to 4 warehouses. It is considering closing some plants to reduce
# costs. What plant(s) should the company close, in order to minimize
# transportation and fixed costs?
#
# Note that this example uses lists instead of dictionaries.  Since
# it does not work with sparse data, lists are a reasonable option.
#
# Based on an example from Frontline Systems:
#   http://www.solver.com/disfacility.htm
# Used with permission.

library(Matrix)
library(gurobi)

# define primitive data
nPlants     <- 5
nWarehouses <- 4
# Warehouse demand in thousands of units
Demand      <- c(15, 18, 14, 20)
# Plant capacity in thousands of units
Capacity    <- c(20, 22, 17, 19, 18)
# Fixed costs for each plant
FixedCosts  <- c( 12000, 15000, 17000, 13000, 16000)
# Transportation costs per thousand units
TransCosts  <- c(4000, 2000, 3000, 2500, 4500,
                 2500, 2600, 3400, 3000, 4000,
                 1200, 1800, 2600, 4100, 3000,
                 2200, 2600, 3100, 3700, 3200 )

flowidx <- function(w, p) {nPlants * (w-1) + p}

# Build model
model <- list()
model$modelname <- 'facility'
model$modelsense <- 'min'

# initialize data for variables
model$lb       <- 0
model$ub       <- c(rep(1, nPlants),   rep(Inf, nPlants * nWarehouses))
model$vtype    <- c(rep('B', nPlants), rep('C', nPlants * nWarehouses))
model$obj      <- c(FixedCosts, TransCosts)
model$varnames <- c(paste0(rep('Open',nPlants),1:nPlants),
                    sprintf('Trans%d,%d',
                            c(mapply(rep,1:nWarehouses,nPlants)),
                            1:nPlants))

# build production constraint matrix
A1 <- spMatrix(nPlants, nPlants, i = c(1:nPlants), j = (1:nPlants), x = -Capacity)
A2 <- spMatrix(nPlants, nPlants * nWarehouses,
               i = c(mapply(rep, 1:nPlants, nWarehouses)),
               j = mapply(flowidx,1:nWarehouses,c(mapply(rep,1:nPlants,nWarehouses))),
               x = rep(1, nWarehouses * nPlants))
A3 <- spMatrix(nWarehouses, nPlants)
A4 <- spMatrix(nWarehouses, nPlants * nWarehouses,
               i = c(mapply(rep, 1:nWarehouses, nPlants)),
               j = mapply(flowidx,c(mapply(rep,1:nWarehouses,nPlants)),1:nPlants),
               x = rep(1, nPlants * nWarehouses))
model$A           <- rbind(cbind(A1, A2), cbind(A3, A4))
model$rhs         <- c(rep(0, nPlants),   Demand)
model$sense       <- c(rep('<', nPlants), rep('=', nWarehouses))
model$constrnames <- c(sprintf('Capacity%d',1:nPlants),
                       sprintf('Demand%d',1:nWarehouses))

# Save model
gurobi_write(model,'facilityR.lp')

# Guess at the starting point: close the plant with the highest fixed
# costs; open all others first open all plants
model$start <- c(rep(1,nPlants),rep(NA, nPlants * nWarehouses))

# find most expensive plant, and close it in mipstart
cat('Initial guess:\n')
worstidx <- which.max(FixedCosts)
model$start[worstidx] <- 0
cat('Closing plant',worstidx,'\n')

# set parameters
params <- list()
params$method <- 2

# Optimize
res <- gurobi(model, params)

# Print solution
if (res$status == 'OPTIMAL') {
  cat('\nTotal Costs:',res$objval,'\nsolution:\n')
  cat('Facilities:', model$varnames[which(res$x[1:nPlants]>1e-5)], '\n')
  active <- nPlants + which(res$x[(nPlants+1):(nPlants*(nWarehouses+1))] > 1e-5)
  cat('Flows: ')
  cat(sprintf('%s=%g ',model$varnames[active], res$x[active]), '\n')
  rm(active)
} else {
  cat('No solution\n')
}

# Clear space
rm(res, model, params, A1, A2, A3, A4)
