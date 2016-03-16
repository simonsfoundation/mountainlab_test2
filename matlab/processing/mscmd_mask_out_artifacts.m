function mscmd_mask_out_artifacts(raw_in_path,raw_out_path,opts)

if (nargin<3) opts=struct; end;

if (~isfield(opts,'threshold')) opts.threshold=10; end;
if (~isfield(opts,'interval_size')) opts.interval_size=200; end;

cmd=sprintf('%s mask_out_artifacts --raw_in=%s --raw_out=%s --threshold=%g --interval_size=%d',mscmd_exe,raw_in_path,raw_out_path,...
    opts.threshold,opts.interval_size);

fprintf('\n*** MASK OUT ARTIFACTS ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end
