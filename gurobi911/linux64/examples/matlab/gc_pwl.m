function gc_pwl
% Copyright 2020, Gurobi Optimization, LLC
%
% This example formulates and solves the following simple model
% with PWL constraints:
%
%  maximize
%        sum c(j) * x(j)
%  subject to
%        sum A(i,j) * x(j) <= 0,  for i = 1, ..., m
%        sum y(j) <= 3
%        y(j) = pwl(x(j)),        for j = 1, ..., n
%        x(j) free, y(j) >= 0,    for j = 1, ..., n
%
%  where pwl(x) = 0,     if x  = 0
%               = 1+|x|, if x != 0
%
%  Note
%   1. sum pwl(x(j)) <= b is to bound x vector and also to favor sparse x vector.
%      Here b = 3 means that at most two x(j) can be nonzero and if two, then
%      sum x(j) <= 1
%   2. pwl(x) jumps from 1 to 0 and from 0 to 1, if x moves from negative 0 to 0,
%      then to positive 0, so we need three points at x = 0. x has infinite bounds
%      on both sides, the piece defined with two points (-1, 2) and (0, 1) can
%      extend x to -infinite. Overall we can use five points (-1, 2), (0, 1),
%      (0, 0), (0, 1) and (1, 2) to define y = pwl(x)

n = 5;

% A x <= 0
A1 = [
    0, 0, 0, 1, -1;
    0, 0, 1, 1, -1;
    1, 1, 0, 0, -1;
    1, 0, 1, 0, -1;
    1, 0, 0, 1, -1;
    ];

% sum y(j) <= 3
A2 = [1, 1, 1, 1, 1];

% Constraint matrix altogether
model.A = sparse(blkdiag(A1, A2));

% Right-hand-side coefficient vector
model.rhs = [zeros(n,1); 3];

% Objective function (x coefficients arbitrarily chosen)
model.obj = [0.5, 0.8, 0.5, 0.1, -1, zeros(1, n)];

% It's a maximization model
model.modelsense = 'max';

% Lower bounds for x and y
model.lb = [-inf*ones(n,1); zeros(n,1)];

% PWL constraints
for k = 1:n
    model.genconpwl(k).xvar = k;
    model.genconpwl(k).yvar = n + k;
    model.genconpwl(k).xpts = [-1, 0, 0, 0, 1];
    model.genconpwl(k).ypts = [2, 1, 0, 1, 2];
end

result = gurobi(model);

for k = 1:n
    fprintf('x(%d) = %g\n', k, result.x(k));
end

fprintf('Objective value: %g\n', result.objval);
end