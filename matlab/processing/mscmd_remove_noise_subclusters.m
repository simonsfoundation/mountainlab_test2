function mscmd_remove_noise_subclusters(raw_path,firings_in_path,firings_out_path,opts)

if (nargin<4) opts=struct; end;

cmd=sprintf('%s remove_noise_subclusters --raw=%s --firings_in=%s --firings_out=%s --clip_size=%d --detectability_threshold=%g --shell_increment=%g --min_shell_size=%d',mscmd_exe,...
    raw_path,firings_in_path,firings_out_path,opts.clip_size,opts.detectability_threshold,opts.shell_increment,opts.min_shell_size);

fprintf('\n*** REMOVE NOISE SUBCLUSTERS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
