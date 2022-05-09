#  Copyright 2020, Gurobi Optimization, LLC */
# 
# Sudoku example.
# 
# The Sudoku board is a 9x9 grid, which is further divided into a 3x3 grid
# of 3x3 grids.  Each cell in the grid must take a value from 0 to 9.
# No two grid cells in the same row, column, or 3x3 subgrid may take the
# same value.
# 
# In the MIP formulation, binary variables x[i,j,v] indicate whether
# cell <i,j> takes value 'v'.  The constraints are as follows:
#   1. Each cell must take exactly one value (sum_v x[i,j,v] = 1)
#   2. Each value is used exactly once per row (sum_i x[i,j,v] = 1)
#   3. Each value is used exactly once per column (sum_j x[i,j,v] = 1)
#   4. Each value is used exactly once per 3x3 subgrid (sum_grid x[i,j,v] = 1)
# 
# Input datasets for this example can be found in examples/data/sudoku*.
# 

library(Matrix)
library(gurobi)

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
  stop('Usage: Rscript sudoku.R filename\n')
}

# Read input file
conn <- file(args[1], open='r')
if(!isOpen(conn)) {
  cat('Could not read file',args[1],'\n')
  stop('Stop now\n')
}
linn <- readLines(conn)
close(conn)

# Ensure that all lines have the same length as the number of lines, and
# that the character set is the correct one.
# Load fixed positions in board
Dim    <- length(linn)
board  <- matrix(0, Dim, Dim, byrow = TRUE)
if (Dim != 9) {
  cat('Input file',args[1],'has',Dim,'lines instead of 9\n')
  stop('Stop now\n')
}
for (i in 1:Dim) {
  line <- strsplit(linn[[i]],split='')[[1]]
  if (length(line) != Dim) {
    cat('Input line',i,'has',length(line),'characters, expected',Dim,'\n')
    stop('Stop now\n')
  }
  for (j in 1:Dim) {
    if (line[[j]] != '.') {
      k <- as.numeric(line[[j]])
      if (k < 1 || k > Dim) {
        cat('Unexpected character in Input line',i,'character',j,'\n')
        stop('Stop now\n')
      } else {
        board[i,j] = k
      }
    }
  }
}

# Map X[i,j,k] into an index variable in the model
nVars  <- Dim * Dim * Dim
varIdx <- function(i,j,k) {i + (j-1) * Dim + (k-1) * Dim * Dim}

cat('Dataset grid:',Dim,'x',Dim,'\n')

# Set-up environment
env <- list()
env$logfile <- 'sudoku.log'

# Build model
model            <- list()
model$modelname  <- 'sudoku'
model$modelsense <- 'min'

# Create variable names, types, and bounds
model$vtype    <- 'B'
model$lb       <- rep(0,  nVars)
model$ub       <- rep(1,  nVars)
model$varnames <- rep('', nVars)
for (i in 1:Dim) {
  for (j in 1:Dim) {
    for (k in 1:Dim) {
      if (board[i,j] == k) model$lb[varIdx(i,j,k)] = 1
      model$varnames[varIdx(i,j,k)] = paste0('X',i,j,k)
    }
  }
}

# Create (empty) constraints:
model$A           <- spMatrix(0,nVars)
model$rhs         <- c()
model$sense       <- c()
model$constrnames <- c()

# Each cell gets a value:
for (i in 1:Dim) {
  for (j in 1:Dim) {
    B <- spMatrix(1, nVars,
            i = rep(1,Dim),
            j = varIdx(i,j,1:Dim),
            x = rep(1,Dim))
    model$A           <- rbind(model$A, B)
    model$rhs         <- c(model$rhs, 1)
    model$sense       <- c(model$sense, '=')
    model$constrnames <- c(model$constrnames, paste0('OneValInCell',i,j))
  }
}

# Each value must appear once in each column
for (i in 1:Dim) {
  for (k in 1:Dim) {
    B <- spMatrix(1, nVars,
            i = rep(1,Dim),
            j = varIdx(i,1:Dim,k),
            x = rep(1,Dim))
    model$A           <- rbind(model$A, B)
    model$rhs         <- c(model$rhs, 1)
    model$sense       <- c(model$sense, '=')
    model$constrnames <- c(model$constrnames, paste0('OnceValueInRow',i,k))
  }
}

#Each value must appear once in each row
for (j in 1:Dim) {
  for (k in 1:Dim) {
    B <- spMatrix(1, nVars,
            i = rep(1,Dim),
            j = varIdx(1:Dim,j,k),
            x = rep(1,Dim))
    model$A           <- rbind(model$A, B)
    model$rhs         <- c(model$rhs, 1)
    model$sense       <- c(model$sense, '=')
    model$constrnames <- c(model$constrnames, paste0('OnceValueInColumn',j,k))
  }
}

# Each value must appear once in each subgrid
SubDim <- 3
for (k in 1:Dim) {
  for (g1 in 1:SubDim) {
    for (g2 in 1:SubDim) {
      B <- spMatrix(1, nVars,
              i = rep(1,Dim),
              j = c(varIdx(1+(g1-1)*SubDim,(g2-1)*SubDim + 1:SubDim, k),
                    varIdx(2+(g1-1)*SubDim,(g2-1)*SubDim + 1:SubDim, k),
                    varIdx(3+(g1-1)*SubDim,(g2-1)*SubDim + 1:SubDim, k)),
              x = rep(1,Dim))
      model$A           <- rbind(model$A, B)
      model$rhs         <- c(model$rhs, 1)
      model$sense       <- c(model$sense, '=')
      model$constrnames <- c(model$constrnames,
                             paste0('OnceValueInSubGrid',g1,g2,k))
    }
  }
}

# Save model
gurobi_write(model, 'sudoku.lp', env)

# Optimize model
result <- gurobi(model, env =  env)

if (result$status == 'OPTIMAL') {
  cat('Solution:\n')
  cat('----------------------------------\n')
  for (i in 1:Dim) {
    for (j in 1:Dim) {
      if (j %% SubDim == 1) cat('| ')
      for (k in 1:Dim) {
        if (result$x[varIdx(i,j,k)] > 0.99) {
          cat(k,' ')
        }
      }
    }
    cat('|\n')
    if (i %% SubDim == 0) cat('----------------------------------\n')
  }
} else {
  cat('Problem was infeasible\n')
}

# Clear space
rm(result, model, board, linn, env)
