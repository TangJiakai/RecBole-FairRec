function multiobj()

% Copyright 2020, Gurobi Optimization, LLC
%
% Want to cover three different sets but subject to a common budget of
% elements allowed to be used. However, the sets have different priorities to
% be covered; and we tackle this by using multi-objective optimization.

% define primitive data
groundSetSize     = 20;
nSubSets          = 4;
Budget            = 12;
Set               = [
    1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0;
    0 0 0 0 0 1 1 1 1 1 0 0 0 0 0 1 1 1 1 1;
    0 0 0 1 1 0 1 1 0 0 0 0 0 1 1 0 1 1 0 0;
    0 0 0 1 1 1 0 0 0 1 1 1 0 0 0 1 1 1 0 0
    ];
SetObjPriority    = [3; 2; 2; 1];
SetObjWeight      = [1.0; 0.25; 1.25; 1.0];

% Initialize model
model.modelsense  = 'max';
model.modelname   = 'multiobj';

% Set variables and constraints
model.vtype       = repmat('B', groundSetSize, 1);
model.lb          = zeros(groundSetSize, 1);
model.ub          = ones(groundSetSize, 1);
model.A           = sparse(1, groundSetSize);
model.rhs         = Budget;
model.sense       = '<';
model.constrnames = {'Budget'};

for j = 1:groundSetSize
    model.varnames{j} = sprintf('El%d', j);
    model.A(1, j)     = 1;
end

% Set multi-objectives
for m = 1:nSubSets
    model.multiobj(m).objn     = Set(m, :);
    model.multiobj(m).priority = SetObjPriority(m);
    model.multiobj(m).weight   = SetObjWeight(m);
    model.multiobj(m).abstol   = m;
    model.multiobj(m).reltol   = 0.01;
    model.multiobj(m).name     = sprintf('Set%d', m);
    model.multiobj(m).con      = 0.0;
end

% Save model
gurobi_write(model,'multiobj_m.lp')

% Set parameters
params.PoolSolutions = 100;

% Optimize
result = gurobi(model, params);

% Capture solution information
if ~strcmp(result.status, 'OPTIMAL')
    fprintf('Optimization finished with status %d, quit now\n', result.status);
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
    fprintf('Objective values for first %d solutions:\n', solcount);
    for m = 1:nSubSets
        fprintf('  %s:', model.multiobj(m).name);
        for k = 1:solcount
            fprintf('  %3g', result.pool(k).objval(m));
        end
        fprintf('\n');
    end
    fprintf('\n');
else
    fprintf('Number of solutions found: 1\n');
    fprintf('Solution 1 has objective values:');
    for k = 1:nSubSets
        fprintf('  %g', result.objval(k));
    end
    fprintf('\n');
end
