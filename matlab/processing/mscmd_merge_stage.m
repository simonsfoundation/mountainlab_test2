function mscmd_merge_stage(timeseries_in_path,firings_in_path,firings_out_path,opts)

if (nargin<4) opts=struct; end;

if (~isfield(opts,'clip_size')) opts.clip_size=100; end;
if (~isfield(opts,'min_peak_ratio')) opts.min_peak_ratio=0.7; end;
if (~isfield(opts,'min_template_corr_coef')) opts.min_template_corr_coef=0.5; end;

cmd=sprintf('%s merge_stage --timeseries=%s --firings=%s --firings_out=%s --clip_size=%d --min_peak_ratio=%g --min_template_corr_coef=%g ',mscmd_exe,timeseries_in_path,firings_in_path,firings_out_path,...
    opts.clip_size,opts.min_peak_ratio,opts.min_template_corr_coef);

fprintf('\n*** MERGE STAGE ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
