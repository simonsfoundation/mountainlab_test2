function mscmd_fit_stage(timeseries_in_path,firings_in_path,firings_out_path,opts)

if (nargin<4) opts=struct; end;

if (~isfield(opts,'clip_size')) opts.clip_size=100; end;
if (~isfield(opts,'neglogprior')) opts.neglogprior=30; end;

cmd=sprintf('%s fit_stage --timeseries=%s --firings=%s --firings_out=%s --use_old=0 --neglogprior=%g --clip_size=%d --shell_increment=%g --min_shell_size=%d',mscmd_exe,timeseries_in_path,firings_in_path,firings_out_path,...
    opts.neglogprior,opts.clip_size,opts.shell_increment,opts.min_shell_size);

fprintf('\n*** FIT STAGE ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
