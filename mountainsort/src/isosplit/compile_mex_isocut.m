tmp=pwd;
cd(fileparts(mfilename('fullpath')))
mex ./_mcwrap/mcwrap_isocut.cpp ./isocut.cpp ./mda.cpp ./jisotonic.cpp -output ./isocut 
cd(tmp);
