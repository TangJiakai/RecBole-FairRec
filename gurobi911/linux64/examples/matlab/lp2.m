function lp2()
% Copyright 2020, Gurobi Optimization, LLC
%
% Formulate a simple linear program, solve it, and then solve it
% again using the optimal basis.

model.A = sparse([1 3 4; 8 2 3]);
model.obj = [1 2 3];
model.rhs = [4 7];
model.sense = ['>' '>'];

% First solve requires a few simplex iterations
result = gurobi(model)

% Second solve - start from an optimal basis, so no iterations
model.vbasis = result.vbasis;
model.cbasis = result.cbasis;
result = gurobi(model)
end