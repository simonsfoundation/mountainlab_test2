function mountainlab_setup

mfile_path=fileparts(mfilename('fullpath'));
addpath([mfile_path,'/sorting_algorithms']);
addpath([mfile_path,'/processing']);
addpath([mfile_path,'/view']);
addpath([mfile_path,'/view/colorspace']);
addpath([mfile_path,'/msutils']);
addpath([mfile_path,'/3rdparty']);
addpath([mfile_path,'/3rdparty/jsonlab']);
addpath([mfile_path,'/unit_tests']);
addpath([mfile_path,'/validation']);
addpath([mfile_path,'/misc_examples']);
addpath([mfile_path,'/../mountainsort/src/isosplit']);
addpath([mfile_path,'/../spikespy/matlab']); %added on 4/12/16

% let ML see env variables without Matlab's glibc, which may
% be older than on the linux distribution:
% (ahb added after Matlab system calls failed 8/9/16)
setenv('LD_LIBRARY_PATH','');

end
