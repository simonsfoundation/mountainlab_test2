function mscmd_remove_duplicate_clusters(timeseries_in_path,firings_in_path,firings_out_path,opts)
% TODO: Docs
if (nargin<4) opts=struct; end;

if (~isfield(opts,'clip_size')) opts.clip_size=100; end;

cmd=sprintf('%s remove_duplicate_clusters --timeseries=%s --firings=%s --firings_out=%s --clip_size=%d',mscmd_exe,...
    timeseries_in_path,firings_in_path,firings_out_path,...
    opts.clip_size);

fprintf('\n*** REMOVE DUPLICATE CLUSTERS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
