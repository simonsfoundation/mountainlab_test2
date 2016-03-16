function mscmd_copy(input_path,output_path)

cmd=sprintf('%s copy --input=%s --output=%s',mscmd_exe,...
    input_path,output_path);

fprintf('\n*** COPY ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;


end