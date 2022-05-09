function [x,fval,exitflag,output,lambda] = linprog(f,A,b,Aeq,beq,lb,ub,x0,options)
%Copyright 2020, Gurobi Optimization, LLC
%
%LINPROG A linear programming example using the Gurobi MATLAB interface
%
%   This example is based on the linprog interface defined in the
%   MATLAB Optimization Toolbox. The Optimization Toolbox
%   is a registered trademark of The Math Works, Inc.
%
%   x = LINPROG(f,A,b) solves the linear programming problem:
%
%     minimize     f'*x
%     subject to    A*x <= b.
%
%   For large problems, you can pass A as a sparse matrix and b as a
%   sparse vector.
%
%   x = LINPROG(f,A,b,Aeq,beq) solves the problem:
%
%     minimize     f'*x
%     subject to    A*x <= b,
%                 Aeq*x == beq.
%
%   For large problems, you can pass Aeq as a sparse matrix and beq as a
%   sparse vector. You can set A=[] and b=[] if no inequalities exist.
%
%   x = LINPROG(f,A,b,Aeq,beq,lb,ub) solves the problem:
%
%     minimize     f'*x
%     subject to    A*x <= b,
%                 Aeq*x == beq,
%           lb <=     x <= ub.
%
%   You can set lb(j) = -inf, if x(j) has no lower bound, and ub(j) = inf,
%   if x(j) has no upper bound. You can set Aeq=[] and beq=[] if no
%   equalities exist.
%
%   x = LINPROG(f,A,b,Aeq,beq,lb,ub,OPTIONS) solves the problem above
%   given the specified OPTIONS. Only a subset of possible options have
%   any effect:
%
%     OPTIONS.Display  'off' or 'none' disables output,
%     OPTIONS.MaxTime  time limit in seconds.
%
%   You can set lb=[] or ub=[] if no bounds exist.
%
%   x = LINPROG(PROBLEM) solves PROBLEM, which is a structure that must
%   have solver name 'linprog' in PROBLEM.solver. You can also specify
%   any of the input arguments above using fields PROBLEM.f, PROBLEM.A, ...
%
%   [x,fval] = LINPROG(f,A,b) returns the objective value at the solution.
%   That is, fval = f'*x.
%
%   [x,fval,exitflag] = LINPROG(f,A,b) returns an exitflag containing the
%   status of the optimization. The values for exitflag and the
%   corresponding status codes are:
%
%      1  converged to a solution (OPTIMAL),
%      0  maximum number of iterations reached (ITERATION_LIMIT),
%     -2  no feasible point found (INFEASIBLE, NUMERIC, ...),
%     -3  problem is unbounded (UNBOUNDED).
%
%   [x,fval,exitflag,OUTPUT] = LINPROG(f,A,b) returns information about
%   the optimization. OUTPUT is a structure with the following fields:
%
%     OUTPUT.message          Gurobi status code
%     OUTPUT.constrviolation  maximum violation for constraints and bounds
%
%   [x,fval,exitflag,OUTPUT,LAMBDA] = LINPROG(f,A,b) returns the
%   Lagrangian multipliers at the solution. LAMBDA is a structure with
%   the following fields:
%
%     LAMBDA.lower    multipliers corresponding to x >= lb
%     LAMBDA.upper    multipliers corresponding to x <= ub
%     LAMBDA.ineqlin  multipliers corresponding to A*x <= b
%     LAMBDA.eqlin    multipliers corresponding to Aeq*x == beq
%

% Initialize missing arguments
if nargin == 1
    if isa(f,'struct') && isfield(f,'solver') && strcmpi(f.solver,'linprog')
        [f,A,b,Aeq,beq,lb,ub,x0,options] = probstruct2args(f);
    else
        error('PROBLEM should be a structure with valid fields');
    end
elseif nargin < 3 || nargin > 9
    error('LINPROG: the number of input arguments is wrong');
