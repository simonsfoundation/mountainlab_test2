function run_mountainlab_setup

mfile_path=fileparts(mfilename('fullpath'));
addpath([mfile_path,'/../../../matlab']);
mountainlab_setup;

end