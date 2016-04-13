function mscmd_merge_labels(timeseries_in_path,firings_in_path,firings_out_path,opts)

if (nargin<4) opts=struct; end;

if (~isfield(opts,'merge_threshold')) opts.merge_threshold=0.9; end;
if (~isfield(opts,'clip_size')) opts.clip_size=100; end;

cmd=sprintf('%s merge_labels --timeseries=%s --firings=%s --firings_out=%s --clip_size=%d --merge_threshold=%g',mscmd_exe,timeseries_in_path,firings_in_path,firings_out_path,...
    opts.clip_size,opts.merge_threshold);

fprintf('\n*** MERGE LABELS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
