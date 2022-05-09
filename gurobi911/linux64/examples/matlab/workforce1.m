function workforce1()

% Copyright 2020, Gurobi Optimization, LLC
%
% Assign workers to shifts; each worker may or may not be available on a
% particular day. If the problem cannot be solved, use IIS to find a set of
% conflicting constraints. Note that there may be additional conflicts
% besides what is reported via IIS.

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
model.modelname  = 'workforce1';
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
gurobi_write(model,'workforce1_m.lp');

% Optimize
params.logfile = 'workforce1_m.log';
result = gurobi(model, params);

% Display results
if strcmp(result.status, 'OPTIMAL')
    % The code may enter here if you change some of the data... otherwise
    % this will never be executed.
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
else
    if strcmp(result.status, 'INFEASIBLE')
        fprintf('Problem is infeasible.... computing IIS\n');
        iis = gurobi_iis(model, params);
        if iis.minimal
            fprintf('IIS is minimal\n');
        else
            fprintf('IIS is not minimal\n');
        end
        
        if any(iis.Arows)
            fprintf('Rows in IIS: ');
            disp(strjoin(model.constrnames(iis.Arows)));
        end
        if any(iis.lb)
            fprintf('LB in IIS: ');
            disp(strjoin(model.varnames(iis.lb)));
        end
        if any(iis.ub)
            fprintf('UB in IIS: ');
            disp(strjoin(model.varnames(iis.ub)));
        end
    else
        % Just to handle user interruptions or other problems
        fprintf('Unexpected status %s\n',result.status);
    end
end
