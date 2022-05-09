function sensitivity(filename)
% Copyright 2020, Gurobi Optimization, LLC
%
% A simple sensitivity analysis example which reads a MIP model
% from a file and solves it. Then each binary variable is set
% to 1-X, where X is its value in the optimal solution, and
% the impact on the objective function value is reported.

% Read model
fprintf('Reading model %s\n', filename);

model = gurobi_read(filename);
cols = size(model.A, 2);

ivars = find(model.vtype ~= 'C');
if length(ivars) <= 0
    fprintf('All variables of the model are continuous, nothing to do\n');
    return;
end

% Optimize
result = gurobi(model);

% Capture solution information
if result.status ~= 'OPTIMAL'
    fprintf('Model status is %d, quit now\n', result.status);
end

origx = result.x;
origobjval = result.objval;

params.OutputFlag = 0;

% Iterate through unfixed binary variables in the model
for j = 1:cols
    if model.vtype(j) ~= 'B' && model.vtype(j) ~= 'I'
        continue;
    end
    if model.vtype(j) == 'I'
        if model.lb(j) ~= 0.0 || model.ub(j) ~= 1.0
            continue;
        end
    else
        if model.lb(j) > 0.0 || model.ub(j) < 1.0
            continue;
        end
    end
    
    % Update MIP start for all variables
    model.start = origx;
    
    % Set variable to 1-X, where X is its value in optimal solution
    if origx(j) < 0.5
        model.start(j) = 1;
        model.lb(j) = 1;
    else
        model.start(j) = 0;
        model.ub(j) = 0;
    end
    
    % Optimize
    result = gurobi(model, params);
    
    % Display result
    if ~strcmp(result.status, 'OPTIMAL')
        gap = inf;
    else
        gap = result.objval - origobjval;
    end
    fprintf('Objective sensitivity for variable %s is %g\n', ...
        model.varnames{j}, gap);
    
    % Restore original bounds
    model.lb(j) = 0;
    model.ub(j) = 1;
end
