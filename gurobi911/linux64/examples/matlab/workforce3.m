function workforce3()

% Copyright 2020, Gurobi Optimization, LLC
%
% Assign workers to shifts; each worker may or may not be available on a
% particular day. If the problem cannot be solved, relax the model
% to determine which constraints cannot be satisfied, and how much
% they need to be relaxed.

% define data
nShifts  = 14;
nWorkers =  7;
nVars    = nShifts * nWorkers;

Shifts  = {'Mon1'; 'Tue2'; 'Wed3'; 'Thu4'; 'Fri5'; 'Sat6'; 'Sun7';
    'Mon8'; 'Tue9'; 'Wed10'; 'Thu11'; 'Fri12'; 'Sat13'; 'Sun14'};
Workers = {'Amy'; 'Bob'; 'Cathy'; 'Dan'; 'Ed'; 'Fred'; 'Gu'};

pay     = [10; 12; 10; 8; 8; 9; 11];

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
model.modelname  = 'workforce3';
model.modelsense = 'min';

% Initialize assignment decision variables:
%    x[w][s] == 1 if worker w is assigned
%    to shift s. Since an assignment model always produces integer
%    solutions, we use continuous variables and solve as an LP.
model.ub    = ones(nVars, 1);
model.obj   = zeros(nVars, 1);

for w = 1:nWorkers
    for s = 1:nShifts
        model.varnames{s+(w-1)*nShifts} = sprintf('%s.%s', Workers{w}, Shifts{s});
        model.obj(s+(w-1)*nShifts) = pay(w);
        if availability(w, s) == 0
            model.ub(s+(w-1)*nShifts) = 0;
        end
    end
end

% Set-up shift-requirements constraints
model.sense = repmat('=', nShifts, 1);
model.rhs   = shiftRequirements;
model.constrnames = Shifts;
model.A = sparse(nShifts, nVars);
for s = 1:nShifts
    for w = 1:nWorkers
        model.A(s, s+(w-1)*nShifts) = 1;
    end
end

% Save model
gurobi_write(model,'workforce3_m.lp');

% Optimize
params.logfile = 'workforce3_m.log';
result = gurobi(model, params);

% Display results
if strcmp(result.status, 'OPTIMAL')
    % The code may enter here if you change some of the data... otherwise
    % this will never be executed.
    printsolution(result, Shifts, Workers)
else
    if strcmp(result.status, 'INFEASIBLE')
        penalties.lb  = inf(nVars, 1);
        penalties.ub  = inf(nVars, 1);
        penalties.rhs = ones(nShifts, 1);
        feasrelax     = gurobi_feasrelax(model, 0, false, penalties, params);
        result        = gurobi(feasrelax.model, params);
        if strcmp(result.status, 'OPTIMAL')
            printsolution(result, Shifts, Workers);
            fprintf('Slack value:\n');
            for j = nVars+1:length(result.x)
                if result.x(j) > 0.1
                    fprintf('\t%s, %g\n', feasrelax.model.varnames{j}, result.x(j));
                end
            end
        else
            fprintf('Unexpected status %s\n',result.status);
        end
    else
        % Just to handle user interruptions or other problems
        fprintf('Unexpected status %s\n',result.status);
    end
end

end

function printsolution(result, Shifts, Workers)
% Helper function to display results
nShifts = length(Shifts);
nWorkers = length(Workers);
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
end
