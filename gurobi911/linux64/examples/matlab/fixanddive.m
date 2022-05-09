function fixanddive(filename)
%
% Copyright 2020, Gurobi Optimization, LLC
%
% Implement a simple MIP heuristic.  Relax the model,
% sort variables based on fractionality, and fix the 25% of
% the fractional variables that are closest to integer variables.
% Repeat until either the relaxation is integer feasible or
% linearly infeasible.

% Read model
fprintf('Reading model %s\n', filename);

model = gurobi_read(filename);
cols = size(model.A, 2);
ivars = find(model.vtype ~= 'C');

if length(ivars) <= 0
    fprintf('All variables of the model are continuous, nothing to do\n');
    return;
end

% save vtype and set all variables to continuous
vtype = model.vtype;
model.vtype = repmat('C', cols, 1);

params.OutputFlag = 0;

result = gurobi(model, params);

% Perform multiple iterations. In each iteration, identify the first
% quartile of integer variables that are closest to an integer value
% in the relaxation, fix them to the nearest integer, and repeat.

frac = zeros(cols, 1);
for iter = 1:1000
    % See if status is optimal
    if ~strcmp(result.status, 'OPTIMAL')
        fprintf('Model status is %s\n', result.status);
        fprintf('Can not keep fixing variables\n');
        break;
    end
    % collect fractionality of integer variables
    fracs = 0;
    for j = 1:cols
        if vtype(j) == 'C'
            frac(j) = 1; % indicating not integer variable
        else
            t = result.x(j);
            t = t - floor(t);
            if t > 0.5
                t = t - 0.5;
            end
            if t > 1e-5
                frac(j) = t;
                fracs = fracs + 1;
            else
                frac(j) = 1; % indicating not fractional
            end
        end
    end
    
    fprintf('Iteration %d, obj %g, fractional %d\n', iter, result.objval, fracs);
    
    if fracs == 0
        fprintf('Found feasible solution - objective %g\n', result.objval);
        break;
    end
    
    % sort variables based on fractionality
    [~, I] = sort(frac);
    
    % fix the first quartile to the nearest integer value
    nfix = max(fracs/4, 1);
    for i = 1:nfix
        j = I(i);
        t = floor(result.x(j) + 0.5);
        model.lb(j) = t;
        model.ub(j) = t;
    end
    
    % use warm start basis and reoptimize
    model.vbasis = result.vbasis;
    model.cbasis = result.cbasis;
    result = gurobi(model, params);
end
