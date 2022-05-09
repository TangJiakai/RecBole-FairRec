%   gurobi_read()
%
%    gurobi_read ( filename, env )
%    gurobi_read ( filename )
%
%    Reads a model from a file.
%
%    Arguments:
%
%    filename: Name of the file to read. Note that the type of the file is
%    encoded in the file name suffix. The filename suffix should be one of
%    .mps, .rew, .lp, .rlp, .ilp, or .opb (see the file formats section for
%    details on Gurobi file formats). The files can be compressed, so
%    additional suffixes of .gz, .bz2, .zip, or .7z are accepted.
%
%    env: The env struct, when provided, allows you to use Gurobi Compute
%    Server or Gurobi Instant Cloud. See the env argument section for more
%    information.
%
%    Return value:
%
%    A model struct variable, as described in the model section.
%
%    Example usage:
%    model = gurobi_read('stein9.mps');
%    result = gurobi(model);
%
%    All details on input and output arguments, and general
%    information on the Gurobi Optimizer Matlab Interface, are given in
%    <a href="matlab:web(fullfile(fileparts(which('gurobi')), 'html', 'index.html'))">included html documentation</a>, and on-line at the <a href="matlab:web('https://www.gurobi.com/documentation/9.1/refman/matlab_api_overview.html')">Gurobi Documentation</a> page.
%
% Copyright 2020, Gurobi Optimization, LLC
%
