function qcp()
% Copyright 2020, Gurobi Optimization, LLC
%
% This example formulates and solves the following simple QCP model:
%  maximize
%      x
%  subject to
%      x + y + z = 1
%      x^2 + y^2 <= z^2 (second-order cone)
%      x^2 <= yz        (rotated second-order cone)
%      x, y, z non-negative

names = {'x', 'y', 'z'};
model.varnames = names;

% Set objective: x
model.obj = [ 1 0 0 ];
model.modelsense = 'max';

% Add constraint: x + y + z = 1
model.A   = sparse([1 1 1]);
model.rhs = 1;
model.sense = '=';

% Add second-order cone: x^2 + y^2 <= z^2 using a sparse matrix
model.quadcon(1).Qc = sparse([
    1 0  0;
    0 1  0;
    0 0 -1]);
model.quadcon(1).q  = zeros(3,1); 
model.quadcon(1).rhs = 0.0;
model.quadcon(1).name = 'std_cone';

% Add rotated cone: x^2 <= yz using sparse triplet representation
% Equivalent sparse matrix data:
%model.quadcon(2).Qc = sparse([
%    1 0  0;
%    0 0 -1;
%    0 0  0]);
model.quadcon(2).Qrow = [1, 2]
model.quadcon(2).Qcol = [1, 3]
model.quadcon(2).Qval = [1, -1]
% All-zero sparse 3-by-1 vector
model.quadcon(2).q  = sparse(3,1);
model.quadcon(2).rhs = 0.0;
model.quadcon(2).name = 'rot_cone';

gurobi_write(model, 'qcp.lp');

result = gurobi(model);

for j=1:3
    fprintf('%s %e\n', names{j}, result.x(j))
end

fprintf('Obj: %e\n', result.objval);
end
