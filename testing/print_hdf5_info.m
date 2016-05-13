function print_hdf5_info(path)
%PRINT_HDF5_INFO - display information about a generic HDF5 file. Useful
%for exploring the contents of the file.
%
% Uses MATLAB's hdf5info and hdf5read
%
% Syntax: print_hdf5_info(path)
%
% Inputs:
%    path - path to the hdf5 file
%
% Other m-files required: none
%
% See also: 

% Author: Jeremy Magland
% Jan 2015; Last revision: 15-Feb-2106

fprintf('%s\n',path);
info=hdf5info(path);
print_hdf5_group(info.GroupHierarchy,path);

end


function print_hdf5_group(G,path)
for j=1:length(G.Groups)
    print_hdf5_group(G.Groups(j),path);
end;
for j=1:length(G.Datasets)
    print_hdf5_dataset(G.Datasets(j),path);
end;
end

function print_hdf5_dataset(D,path)
dtype=D.Datatype(1).Class;
fprintf('%s: ',D.Name);
if (strcmp(dtype,'H5T_STRING'));
    tmp=hdf5read(path,D.Name);
    str=tmp.Data;
    fprintf('"%s"',str);
    fprintf('\n');
else
    if (length(D.Dims)==0)
        tmp=hdf5read(path,D.Name);
        fprintf('%g\n',tmp);
    else
        fprintf('%s %s\n',dtype,num2str(D.Dims));
    end;
end;
end
