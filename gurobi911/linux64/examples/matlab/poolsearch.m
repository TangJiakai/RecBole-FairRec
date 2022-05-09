function poolsearch()

% Copyright 2020, Gurobi Optimization, LLC
%
% We find alternative epsilon-optimal solutions to a given knapsack
% problem by using PoolSearchMode

% define primitive data
groundSetSize = 10;
objCoef       = [32; 32; 15; 15; 6; 6; 1; 1; 1; 1];
knapsackCoef  = [16, 16,  8,  8, 4, 4, 2, 2, 1, 1];
Budget        = 33;

% Initialize model
model.modelsense  = 'max';
model.modelname   = 'poolsearch';

% Set variables
model.obj         = objCoef;
model.vtype       = repmat('B', groundSetSize, 1);
model.lb          = zeros(groundSetSize, 1);
model.ub          = ones(groundSetSize, 1);
for j = 1:groundSetSize
    model.varnames{j} = sprintf('El%d', j);
end

% Build constraint matrix
model.A           = sparse(knapsackCoef);
model.rhs         = Budget;
model.sense       = '<';
model.constrnames = {'Budget'};

% Set poolsearch parameters
params.PoolSolutions  = 1024;
params.PoolGap        = 0.10;
params.PoolSearchMode = 2;

% Save problem
gurobi_write(model, 'poolsearch_m.lp');

% Optimize
result = gurobi(model, params);

% Capture solution information
if ~strcmp(result.status, 'OPTIMAL')
    fprintf('Optimization finished with status %s, quit now\n', result.status);
    return;
end

% Print best solution
fprintf('Selected elements in best solution:\n');
for j = 1:groundSetSize
    if result.x(j) >= 0.9
        fprintf('%s ', model.varnames{j});
    end
end
fprintf('\n');

% Print all solution objectives and best furth solution
if isfield(result, 'pool') && ~isempty(result.pool)
    solcount = length(result.pool);
    fprintf('Number of solutions found: %d\n', solcount);
    fprintf('Objective values for all %d solutions:\n', solcount);
    for k = 1:solcount
        fprintf('%g ', result.pool(k).objval);
    end
    fprintf('\n');
    if solcount >= 4
        fprintf('Selected elements in fourth best solution:\n');
        for k = 1:groundSetSize
            if result.pool(4).xn(k) >= 0.9
                fprintf(' %s', model.varnames{k});
            end
        end
        fprintf('\n');
    end
else
    fprintf('Number of solutions found: 1\n');
    fprintf('Solution 1 has objective: %g \n', result.objval);
end
