function workforce5()

% Copyright 2020, Gurobi Optimization, LLC
%
% Assign workers to shifts; each worker may or may not be available on a
% particular day. We use multi-objective optimization to solve the model.
% The highest-priority objective minimizes the sum of the slacks
% (i.e., the total number of uncovered shifts). The secondary objective
% minimizes the difference between the maximum and minimum number of
% shifts worked among all workers.  The second optimization is allowed
% to degrade the first objective by up to the smaller value of 10% and 2

% define data
nShifts  = 14;
nWorkers =  8;
nVars    = (nShifts + 1) * (nWorkers + 1) + 2;
minShiftIdx = (nShifts+1)*(nWorkers+1);
maxShiftIdx = minShiftIdx+1;
totalSlackIdx = nVars;

Shifts  = {'Mon1'; 'Tue2'; 'Wed3'; 'Thu4'; 'Fri5'; 'Sat6'; 'Sun7';
    'Mon8'; 'Tue9'; 'Wed10'; 'Thu11'; 'Fri12'; 'Sat13'; 'Sun14'};
Workers = {'Amy'; 'Bob'; 'Cathy'; 'Dan'; 'Ed'; 'Fred'; 'Gu'; 'Tobi'};

shiftRequirements = [3; 2; 4; 4; 5; 6; 5; 2; 2; 3; 4; 6; 7; 5];

availability = [
    0 1 1 0 1 0 1 0 1 1 1 1 1 1;
    1 1 0 0 1 1 0 1 0 0 1 0 1 0;
    0 0 1 1 1 0 1 1 1 1 1 1 1 1;
    0 1 1 0 1 1 0 1 1 1 1 1 1 1;
    1 1 1 1 1 0 1 1 1 0 1 0 1 1;
    1 1 1 0 0 1 0 1 1 0 0 1 1 1;
    0 1 1 1 0 1 1 0 1 1 1 0 1 1;
    1 1 1 0 1 1 1 1 1 1 1 1 1 1
    ];

% Build model
model.modelname  = 'workforce5';
model.modelsense = 'min';

% Initialize assignment decision variables:
%    x[w][s] == 1 if worker w is assigned
%    to shift s. Since an assignment model always produces integer
%    solutions, we use continuous variables and solve as an LP.
model.vtype = repmat('C', nVars, 1);
model.lb    = zeros(nVars, 1);
model.ub    = ones(nVars, 1);

for w = 1:nWorkers
    for s = 1:nShifts
        model.vtype(s+(w-1)*nShifts) = 'B';
        model.varnames{s+(w-1)*nShifts} = sprintf('%s.%s', Workers{w}, Shifts{s});
        if availability(w, s) == 0
            model.ub(s+(w-1)*nShifts) = 0;
        end
    end
end

% Initialize shift slack variables
for s = 1:nShifts
    model.varnames{s+nShifts*nWorkers} = sprintf('ShiftSlack_%s', Shifts{s});
    model.ub(s+nShifts*nWorkers) = inf;
end

% Initialize worker slack and diff variables
for w = 1:nWorkers
    model.varnames{w + nShifts * (nWorkers+1)} = sprintf('TotalShifts_%s', Workers{w});
    model.ub(w + nShifts * (nWorkers+1))       = inf;
end

% Initialize min/max shift variables
model.ub(minShiftIdx)       = inf;
model.varnames{minShiftIdx} = 'MinShift';
model.ub(maxShiftIdx)       = inf;
model.varnames{maxShiftIdx} = 'MaxShift';

% Initialize total slack variable
model.ub(totalSlackIdx)       = inf;
model.varnames{totalSlackIdx} = 'TotalSlack';

% Set-up shift-requirements constraints with shift slack
model.sense = repmat('=', nShifts+1+nWorkers, 1);
model.rhs   = [shiftRequirements; zeros(1+nWorkers, 1)];
model.constrnames = Shifts;
model.A = sparse(nShifts+1+nWorkers, nVars);
for s = 1:nShifts
    for w = 1:nWorkers
        model.A(s, s+(w-1)*nShifts) = 1;
    end
    model.A(s, s + nShifts*nWorkers) = 1;
end

% Set TotalSlack equal to the sum of each shift slack
for s = 1:nShifts
    model.A(nShifts+1, s+nShifts*nWorkers) = -1;
end
model.A(nShifts+1, totalSlackIdx) = 1;
model.constrnames{nShifts+1} = 'TotalSlack';

% Set total number of shifts for each worker
for w = 1:nWorkers
    for s = 1:nShifts
        model.A(w + nShifts+1, s+(w-1)*nShifts) = -1;
    end
    model.A(w + nShifts+1, w + nShifts * (nWorkers+1)) = 1;
    model.constrnames{nShifts+1+w} = sprintf('totShifts_%s', Workers{w});
end

% Set minShift / maxShift general constraints
model.genconmin.resvar = minShiftIdx;
model.genconmin.name   = 'MinShift';
model.genconmax.resvar = maxShiftIdx;
model.genconmax.name   = 'MaxShift';
for w = 1:nWorkers
    model.genconmin.vars(w) = w + nShifts * (nWorkers+1);
    model.genconmax.vars(w) = w + nShifts * (nWorkers+1);
end

% Set multiobjective
model.multiobj(1).objn                = zeros(nVars, 1);
model.multiobj(1).objn(totalSlackIdx) = 1;
model.multiobj(1).priority            = 2;
model.multiobj(1).weight              = 1;
model.multiobj(1).abstol              = 2;
model.multiobj(1).reltol              = 0.1;
model.multiobj(1).name                = 'TotalSlack';
model.multiobj(1).con                 = 0.0;
model.multiobj(2).objn                = zeros(nVars, 1);
model.multiobj(2).objn(minShiftIdx)   = -1;
model.multiobj(2).objn(maxShiftIdx)   = 1;
model.multiobj(2).priority            = 1;
model.multiobj(2).weight              = 1;
model.multiobj(2).abstol              = 0;
model.multiobj(2).reltol              = 0;
model.multiobj(2).name                = 'Fairness';
model.multiobj(2).con                 = 0.0;

% Save initial model
gurobi_write(model,'workforce5_m.lp');

% Optimize
params.logfile = 'workforce5_m.log';
result = solveandprint(model, params, Shifts, Workers);
if ~strcmp(result.status, 'OPTIMAL')
    fprintf('Not optimal\n');
end

end

function result = solveandprint(model, params, Shifts, Workers)
% Helper function to solve and display results

nShifts = length(Shifts);
nWorkers = length(Workers);
result = gurobi(model, params);
if strcmp(result.status, 'OPTIMAL')
    fprintf('The optimal objective is %g\n', result.objval);
    fprintf('Schedule:\n');
    for s = 1:nShifts
        fprintf('\t%s:', Shifts{s});
        for w = 1:nWorkers
            if result.x(s+(w-1)*nShifts) > 0.9
                fprintf('%s ', Workers{w});
            end
        end
        fprintf('\n');
    end
    fprintf('Workload:\n');
    for w = 1:nWorkers
        fprintf('\t%s: %g\n', Workers{w}, result.x(w + nShifts * (nWorkers+1)));
    end
else
    fprintf('Optimization finished with status %s\n', result.status);
end
end
