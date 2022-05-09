function genconstr()

% Copyright 2020, Gurobi Optimization, LLC
%
% In this example we show the use of general constraints for modeling
% some common expressions. We use as an example a SAT-problem where we
% want to see if it is possible to satisfy at least four (or all) clauses
% of the logical for
%
% L = (x1 or ~x2 or x3)  and (x2 or ~x3 or x4)  and
%     (x3 or ~x4 or x1)  and (x4 or ~x1 or x2)  and
%     (~x1 or ~x2 or x3) and (~x2 or ~x3 or x4) and
%     (~x3 or ~x4 or x1) and (~x4 or ~x1 or x2)
%
% We do this by introducing two variables for each literal (itself and its
% negated value), a variable for each clause, and then two
% variables for indicating if we can satisfy four, and another to identify
% the minimum of the clauses (so if it one, we can satisfy all clauses)
% and put these two variables in the objective.
% i.e. the Objective function will be
%
% maximize Obj1 + Obj2
%
%  Obj1 = MIN(Clause2, ... , Clause8)
%  Obj2 = 2 -> Clause2 + ... + Clause8 >= 4
%
% thus, the objective value will be two if and only if we can satisfy all
% clauses; one if and only if at least four clauses can be satisfied, and
% zero otherwise.
%


% define primitive data
n         = 4;
nLiterals = 4;
nClauses  = 8;
nObj      = 2;
nVars     = 2 * nLiterals + nClauses + nObj;
Clauses = [
      1, n+2, 3;   2, n+3, 4;
      3, n+4, 1;   4, n+1, 2;
    n+1, n+2, 3; n+2, n+3, 4;
    n+3, n+4, 1; n+4, n+1, 2
    ];

% Create model
model.modelname  = 'genconstr';
model.modelsense = 'max';

% Set-up data for variables and constraints
model.vtype = repmat('B', nVars, 1);
model.ub    = ones(nVars, 1);
model.obj   = [zeros(2*nLiterals + nClauses, 1); ones(nObj, 1)];
model.A     = sparse(nLiterals, nVars);
model.rhs   = ones(nLiterals, 1);
model.sense = repmat('=', nLiterals, 1);

for j = 1:nLiterals
    model.varnames{j} = sprintf('X%d', j);
    model.varnames{nLiterals+j} = sprintf('notX%d', j);
end
for j = 1:nClauses
    model.varnames{2*nLiterals+j} = sprintf('Clause%d', j);
end
for j = 1:nObj
    model.varnames{2*nLiterals+nClauses+j} = sprintf('Obj%d', j);
end

% Link Xi and notXi
for i = 1:nLiterals
    model.A(i, i) = 1;
    model.A(i, nLiterals+i) = 1;
    model.constrnames{i} = sprintf('CNSTR_X%d', i);
end

% Link clauses and literals
for i = 1:nClauses
    model.genconor(i).resvar = 2 * nLiterals + i;
    model.genconor(i).vars = Clauses(i:i,1:3);
    model.genconor(i).name = sprintf('CNSTR_Clause%d', i);
end

% Link objs with clauses
model.genconmin.resvar = 2 * nLiterals + nClauses + 1;
for i = 1:nClauses
    model.genconmin.vars(i) = 2 * nLiterals + i;
end
model.genconmin.name = 'CNSTR_Obj1';

model.genconind.binvar = 2 * nLiterals + nClauses + 2;
model.genconind.binval = 1;
model.genconind.a      = [zeros(2*nLiterals,1); ones(nClauses,1); zeros(nObj,1)];
model.genconind.sense  = '>';
model.genconind.rhs    = 4;
model.genconind.name   = 'CNSTR_Obj2';

% Save model
gurobi_write(model, 'genconstr_m.lp');

% Optimize
params.logfile = 'genconstr.log';
result = gurobi(model, params);

% Check optimization status
if strcmp(result.status, 'OPTIMAL')
    if result.objval > 1.9
        fprintf('Logical expression is satisfiable\n');
    else
        if result.objval > 0.9
            fprintf('At least four clauses are satisfiable\n');
        else
            fprintf('At most three clauses may be satisfiable\n');
        end
    end
else
    fprintf('Optimization falied\n');
end
