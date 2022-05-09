function params(filename)
% Copyright 2020, Gurobi Optimization, LLC
%
% Use parameters that are associated with a model.
%
% A MIP is solved for a few seconds with different sets of parameters.
% The one with the smallest MIP gap is selected, and the optimization
% is resumed until the optimal solution is found.

% Read model
fprintf('Reading model %s\n', filename);

model = gurobi_read(filename);

ivars = find(model.vtype ~= 'C');

if length(ivars) <= 0
    fprintf('All variables of the model are continuous, nothing to do\n');
    return;
end

% Set a 2 second time limit
params.TimeLimit = 2;

% Now solve the model with different values of MIPFocus

params.MIPFocus = 0;
result          = gurobi(model, params);
bestgap         = result.mipgap;
bestparams      = params;
for i = 1:3
    params.MIPFocus = i;
    result          = gurobi(model, params);
    if result.mipgap < bestgap
        bestparams = params;
        bestgap    = result.mipgap;
    end
end

% Finally, reset the time limit and Re-solve model to optimality
bestparams.TimeLimit = Inf;
result = gurobi(model, bestparams);
fprintf('Solution status: %s, objective value %g\n', ...
    result.status, result.objval);
end
