function mscmd_merge_across_channels(timeseries_path,firings_path,firings_out_path,opts)

if (nargin<4) opts=struct; end;
if ~isfield(opts,'min_peak_ratio'), opts.min_peak_ratio = 0.7; end
if ~isfield(opts,'max_dt'), opts.max_dt = 10; end      % max difference between peaks of same event peak on different channels
if ~isfield(opts,'min_coinc_frac'), opts.min_coinc_frac = 0.1; end 
if ~isfield(opts,'max_corr_stddev'), opts.max_corr_stddev = 3; end     % in sample units
if ~isfield(opts,'min_template_corr_coef'), opts.min_template_corr_coef = 0.5; end    % waveform corr coeff
if ~isfield(opts,'min_coinc_num'), opts.min_coinc_num = 10; end
if ~isfield(opts,'clip_size'), opts.clip_size = 100; end

cmd=sprintf('%s merge_across_channels --timeseries=%s --firings=%s --firings_out=%s --min_peak_ratio=%g --max_dt=%d --min_coinc_frac=%g --min_coinc_num=%d --max_corr_stddev=%g --min_template_corr_coef=%g --clip_size=%d',mscmd_exe,...
    timeseries_path,firings_path,firings_out_path,...
    opts.min_peak_ratio,opts.max_dt,opts.min_coinc_frac,opts.min_coinc_num,...
    opts.max_corr_stddev,opts.min_template_corr_coef,opts.clip_size);

fprintf('\n*** MERGE ACROSS CHANNELS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
