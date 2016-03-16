function mscmd_mda2txt(mda_path,txt_path,opts)

if (nargin<3) opts=struct; end;

if (~isfield(opts,'transpose')) opts.transpose=1; end;
if (~isfield(opts,'max_rows')) opts.max_rows=1e9; end;
if (~isfield(opts,'max_cols')) opts.max_cols=200; end;
if (~isfield(opts,'delimiter')) opts.delimiter='tab'; end;

cmd=sprintf('%s mda2txt --mda_file=%s --txt_file=%s --transpose=%d --max_rows=%g --max_cols=%g --delimiter=%s',mscmd_exe,...
    mda_path,txt_path,opts.transpose,opts.max_rows,opts.max_cols,opts.delimiter);

fprintf('\n*** MDA2TXT ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;


end