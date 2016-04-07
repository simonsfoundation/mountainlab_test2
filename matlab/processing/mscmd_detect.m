function mscmd_detect(timeseries_path,detect_out_path,opts)
%MSCMD_DETECT - Detect super-threshold events in a raw/preprocessed dataset
%
%This is a wrapper to a command-line call to mountainsort. The
%functionality is similar to that of ms_detect.
%
% Syntax:  mscmd_detect(input_path,output_path,opts)
%
% Inputs:
%    timeseries_path - path to input MxN array of raw or preprocessed data
%    detect_out_path - path to output 2xL array of detected event channels and
%                  times. The first row contains the primary channels of
%                  the detected events and the second row contains the
%                  integer timepoints at the events.
%    opts.detect_interval - minimum number of integer timepoints separating
%                           two detected events
%    opts.detect_threshold - detect events where the absolute value of the
%                            signal exceeds this threshold.
%    opts.clip_size - don't get too close to the edges!!
%    opts.sign - 0,-1,or 1 (use zero to detect both pos and neg peaks)
%    opts.individual_channels - 0 or 1 (if 0 fills the first row of output with zeros)
%
% Other m-files required: mscmd_exe
%
% See also: ms_detect

% Author: Jeremy Magland
% Jan 2016

if (nargin<3) opts=struct; end;

if ~isfield(opts,'clip_size') opts.clip_size=100; end;
if ~isfield(opts,'sign') opts.sign=0; end;
if ~isfield(opts,'individual_channels') opts.individual_channels=1; end;

cmd=sprintf('%s detect --timeseries=%s --detect_out=%s ',mscmd_exe,timeseries_path,detect_out_path);
cmd=[cmd,sprintf('--clip_size=%d ',opts.clip_size)];
cmd=[cmd,sprintf('--detect_threshold=%g --detect_interval=%d ',opts.detect_threshold,opts.detect_interval)];
cmd=[cmd,sprintf('--sign=%d ',opts.sign)];
cmd=[cmd,sprintf('--individual_channels=%d ',opts.individual_channels)];

fprintf('\n*** DETECT ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
