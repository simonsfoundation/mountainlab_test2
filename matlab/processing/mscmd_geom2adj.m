function mscmd_geom2adj(infile,outfile,opts)
% mscmd_geom2adj  call mountainsort to convert geom CSV to adjacency matrix
%
% mscmd_geom2adj(infile,outfile,opts)
% Inputs:
%  infile - filename of input geometry x,y for electrodes in CSV
%  outfile - filename for output adjacency matrix in MDA
%  opts - struct containing required:
%       opts.
% Outputs:  (none)

% todo: self-test

% Barnett 7/27/16

if (nargin<3) opts=struct; end;
def_opts.adj_radius=0;
def_opts.channels=[];                      % will be all of them
opts=ms_set_default_opts(opts,def_opts);

commasepchanlist = sprintf('%d,',opts.channels);  % has trailing comma, is ok

cmd=sprintf('%s geom2adj --input=%s --output=%s --radius=%g --channels=%s',mscmd_exe,infile,outfile,opts.adj_radius,commasepchanlist);

fprintf('\n*** GEOM2ADJ ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end
