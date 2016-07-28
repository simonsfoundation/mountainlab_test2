function mscmd_extract_raw(input_path,output_path,opts)
%MSCMD_EXTACT_RAW - extract a time chunk and/or channel subset of a timeseries
%
% This is a wrapper to the command-line mountainsort procedure.
%
% Syntax:  mscmd_extract_raw(input_path,output_path,opts)
%
% Inputs:
%    input_path - path to MxN timeseries array
%    output_path - path to Mxn output array
%    opts.t1, opts.t2 - start and end times in time points (if both -1 uses entire
%                       time range).
%
% Other m-files required: none
%
% Author: Alex Barnett 7/28/16

if (nargin<3) opts=struct; end;
def_opts.t1=-1;                       % defaults
def_opts.t2=-1;
def_opts.channels=[];                      % will be all of them
opts=ms_set_default_opts(opts,def_opts);

commasepchanlist = sprintf('%d,',opts.channels);  % has trailing comma, is ok

cmd=sprintf('%s extract_raw --timeseries=%s --timeseries_out=%s --t1=%d --t2=%d --channels=%s',mscmd_exe,input_path,output_path,opts.t1,opts.t2,commasepchanlist);

fprintf('\n*** EXTACT RAW ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
