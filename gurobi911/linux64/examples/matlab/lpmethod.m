function lpmethod(filename)
% Copyright 2020, Gurobi Optimization, LLC
%
% Solve a model with different values of the Method parameter;
% show which value gives the shortest solve time.

% Read model
fprintf('Reading model %s\n', filename);
model = gurobi_read(filename);

bestTime = inf;
bestMethod = -1;

for i = 0:4
    params.method = i;
    res = gurobi(model, params);
    if strcmp(res.status, 'OPTIMAL')
        bestMethod = i;
        bestTime = res.runtime;
        params.TimeLimit = bestTime;
    end
end

% Report which method was fastest
if bestMethod == -1
    fprintf('Unable to solve this model\n');
else
    fprintf('Solved in %g seconds with Method %d\n', bestTime, bestMethod);
end
