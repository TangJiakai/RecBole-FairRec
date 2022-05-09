function gc_pwl_func

% Copyright 2020, Gurobi Optimization, LLC
%
% This example considers the following nonconvex nonlinear problem
%
%  maximize    2 x    + y
%  subject to  exp(x) + 4 sqrt(y) <= 9
%              x, y >= 0
%
%  We show you two approaches to solve this:
%
%  1) Use a piecewise-linear approach to handle general function
%     constraints (such as exp and sqrt).
%     a) Add two variables
%        u = exp(x)
%        v = sqrt(y)
%     b) Compute points (x, u) of u = exp(x) for some step length (e.g., x
%        = 0, 1e-3, 2e-3, ..., xmax) and points (y, v) of v = sqrt(y) for
%        some step length (e.g., y = 0, 1e-3, 2e-3, ..., ymax). We need to
%        compute xmax and ymax (which is easy for this example, but this
%        does not hold in general).
%     c) Use the points to add two general constraints of type
%        piecewise-linear.
%
%  2) Use the Gurobis built-in general function constraints directly (EXP
%     and POW). Here, we do not need to compute the points and the maximal
%     possible values, which will be done internally by Gurobi.  In this
%     approach, we show how to "zoom in" on the optimal solution and
%     tighten tolerances to improve the solution quality.
%


% Four nonneg. variables x, y, u, v, one linear constraint u + 4*v <= 9
m.varnames = {'x', 'y', 'u', 'v'};
m.lb = zeros(4, 1);
m.ub = +inf(4, 1);
m.A = sparse([0, 0, 1, 4]);
m.rhs = 9;

% Objective
m.modelsense = 'max';
m.obj = [2; 1; 0; 0];

% First approach: PWL constraints

% Approximate u \approx exp(x), equispaced points in [0, xmax], xmax = log(9)
m.genconpwl(1).xvar = 1;
m.genconpwl(1).yvar = 3;
m.genconpwl(1).xpts = 0:1e-3:log(9);
m.genconpwl(1).ypts = exp(m.genconpwl(1).xpts);

% Approximate v \approx sqrt(y), equispaced points in [0, ymax], ymax = (9/4)^2
m.genconpwl(2).xvar = 2;
m.genconpwl(2).yvar = 4;
m.genconpwl(2).xpts = 0:1e-3:(9/4)^2;
m.genconpwl(2).ypts = sqrt(m.genconpwl(2).xpts);

% Solve and print solution
result = gurobi(m);
printsol(result.objval, result.x(1), result.x(2), result.x(3), result.x(4));

% Second approach: General function constraint approach with auto PWL
% translation by Gurobi

% Delete explicit PWL approximations from model
m = rmfield(m, 'genconpwl');

% Set u \approx exp(x)
m.genconexp.xvar = 1;
m.genconexp.yvar = 3;
m.genconexp.name = 'gcf1';

% Set v \approx sqrt(y) = y^0.5
m.genconpow.xvar = 2;
m.genconpow.yvar = 4;
m.genconpow.a = 0.5;
m.genconpow.name = 'gcf2';

% Parameters for discretization: use equal piece length with length = 1e-3
params.FuncPieces = 1;
params.FuncPieceLength = 1e-3;

% Solve and print solution
result = gurobi(m, params);
printsol(result.objval, result.x(1), result.x(2), result.x(3), result.x(4));

% Zoom in, use optimal solution to reduce the ranges and use a smaller
% pclen=1-5 to resolve
m.lb(1) = max(m.lb(1), result.x(1) - 0.01);
m.ub(1) = min(m.ub(1), result.x(1) + 0.01);
m.lb(2) = max(m.lb(2), result.x(2) - 0.01);
m.ub(2) = min(m.ub(2), result.x(2) + 0.01);
params.FuncPieceLength = 1e-5;

% Solve and print solution
result = gurobi(m, params);
printsol(result.objval, result.x(1), result.x(2), result.x(3), result.x(4));
end

function printsol(objval, x, y, u, v)
    fprintf('x = %g, u = %g\n', x, u);
    fprintf('y = %g, v = %g\n', y, v);
    fprintf('Obj = %g\n', objval);

    % Calculate violation of exp(x) + 4 sqrt(y) <= 9
    vio = exp(x) + 4 * sqrt(y) - 9;
    if vio < 0
        vio = 0;
    end
    fprintf('Vio = %g\n', vio);
end
