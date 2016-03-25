function mscmd_remove_noise_subclusters(timeseries_path,firings_in_path,firings_out_path,opts)
%MSCMD_REMOVE_NOISE_SUBCLUSTERS - Remove noise subclusters by splitting clusters
%into shells and comparing the subcluster template waveforms with the
%expected waveform shape in the situation of pure noise detected via
%amplitude threshold.
%
%Use of this instead of ms_remove_noise_subclusters is recommended
%
% Syntax:  mscmd_remove_noise_subclusters(pre,firings,opts)
%
% Inputs:
%    pre - path to MxN array of preprocessed raw data
%    firings - path to RxL array of firings containing times/labels etc (see docs)
%    firings_out - path to output array
%    opts.clip_size - the clip size in timepoints, e.g. 100
%    opts.detectability_threshold - the threshold for accepting a
%             subcluster as non-noise. For example use 4.
%    opts.shell_increment - Controls the definition of subshells, e.g. 0.5
%    opts.min_shell_size - Controls the definition of subshells, e.g. 100
%
% Other m-files required: mscmd_exe
%
% See also: ms_remove_noise_subclusters

% Author: Jeremy Magland
% Feb 2016; Last revision: 29-Feb-2016 (leap day)

if (nargin<4) opts=struct; end;

cmd=sprintf('%s remove_noise_subclusters --timeseries=%s --firings=%s --firings_out=%s --clip_size=%d --detectability_threshold=%g --shell_increment=%g --min_shell_size=%d',mscmd_exe,...
    timeseries_path,firings_in_path,firings_out_path,opts.clip_size,opts.detectability_threshold,opts.shell_increment,opts.min_shell_size);

fprintf('\n*** REMOVE NOISE SUBCLUSTERS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
