function mscmd_copy(input_path,output_path)
%MSCMD_COPY - Copy a file using mountainsort, so that provenance is
%             preserved
%
%Sometimes we just want to copy a file from one path to another. But we
%don't want to disrupt the provenance tracking of mountainsort. In those
%cases, mscmd_copy should be used!
%
% Syntax:  mscmd_copy(input_path,output_path)
%
% Inputs:
%    input_path - the path of the source file
%    output_path - the path of the destination file
%
% Other m-files required: mscmd_exe
%
% See also:

% Author: Jeremy Magland
% Jan 2016; Last revision: 13-Feb-2016

cmd=sprintf('%s copy --input=%s --output=%s',mscmd_exe,...
    input_path,output_path);

fprintf('\n*** COPY ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;


end