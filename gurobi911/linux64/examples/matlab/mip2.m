function mip2(filename)

% Copyright 2020, Gurobi Optimization, LLC
%
% This example reads a MIP model from a file, solves it and prints
% the objective values from all feasible solutions generated while
% solving the MIP. Then it creates the associated fixed model and
% solves that model.

% Read model
fprintf('Reading model %s\n', filename);

model = gurobi_read(filename);

cols = size(model.A, 2);

ivars = find(model.vtype ~= 'C');
ints = length(ivars);

if ints <= 0
    fprintf('All variables of the model are continuous, nothing to do\n');
    return;
end

% Optimize
params.poolsolutions = 20;
result = gurobi(model, params);

% Capture solution information
if ~strcmp(result.status, 'OPTIMAL')
    fprintf('This model cannot be solved because its optimization status is %s\n', ...
        result.status);
    return;
end

% Iterate over the solutions
if isfield(result, 'pool') && ~isempty(result.pool)
    solcount = length(result.pool);
    for k = 1:solcount
        fprintf('Solution %d has objective %g\n', k, result.pool(k).objval);
    end
else
    fprintf('Solution 1 has objective %g\n', result.objval);
end

% Convert to fixed model
for j = 1:cols
    if model.vtype(j) ~= 'C'
        t = floor(result.x(j) + 0.5);
        model.lb(j) = t;
        model.ub(j) = t;
    end
end

% Solve the fixed model
result2 = gurobi(model, params);
if ~strcmp(result.status, 'OPTIMAL')
    fprintf('Error: fixed model is not optimal\n');
    return;
end
if abs(result.objval - result2.objval) > 1e-6 * (1 + abs(result.objval))
    fprintf('Error: Objective values differ\n');
end

% Print values of non-zero variables
for j = 1:cols
    if abs(result2.x(j)) > 1e-6
        fprintf('%s %g\n', model.varnames{j}, result2.x(j));
    end
end
