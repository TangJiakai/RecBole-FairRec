function lpmod(filename)

% Copyright 2020, Gurobi Optimization, LLC
%
% This example reads an LP model from a file and solves it.
% If the model can be solved, then it finds the smallest positive variable,
% sets its upper bound to zero, and resolves the model two ways:
% first with an advanced start, then without an advanced start
% (i.e. 'from scratch').

% Read model
fprintf('Reading model %s\n', filename);

model = gurobi_read(filename);

if (isfield(model, 'multiobj')  && ~isempty(model.multiobj))  || ...
   (isfield(model, 'sos')       && ~isempty(model.sos))       || ...
   (isfield(model, 'pwlobj')    && ~isempty(model.pwlobj))    || ...
   (isfield(model, 'quadcon')   && ~isempty(model.quadcon))   || ...
   (isfield(model, 'genconstr') && ~isempty(model.genconstr)) || ...
    isfield(model, 'Q')
    fprintf('The model is not a linear program, quit\n');
    return;
end

ivars = find(model.vtype ~= 'C');
ints = length(ivars);

if ints > 0
    fprintf('problem is a MIP, quit\n');
    return;
end

result = gurobi(model);
if ~strcmp(result.status, 'OPTIMAL')
    fprintf('This model cannot be solved because its optimization status is %s\n', result.status);
    return;
end

cols = size(model.A,2);

% create lb if they do not exists, and set them to default values
if ~isfield(model, 'lb') || isempty(model.lb)
    model.lb = zeros(cols, 1);
end

% Find the smallest variable value
minVal = inf;
minVar = -1;
for j = 1:cols
    if result.x(j) > 0.0001 && result.x(j) < minVal && model.lb(j) == 0.0
        minVal = result.x(j);
        minVar = j;
    end
end

fprintf('\n*** Setting %s from %d to zero ***\n', model.varnames{minVar}, minVar);
model.ub(minVar) = 0;

model.vbasis = result.vbasis;
model.cbasis = result.cbasis;

% Solve from this starting point
result = gurobi(model);

% Save iteration & time info
warmCount = result.itercount;
warmTime = result.runtime;

% Remove warm start basis and resolve
model = rmfield(model, 'vbasis');
model = rmfield(model, 'cbasis');
result = gurobi(model);

coldCount = result.itercount;
coldTime = result.runtime;

fprintf('\n');
fprintf('*** Warm start: %g iterations, %g seconds\n', warmCount, warmTime);
fprintf('*** Cold start: %g iterations, %g seconds\n', coldCount, coldTime);
