function mscmd_mda2txt(mda_path,txt_path,opts)
%MSCMD_MDA2TXT - Convert a .mda file to .txt file
%
% Syntax:  mscmd_mda2txt(input_path,output_path,opts)
%
% Inputs:
%    input_path - the path of the source .mda file
%    output_path - the path of the destination .txt file
%    opts -- script below
%
% Other m-files required: mscmd_exe
%
% See also: mscmd_copy.m

% Author: Jeremy Magland
% Mar 2016; Last revision: 4-Mar-2016

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