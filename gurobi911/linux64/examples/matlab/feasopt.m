function feasopt(filename)
%
% Copyright 2020, Gurobi Optimization, LLC
%
% This example reads a MIP model from a file, adds artificial
% variables to each constraint, and then minimizes the sum of the
% artificial variables.  A solution with objective zero corresponds
% to a feasible solution to the input model.
% We can also use FeasRelax feature to do it. In this example, we
% use minrelax=1, i.e. optimizing the returned model finds a solution
% that minimizes the original objective, but only from among those
% solutions that minimize the sum of the artificial variables.

% Read model
fprintf('Reading model %s\n', filename);
model = gurobi_read(filename);

params.logfile = 'feasopt.log';
result1 = gurobi(model, params);

[rows, cols] = size(model.A);

% Create penalties, only linear constraints are allowed to be relaxed
penalties.rhs = ones(rows, 1);

result = gurobi_feasrelax(model, 0, true, penalties, params);
gurobi_write(result.model, 'feasopt1.lp');

% clear objective
model.obj = zeros(cols, 1);

nvar = cols;
for c = 1:rows
    if model.sense(c) ~= '>'
        nvar = nvar + 1;
        model.A(c, nvar) = -1;
        model.obj(nvar) = 1;
        model.vtype(nvar) = 'C';
        model.varnames(nvar) = strcat('ArtN_', model.constrnames(c));
        model.lb(nvar) = 0;
        model.ub(nvar) = inf;
    end
    if model.sense(c) ~= '<'
        nvar = nvar + 1;
        model.A(c, nvar) = 1;
        model.obj(nvar) = 1;
        model.vtype(nvar) = 'C';
        model.varnames(nvar) = strcat('ArtP_', model.constrnames(c));
        model.lb(nvar) = 0;
        model.ub(nvar) = inf;
    end
end

gurobi_write(model, 'feasopt2.lp');
result2 = gurobi(model, params);

end
