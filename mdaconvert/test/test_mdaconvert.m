% prerequisites:
%   run mountainlab_setup.m
%   make sure mdaconvert/bin is in your path

function test_mdaconvert

X=rand(5,20)*1000;
path1=pathify64(X)
Y=readmda(path1);
compare_arrays(X,Y);

cmd=sprintf('mdaconvert %s test1.raw',path1);
system(cmd);

cmd=sprintf('mdaconvert test1.raw test1.mda --dtype=float64 --dims=5x20');
system(cmd);

Z=readmda('test1.mda');

compare_arrays(X,Z);

end

function compare_arrays(X,Y)
difference=X-Y;
disp(max(abs(difference(:))));
end