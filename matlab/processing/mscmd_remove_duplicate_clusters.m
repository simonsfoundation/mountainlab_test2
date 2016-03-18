function mscmd_remove_duplicate_clusters(firings_in_path,firings_out_path,opts)
% TODO: Docs
if (nargin<3) opts=struct; end;

if (~isfield(opts,'max_dt')) opts.max_dt=6; end;
if (~isfield(opts,'overlap_threshold')) opts.overlap_threshold=0.25; end;

cmd=sprintf('%s remove_duplicate_clusters --firings=%s --firings_out=%s --max_dt=%d --overlap_threshold=%g',mscmd_exe,firings_in_path,firings_out_path,...
    opts.max_dt,opts.overlap_threshold);

fprintf('\n*** REMOVE DUPLICATE CLUSTERS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
