function mscmd_normalize_channels(input_path,output_path,opts)
%MSCMD_NORMALIZE_CHANNELS - Normalize each row (channel) to have unit variance
%
% This is a wrapper to the command-line mountainsort procedure. It has
% identical functionality to ms_normalize_channels.m
%
% Syntax:  mscmd_normalize_channels(input_path,output_path,opts)
%
% Inputs:
%    input_path - path to MxN array of raw or pre-processed data
%    output_path - path to MxN output array of normalized data
%
% Other m-files required: none
%
% Author: Alex Barnett 7/28/16

if (nargin<3) opts=struct; end;

cmd=sprintf('%s normalize_channels --timeseries=%s --timeseries_out=%s ',mscmd_exe,input_path,output_path);

fprintf('\n*** NORMALIZE CHANNELS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
