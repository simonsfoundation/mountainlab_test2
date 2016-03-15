function mscmd_detect(input_path,output_path,opts)
%MSCMD_DETECT - Detect super-threshold events in a raw/preprocessed dataset
%
%This is a wrapper to a command-line call to mountainsort. The
%functionality is similar to that of ms_detect.
%
% Syntax:  mscmd_detect(input_path,output_path,opts)
%
% Inputs:
%    input_path - path to input MxN array of raw or preprocessed data
%    output_path - path to output 2xL array of detected event channels and
%                  times. The first row contains the primary channels of
%                  the detected events and the second row contains the
%                  integer timepoints at the events.
%    opts.individual_channels - If 1, events are detected independently on
%                               the different channels. If 0, then the
%                               maximum signal over all channels is used to
%                               detect events. The first method will result
%                               in a lot more detected events, and should be
%                               used when the detector array is large.
%    opts.detect_interval - minimum number of integer timepoints separating
%                           two detected events
%    opts.detect_threshold - detect events where the absolute value of the
%                            signal exceeds this threshold.
%    opts.outer_window_width  ... to be documented.
%    opts.clip_size - don't get too close to the edges!!
%
% Other m-files required: mscmd_exe
%
% See also: mscmd_detect, ms_detect

% Author: Jeremy Magland
% Jan 2016; Last revision: 16-Feb-2016. AHB

if (nargin<3) opts=struct; end;

if ~isfield(opts,'individual_channels') opts.individual_channels=0; end;
if ~isfield(opts,'normalize') opts.normalize=0; end;
if ~isfield(opts,'inner_window_width') opts.inner_window_width=10; end;
if isfield(opts,'detect_interval') opts.inner_window_width=opts.detect_interval; end;
if isfield(opts,'detect_threshold') opts.threshold=opts.detect_threshold; end;
if ~isfield(opts,'outer_window_width') opts.outer_window_width=1000; end;
if ~isfield(opts,'clip_size') opts.clip_size=100; end;

cmd=sprintf('%s detect --input=%s --output=%s ',mscmd_exe,input_path,output_path);
cmd=[cmd,sprintf('--inner_window_width=%d --outer_window_width=%d --individual_channels=%d --clip_size=%d ',opts.inner_window_width,opts.outer_window_width,opts.individual_channels,opts.clip_size)];
cmd=[cmd,sprintf('--threshold=%g --normalize=%d ',opts.threshold,opts.normalize)];

fprintf('\n*** DETECT ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end