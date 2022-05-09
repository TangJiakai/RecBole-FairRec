function opttoolbox_lp()
% Copyright 2020, Gurobi Optimization, LLC
%
% This example uses Matlab 2017b problem based modeling feature, which
% requires Optimization Toolbox, to formulate and solve the following
% simple LP model, the same model as for lp.m
%
% maximize
%       x + 2 y + 3 z
% subject to
%       x +   y        <= 1
%             y +   z  <= 1
%
% To use Gurobi with this example, linprog.m must be in the current
% directory or added to Matlab path

x = optimvar('x', 'LowerBound',0);
y = optimvar('y', 'LowerBound',0);
z = optimvar('z', 'LowerBound',0);

prob = optimproblem('ObjectiveSense','maximize');

prob.Objective = x + 2 * y + 3 * z;

prob.Constraints.cons1 = x + y <= 1;
prob.Constraints.cons2 = y + z <= 1;

options = optimoptions('linprog');

% For Matlab R2017b use the following
% sol = solve(prob, options)

% Syntax for R2018a and later
sol = solve(prob, 'Options', options);
end
