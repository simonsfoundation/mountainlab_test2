function mscmd_compute_detectability_scores(timeseries_path,firings_in_path,firings_out_path,opts)
% TODO: Docs
if (nargin<4) opts=struct; end;

cmd=sprintf('%s compute_detectability_scores --timeseries=%s --firings=%s --firings_out=%s --clip_size=%d --shell_increment=%g --min_shell_size=%d',mscmd_exe,...
    timeseries_path,firings_in_path,firings_out_path,opts.clip_size,opts.shell_increment,opts.min_shell_size);

fprintf('\n*** COMPUTE DETECTABILITY SCORES ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
