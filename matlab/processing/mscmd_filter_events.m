function mscmd_filter_events(firings_in_path,firings_out_path,opts)

if (nargin<3) opts=struct; end;

if (~isfield(opts,'detectability_threshold')) opts.detectablity_threshold=0; end;
if (~isfield(opts,'outlier_threshold')) opts.outlier_threshold=inf; end;

if (isinf(opts.outlier_threshold))
    opts.outlier_threshold=0;
end;


cmd=sprintf('%s filter_events --firings=%s --firings_out=%s --detectability_threshold=%g --outlier_threshold=%g',mscmd_exe,firings_in_path,firings_out_path,...
    opts.detectability_threshold,opts.outlier_threshold);

fprintf('\n*** FILTER EVENTS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
