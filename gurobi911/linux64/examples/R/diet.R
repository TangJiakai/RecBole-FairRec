# Copyright 2020, Gurobi Optimization, LLC
#
# Solve the classic diet model, showing how to add constraints
# to an existing model.

library(Matrix)
library(gurobi)

# display results
printSolution <- function(model, res, nCategories, nFoods) {
  if (res$status == 'OPTIMAL') {
    cat('\nCost: ',res$objval,'\nBuy:\n')
    for (j in nCategories + 1:nFoods) {
      if (res$x[j] > 1e-4) {
        cat(format(model$varnames[j],justify='left',width=10),':',
            format(res$x[j],justify='right',width=10,nsmall=2),'\n')
      }
    }
    cat('\nNutrition:\n')
    for (j in 1:nCategories) {
      cat(format(model$varnames[j],justify='left',width=10),':',
          format(res$x[j],justify='right',width=10,nsmall=2),'\n')
    }
  } else {
    cat('No solution\n')
  }
}

# define primitive data
Categories      <- c('calories', 'protein', 'fat', 'sodium')
nCategories     <- length(Categories)
minNutrition    <- c(     1800 ,       91 ,    0 ,       0 )
maxNutrition    <- c(     2200 ,      Inf ,   65 ,    1779 )

Foods           <- c('hamburger', 'chicken', 'hot dog', 'fries', 'macaroni',
                     'pizza', 'salad', 'milk', 'ice cream')
nFoods          <- length(Foods)
cost            <- c(2.49, 2.89, 1.50, 1.89, 2.09, 1.99, 2.49, 0.89, 1.59)
nutritionValues <- c( 410, 24, 26 ,  730,
                      420, 32, 10 , 1190,
                      560, 20, 32 , 1800,
                      380,  4, 19 ,  270,
                      320, 12, 10 ,  930,
                      320, 15, 12 ,  820,
                      320, 31, 12 , 1230,
                      100,  8, 2.5,  125,
                      330,  8, 10 ,  180 )
# Build model
model     <- list()
model$A   <- spMatrix(nCategories, nCategories + nFoods,
               i = c(mapply(rep,1:4,1+nFoods)),
               j = c(1, (nCategories+1):(nCategories+nFoods),
                     2, (nCategories+1):(nCategories+nFoods),
                     3, (nCategories+1):(nCategories+nFoods),
                     4, (nCategories+1):(nCategories+nFoods) ),
               x = c(-1.0, nutritionValues[1 + nCategories*(0:(nFoods-1))],
                     -1.0, nutritionValues[2 + nCategories*(0:(nFoods-1))],
                     -1.0, nutritionValues[3 + nCategories*(0:(nFoods-1))],
                     -1.0, nutritionValues[4 + nCategories*(0:(nFoods-1))] ))
model$obj         <- c(rep(0, nCategories), cost)
model$lb          <- c(minNutrition, rep(0, nFoods))
model$ub          <- c(maxNutrition, rep(Inf, nFoods))
model$varnames    <- c(Categories, Foods)
model$rhs         <- rep(0,nCategories)
model$sense       <- rep('=',nCategories)
model$constrnames <- Categories
model$modelname   <- 'diet'
model$modelsense  <- 'min'

# Optimize
res <- gurobi(model)
printSolution(model, res, nCategories, nFoods)

# Adding constraint: at most 6 servings of dairy
# this is the matrix part of the constraint
B <- spMatrix(1, nCategories + nFoods,
              i = rep(1,2),
              j = (nCategories+c(8,9)),
              x = rep(1,2))
# append B to A
model$A           <- rbind(model$A,       B)
# extend row-related vectors
model$constrnames <- c(model$constrnames, 'limit_dairy')
model$rhs         <- c(model$rhs,         6)
model$sense       <- c(model$sense,       '<')

# Optimize
res <- gurobi(model)
printSolution(model, res, nCategories, nFoods)

# Clear space
rm(res, model)
