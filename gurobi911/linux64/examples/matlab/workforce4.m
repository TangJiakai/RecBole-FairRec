function workforce4()

% Copyright 2020, Gurobi Optimization, LLC
%
% Assign workers to shifts; each worker may or may not be available on a
% particular day.  We use Pareto optimization to solve the model:
% first, we minimize the linear sum of the slacks. Then, we constrain
% the sum of the slacks, and we minimize a quadratic objective that
% tries to balance the workload among the workers.

% define data
nShifts  = 14;
nWorkers =  7;
nVars    = (nShifts + 1) * (nWorkers + 1) + nWorkers + 1;
avgShiftIdx = (nShifts+1)*(nWorkers+1);
totalSlackIdx = nVars;

Shifts  = {'Mon1'; 'Tue2'; 'Wed3'; 'Thu4'; 'Fri5'; 'Sat6'; 'Sun7';
    'Mon8'; 'Tue9'; 'Wed10'; 'Thu11'; 'Fri12'; 'Sat13'; 'Sun14'};
Workers = {'Amy'; 'Bob'; 'Cathy'; 'Dan'; 'Ed'; 'Fred'; 'Gu'};

shiftRequirements = [3; 2; 4; 4; 5; 6; 5; 2; 2; 3; 4; 6; 7; 5];

availability = [
    0 1 1 0 1 0 1 0 1 1 1 1 1 1;
    1 1 0 0 1 1 0 1 0 0 1 0 1 0;
    0 0 1 1 1 0 1 1 1 1 1 1 1 1;
    0 1 1 0 1 1 0 1 1 1 1 1 1 1;
    1 1 1 1 1 0 1 1 1 0 1 0 1 1;
    1 1 1 0 0 1 0 1 1 0 0 1 1 1;
    1 1 1 0 1 1 1 1 1 1 1 1 1 1
    ];

% Build model
model.modelname  = 'workforce4';
model.modelsense = 'min';

% Initialize assignment decision variables:
%    x[w][s] == 1 if worker w is assigned
%    to shift s. Since an assignment model always produces integer
%    solutions, we use continuous variables and solve as an LP.
model.vtype = repmat('C', nVars, 1);
model.lb    = zeros(nVars, 1);
model.ub    = ones(nVars, 1);
model.obj   = zeros(nVars, 1);

for w = 1:nWorkers
    for s = 1:nShifts
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
    model.varnames{w + avgShiftIdx}            = sprintf('DiffShifts_%s', Workers{w});
    model.ub(w + avgShiftIdx)                  = inf;
    model.lb(w + avgShiftIdx)                  = -inf;
end

% Initialize average shift variable
model.ub((nShifts+1)*(nWorkers+1))       = inf;
model.varnames{(nShifts+1)*(nWorkers+1)} = 'AvgShift';

% Initialize total slack variable
model.ub(totalSlackIdx)       = inf;
model.varnames{totalSlackIdx} = 'TotalSlack';
model.obj(totalSlackIdx)      = 1;

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

% Save model
gurobi_write(model,'workforce4a_m.lp');

% Optimize
params.logfile = 'workforce4_m.log';
result = solveandprint(model, params, Shifts, Workers);
if ~strcmp(result.status, 'OPTIMAL')
    fprintf('Quit now\n');
    return;
end

% Constraint the slack by setting its upper and lower bounds
totalSlack = result.x(totalSlackIdx);
model.lb(totalSlackIdx) = totalSlack;
model.ub(totalSlackIdx) = totalSlack;

Rows = nShifts+1+nWorkers;
for w = 1:nWorkers
    model.A(Rows+w, w + nShifts * (nWorkers+1)) = 1;
    model.A(Rows+w, w + avgShiftIdx) = -1;
    model.A(Rows+w, avgShiftIdx) = -1;
    model.A(Rows+1+nWorkers, w + nShifts * (nWorkers+1)) = 1;
    model.rhs(Rows+w) = 0;
    model.sense(Rows+w) = '=';
    model.constrnames{Rows+w} = sprintf('DiffShifts_%s_AvgShift', Workers{w});
end
model.A(Rows+1+nWorkers, avgShiftIdx) = -nWorkers;
model.rhs(Rows+1+nWorkers) = 0;
model.sense(Rows+1+nWorkers) = '=';
model.constrnames{Rows+1+nWorkers} = 'AvgShift';

% Objective: minimize the sum of the square of the difference from the
% average number of shifts worked
model.obj = zeros(nVars, 1);
model.Q   = sparse(nVars, nVars);
for w = 1:nWorkers
    model.Q(avgShiftIdx + w, avgShiftIdx + w) = 1;
end

% model is no longer an assignment problem, enforce binary constraints
% on shift decision variables.
model.vtype(1:(nWorkers * nShifts), 1) = 'B';
model.vtype((nWorkers * nShifts + 1):nVars, 1) = 'C';

% Save modified model
gurobi_write(model,'workforce4b_m.lp');

% Optimize
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
