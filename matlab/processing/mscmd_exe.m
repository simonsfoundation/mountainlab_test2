function ret=mscmd_exe
%MSCMD_EXE - Return the path to the mountainsort executable file
%
%This function is used internally by the mscmd_*.m files
%
% Syntax:  mscmd_exe
%
% Other m-files required: none
%
% See also:

% Author: Jeremy Magland
% Jan 2016; Last revision: 13-Feb-2106
mfile_path=fileparts(mfilename('fullpath'));
exe_fname=sprintf('%s/../../mountainsort/bin/mountainsort',mfile_path);
%ret=sprintf('%s %s','LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib',exe_fname);
ret=sprintf('%s %s','LD_LIBRARY_PATH=/usr/local/lib',exe_fname);
if (~exist(exe_fname,'file'))
    error('File does not exist: %s\n',exe_fname);
end;
end