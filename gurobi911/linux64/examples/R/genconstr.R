# Copyright 2020, Gurobi Optimization, LLC
# 
# In this example we show the use of general constraints for modeling
# some common expressions. We use as an example a SAT-problem where we
# want to see if it is possible to satisfy at least four (or all) clauses
# of the logical for
# 
# L = (x0 or ~x1 or x2)  and (x1 or ~x2 or x3)  and
#     (x2 or ~x3 or x0)  and (x3 or ~x0 or x1)  and
#     (~x0 or ~x1 or x2) and (~x1 or ~x2 or x3) and
#     (~x2 or ~x3 or x0) and (~x3 or ~x0 or x1)
# 
# We do this by introducing two variables for each literal (itself and its
# negated value), a variable for each clause, and then two
# variables for indicating if we can satisfy four, and another to identify
# the minimum of the clauses (so if it one, we can satisfy all clauses)
# and put these two variables in the objective.
# i.e. the Objective function will be
# 
# maximize Obj0 + Obj1
# 
#  Obj0 = MIN(Clause1, ... , Clause8)
#  Obj1 = 1 -> Clause1 + ... + Clause8 >= 4
# 
# thus, the objective value will be two if and only if we can satisfy all
# clauses; one if and only if at least four clauses can be satisfied, and
# zero otherwise.
# 

library(Matrix)
library(gurobi)

# Set-up environment
env <- list()
env$logfile <- 'genconstr.log'

# define primitive data
nLiterals <- 4
nClauses  <- 8
nObj      <- 2
nVars     <- 2 * nLiterals + nClauses + nObj
Literal <- function(n) {n}
NotLit  <- function(n) {n + nLiterals}
Clause  <- function(n) {2 * nLiterals + n}
Obj     <- function(n) {2 * nLiterals + nClauses + n}

Clauses <- list(c(Literal(1), NotLit(2), Literal(3)),
                c(Literal(2), NotLit(3), Literal(4)),
                c(Literal(3), NotLit(4), Literal(1)),
                c(Literal(4), NotLit(1), Literal(2)),
                c(NotLit(1), NotLit(2), Literal(3)),
                c(NotLit(2), NotLit(3), Literal(4)),
                c(NotLit(3), NotLit(4), Literal(1)),
                c(NotLit(4), NotLit(1), Literal(2)))


# Create model
model <- list()
model$modelname  <- 'genconstr'
model$modelsense <- 'max'

# Create objective function, variable names, and variable types
model$vtype    <- rep('B',nVars)
model$ub       <- rep(1,  nVars)
model$varnames <- c(sprintf('X%d',      seq(1,nLiterals)), 
                    sprintf('notX%d',   seq(1,nLiterals)),
                    sprintf('Clause%d', seq(1,nClauses)),
                    sprintf('Obj%d',    seq(1,nObj)))
model$obj      <- c(rep(0, 2*nLiterals + nClauses), rep(1, nObj))

# Create linear constraints linking literals and not literals
model$A           <- spMatrix(nLiterals, nVars,
                       i = c(seq(1,nLiterals),
                             seq(1,nLiterals)),
                       j = c(seq(1,nLiterals),
                             seq(1+nLiterals,2*nLiterals)),
                       x = rep(1,2*nLiterals))
model$constrnames <- c(sprintf('CNSTR_X%d',seq(1,nLiterals)))
model$rhs         <- rep(1,   nLiterals)
model$sense       <- rep('=', nLiterals)

# Create OR general constraints linking clauses and literals
model$genconor <- lapply(1:nClauses,
                         function(i) list(resvar=Clause(i),
                                          vars  = Clauses[[i]],
                                          name  = sprintf('CNSTR_Clause%d',i)))

# Set-up Obj1 = Min(Clause[1:nClauses])
model$genconmin <- list(list(resvar = Obj(1),
                             vars   = c(seq(Clause(1),Clause(nClauses))),
                             name   = 'CNSTR_Obj1'))


# the indicator constraint excepts the following vector types for the
# linear sum:
#
# - a dense vector, for example
#   c(rep(0, 2*nLiterals), rep(1, nClauses), rep(0,nObj))
# - a sparse vector, for example
#   sparseVector( x = rep(1,nClauses), i = (2*nLiterals+1):(2*nLiterals+nClauses), length=nVars)
# - In case all coefficients are the same, you can pass a vector of length
#   one with the corresponding value which gets expanded to a dense vector
#   with all values being the given one
#
# We use a sparse vector in this example
a <- sparseVector( x = rep(1,nClauses), i = (2*nLiterals+1):(2*nLiterals+nClauses), length=nVars)

# Set-up Obj2 to signal if any clause is satisfied, i.e.
# we use an indicator constraint of the form:
#     (Obj2 = 1) -> sum(Clause[1:nClauses]) >= 4
model$genconind <- list(list(binvar = Obj(2),
                             binval = 1,
                             a      = a,
                             sense  = '>',
                             rhs    = 4,
                             name   = 'CNSTR_Obj2'))

# Save model
gurobi_write(model, 'genconstr.lp', env)

# Optimize
result <- gurobi(model, env = env)

# Check optimization status
if (result$status == 'OPTIMAL') {
  if (result$objval > 1.9) {
    cat('Logical expression is satisfiable\n')
  } else if(result$objval > 0.9) {
    cat('At least four clauses are satisfiable\n')
  } else {
    cat('At most three clauses may be satisfiable\n')
  }
} else {
  cat('Optimization falied\n')
}

# Clear space
rm(model,result,env,Clauses)