elseif nargin < 9
    options = struct();
    if nargin == 8
        if isa(x0,'struct') || isa(x0,'optim.options.SolverOptions')
            options = x0; % x0 was omitted and options were passed instead
            x0 = [];
        end
    else
        x0 = [];
        if nargin < 7
            ub = [];
            if nargin < 6
                lb = [];
                if nargin < 5
                    beq = [];
                    if nargin < 4
                        Aeq = [];
                    end
                end
            end
        end
    end
end

% Warn user if x0 argument is ignored
if ~isempty(x0)
    warning('LINPROG will ignore non-empty starting point X0');
end

% Build Gurobi model
model.obj = f;
model.A = [sparse(A); sparse(Aeq)]; % A must be sparse
model.sense = [repmat('<',size(A,1),1); repmat('=',size(Aeq,1),1)];
model.rhs = full([b(:); beq(:)]); % rhs must be dense
if ~isempty(lb)
    model.lb = lb;
else
    model.lb = -inf(size(model.A,2),1); % default lb for MATLAB is -inf
end
if ~isempty(ub)
    model.ub = ub;
end

% Extract relevant Gurobi parameters from (subset of) options
params = struct();

if isfield(options,'Display') || isa(options,'optim.options.SolverOptions')
    if any(strcmp(options.Display,{'off','none'}))
        params.OutputFlag = 0;
    end
end

if isfield(options,'MaxTime') || isa(options,'optim.options.SolverOptions')
    params.TimeLimit = options.MaxTime;
end

% Solve model with Gurobi
result = gurobi(model,params);

% Resolve model if status is INF_OR_UNBD
if strcmp(result.status,'INF_OR_UNBD')
    params.DualReductions = 0;
    warning('Infeasible or unbounded, resolve without dual reductions to determine...');
    result = gurobi(model,params);
end

% Collect results
x = [];
output.message = result.status;
output.constrviolation = [];

if isfield(result,'x')
    x = result.x;
    if nargout > 3
        slack = model.A*x-model.rhs;
        violA = slack(1:size(A,1));
        violAeq = norm(slack((size(A,1)+1):end),inf);
        viollb = model.lb(:)-x;
        violub = 0;
        if isfield(model,'ub')
            violub = x-model.ub(:);
        end
        output.constrviolation = max([0; violA; violAeq; viollb; violub]);
    end
end

fval = [];

if isfield(result,'objval')
    fval = result.objval;
end

if strcmp(result.status,'OPTIMAL')
    exitflag = 1; % converged to a solution
elseif strcmp(result.status,'UNBOUNDED')
    exitflag = -3; % problem is unbounded
elseif strcmp(result.status,'ITERATION_LIMIT')
    exitflag = 0; % maximum number of iterations reached
else
    exitflag = -2; % no feasible point found
end

lambda.lower = [];
lambda.upper = [];
lambda.ineqlin = [];
lambda.eqlin = [];

if nargout > 4
    if isfield(result,'rc')
        lambda.lower = max(0,result.rc);
        lambda.upper = -min(0,result.rc);
    end
    if isfield(result,'pi')
        if ~isempty(A)
            lambda.ineqlin = -result.pi(1:size(A,1));
        end
        if ~isempty(Aeq)
            lambda.eqlin = -result.pi((size(A,1)+1):end);
        end
    end
end

if isempty(lambda.lower) && isempty(lambda.upper) && ...
        isempty(lambda.ineqlin) && isempty(lambda.eqlin)
    lambda = [];
end

% Local Functions =========================================================

function [f,A,b,Aeq,beq,lb,ub,x0,options] = probstruct2args(s)
%PROBSTRUCT2ARGS Get problem structure fields ([] is returned when missing)

f = getstructfield(s,'f');
A = getstructfield(s,'Aineq');
b = getstructfield(s,'bineq');
Aeq = getstructfield(s,'Aeq');
beq = getstructfield(s,'beq');
lb = getstructfield(s,'lb');
ub = getstructfield(s,'ub');
x0 = getstructfield(s,'x0');
options = getstructfield(s,'options');

function f = getstructfield(s,field)
%GETSTRUCTFIELD Get structure field ([] is returned when missing)

if isfield(s,field)
    f = getfield(s,field);
else
    f = [];
end
