mfile_path=fileparts(mfilename('fullpath'))

mv_file=loadjson([mfile_path,'/example.mv']);

disp('The mv file as a matlab object:');
disp(mv_file);

disp(mv_file.cluster_attributes('0'));