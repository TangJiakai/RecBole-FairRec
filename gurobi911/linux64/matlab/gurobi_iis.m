%   gurobi_iis()
%
%    gurobi_iis ( model, params, env )
%    gurobi_iis ( model, params )
%    gurobi_iis ( model )
%
%    Compute an Irreducible Inconsistent Subsystem (IIS).
%
%    An IIS is a subset of the constraints and variable bounds with the
%    following properties:
%      * It is still infeasible, and
%      * If a single constraint or bound is removed, the subsystem becomes
%        feasible.
%
%    Note that an infeasible model may have multiple IISs. The one returned
%    by Gurobi is not necessarily the smallest one; there may exist others
%    with fewer constraints or bounds.
%
%    IIS results are returned in a number of attributes: [1]IISConstr,
%    [2]IISLB, [3]IISUB, [4]IISSOS, [5]IISQConstr, and [6]IISGenConstr. Each
%    indicates whether the corresponding model element is a member of the
%    computed IIS.
%
%    The [7]IIS log provides information about the progress of the
%    algorithm, including a guess at the eventual IIS size.
%
%    If an IIS computation is interrupted before completion, Gurobi will
%    return the smallest infeasible subsystem found to that point.
%
%    You can obtain information about the outcome of the IIS computation
%    from the returned IIS result (described below). Note that this method
%    can be used to compute IISs for both continuous and MIP models.
%
%    Arguments:
%
%    model: The model struct must contain a valid Gurobi model. See the
%    model argument section for more information.
%
%    params: The params struct, when provided, contains a list of modified
%    Gurobi parameters. See the params argument section for more
%    information.
%
%    env: The env struct, when provided, allows you to use Gurobi Compute
%    Server or Gurobi Instant Cloud. See the env argument section for more
%    information.
%
%    Example usage:
%    model = gurobi_read('examples/data/klein1.mps');
%    iis = gurobi_iis(model);
%
%    All details on input and output arguments, and general
%    information on the Gurobi Optimizer Matlab Interface, are given in
%    <a href="matlab:web(fullfile(fileparts(which('gurobi')), 'html', 'index.html'))">included html documentation</a>, and on-line at the <a href="matlab:web('https://www.gurobi.com/documentation/9.1/refman/matlab_api_overview.html')">Gurobi Documentation</a> page.
%
% Copyright 2020, Gurobi Optimization, LLC
%
