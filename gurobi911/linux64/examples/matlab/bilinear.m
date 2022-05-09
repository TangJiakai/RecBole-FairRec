function bilinear
% This example formulates and solves the following simple bilinear model:
%  maximize    x
%  subject to  x + y + z <= 10
%              x * y <= 2         (bilinear inequality)
%              x * z + y * z = 1  (bilinear equality)
%              x, y, z non-negative (x integral in second version)

% Copyright 2020, Gurobi Optimization, LLC

% Linear constraint matrix
m.A = sparse([1, 1, 1]);
m.sense = '<';
m.rhs = 10;

% Variable names
m.varnames = {'x', 'y', 'z'};

% Objective function max 1.0 * x
m.obj = [1; 0; 0];
m.modelsense = 'max';

% Bilinear inequality constraint: x * y <= 2
m.quadcon(1).Qrow = 1;
m.quadcon(1).Qcol = 2;
m.quadcon(1).Qval = 1.0;
m.quadcon(1).q = sparse(3,1);
m.quadcon(1).rhs = 2.0;
m.quadcon(1).sense = '<';
m.quadcon(1).name = 'bilinear0';

% Bilinear equality constraint: x * z + y * z == 1
m.quadcon(2).Qrow = [1, 2];
m.quadcon(2).Qcol = [3, 3];
m.quadcon(2).Qval = [1.0, 1.0];
m.quadcon(2).q = sparse(3,1);
m.quadcon(2).rhs = 1.0;
m.quadcon(2).sense = '=';
m.quadcon(2).name = 'bilinear1';

% Solve bilinear model, display solution.  The problem is non-convex,
% we need to set the parameter 'NonConvex' in order to solve it.
params.NonConvex = 2;
result = gurobi(m, params);
disp(result.x);

% Constrain 'x' to be integral and solve again
m.vtype = 'ICC';
result = gurobi(m, params);
disp(result.x);
end