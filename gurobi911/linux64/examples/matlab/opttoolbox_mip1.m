function opttoolbox_mip1()
% Copyright 2020, Gurobi Optimization, LLC
%
% This example uses Matlab 2017b problem based modeling feature, which
% requires Optimization Toolbox, to formulate and solve the following
% simple MIP model, the same model as for mip1.m
%
%  maximize
%        x +   y + 2 z
%  subject to
%        x + 2 y + 3 z <= 4
%        x +   y       >= 1
%  x, y, z binary
%
% To use Gurobi with this example, intlinprog.m must be in the current
% directory or added to Matlab path

x = optimvar('x', 'Type','integer','LowerBound',0,'UpperBound',1);
y = optimvar('y', 'Type','integer','LowerBound',0,'UpperBound',1);
z = optimvar('z', 'Type','integer','LowerBound',0,'UpperBound',1);

prob = optimproblem('ObjectiveSense','maximize');

prob.Objective = x + y + 2 * z;

prob.Constraints.cons1 = x + 2 * y + 3 * z <= 4;
prob.Constraints.cons2 = x + y >= 1;

options = optimoptions('intlinprog');

% For Matlab R2017b use the following
% sol = solve(prob, options)

% Syntax for R2018a and later
sol = solve(prob, 'Options', options);

end
