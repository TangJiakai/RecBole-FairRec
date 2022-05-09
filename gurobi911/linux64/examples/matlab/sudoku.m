function sudoku(filename)

%  Copyright 2020, Gurobi Optimization, LLC */
%
% Sudoku example.
%
% The Sudoku board is a 9x9 grid, which is further divided into a 3x3 grid
% of 3x3 grids.  Each cell in the grid must take a value from 0 to 9.
% No two grid cells in the same row, column, or 3x3 subgrid may take the
% same value.
%
% In the MIP formulation, binary variables x[i,j,v] indicate whether
% cell <i,j> takes value 'v'.  The constraints are as follows:
%   1. Each cell must take exactly one value (sum_v x[i,j,v] = 1)
%   2. Each value is used exactly once per row (sum_i x[i,j,v] = 1)
%   3. Each value is used exactly once per column (sum_j x[i,j,v] = 1)
%   4. Each value is used exactly once per 3x3 subgrid (sum_grid x[i,j,v] = 1)
%
% Input datasets for this example can be found in examples/data/sudoku*.
%

SUBDIM = 3;
DIM    = SUBDIM*SUBDIM;

fileID = fopen(filename);
if fileID == -1
    fprintf('Could not read file %s, quit\n', filename);
    return;
end

board = repmat(-1, DIM, DIM);
for i = 1:DIM
    s = fgets(fileID, 100);
    if length(s) <= DIM
        fprintf('Error: not enough board positions specified, quit\n');
        return;
    end
    for j = 1:DIM
        if s(j) ~= '.'
            board(i, j) = str2double(s(j));
            if board(i,j) < 1 || board(i,j) > DIM
                fprintf('Error: Unexpected character in Input line %d, quit\n', i);
                return;
            end
        end
    end
end

% Map X(i,j,k) into an index variable in the model
nVars = DIM * DIM * DIM;


% Build model
model.vtype    = repmat('B', nVars, 1);
model.lb       = zeros(nVars, 1);
model.ub       = ones(nVars, 1);

for i = 1:DIM
    for j = 1:DIM
        for v = 1:DIM
            var = (i-1)*DIM*DIM + (j-1)*DIM + v;
            model.varnames{var} = sprintf('x[%d,%d,%d]', i, j, v);
        end
    end
end

% Create constraints:
nRows = 4 * DIM * DIM;
model.A = sparse(nRows, nVars);
model.rhs = ones(nRows, 1);
model.sense = repmat('=', nRows, 1);

Row = 1;

% Each cell gets a value */
for i = 1:DIM
    for j = 1:DIM
        for v = 1:DIM
            if board(i,j) == v
                model.lb((i-1)*DIM*DIM + (j-1)*DIM + v) = 1;
            end
            model.A(Row, (i-1)*DIM*DIM + (j-1)*DIM + v) = 1;
        end
        Row = Row + 1;
    end
end

% Each value must appear once in each row
for v = 1:DIM
    for j = 1:DIM
        for i = 1:DIM
            model.A(Row, (i-1)*DIM*DIM + (j-1)*DIM + v) = 1;
        end
        Row = Row + 1;
    end
end

% Each value must appear once in each column
for v = 1:DIM
    for i = 1:DIM
        for j = 1:DIM
            model.A(Row, (i-1)*DIM*DIM + (j-1)*DIM + v) = 1;
        end
        Row = Row + 1;
    end
end

% Each value must appear once in each subgrid
for v = 1:DIM
    for ig = 0: SUBDIM-1
        for jg = 0: SUBDIM-1
            for i = ig*SUBDIM+1:(ig+1)*SUBDIM
                for j = jg*SUBDIM+1:(jg+1)*SUBDIM
                    model.A(Row, (i-1)*DIM*DIM + (j-1)*DIM + v) = 1;
                end
            end
            Row = Row + 1;
        end
    end
end

% Save model
gurobi_write(model, 'sudoku_m.lp');

% Optimize model
params.logfile = 'sudoku_m.log';
result = gurobi(model, params);

if strcmp(result.status, 'OPTIMAL')
    fprintf('Solution:\n');
    for i = 1:DIM
        for j = 1:DIM
            for v = 1:DIM
                var = (i-1)*DIM*DIM + (j-1)*DIM + v;
                if result.x(var) > 0.99
                    fprintf('%d', v);
                end
            end
        end
        fprintf('\n');
    end
else
    fprintf('Problem was infeasible\n')
end
