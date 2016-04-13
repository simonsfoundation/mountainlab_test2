function mscmd_branch_cluster_v2(timeseries_path,detect_path,adjacency_matrix_path,out_firings_path,opts)
% TODO: Docs
if (nargin<5) opts=struct; end;

if (~isfield(opts,'clip_size')) opts.clip_size=100; end;
if (~isfield(opts,'min_shell_size')) opts.min_shell_size=50; end;
if (~isfield(opts,'shell_increment')) opts.shell_increment=0.5; end;
if (~isfield(opts,'num_features')) opts.num_features=3; end;
if (~isfield(opts,'detect_interval')) opts.detect_interval=10; end;
if (~isfield(opts,'consolidation_factor')) opts.consolidation_factor=0.9; end; %added 4/12/16

cmd=sprintf('%s branch_cluster_v2 --timeseries=%s --detect=%s --adjacency_matrix=%s --firings_out=%s --clip_size=%d --min_shell_size=%d --shell_increment=%g --num_features=%d --detect_interval=%d --consolidation_factor=%g',mscmd_exe,timeseries_path,detect_path,adjacency_matrix_path,out_firings_path,...
    opts.clip_size,opts.min_shell_size,opts.shell_increment,opts.num_features,opts.detect_interval,opts.consolidation_factor);

fprintf('\n*** BRANCH CLUSTER V2 ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
