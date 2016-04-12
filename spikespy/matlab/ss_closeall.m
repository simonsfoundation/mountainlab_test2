function ss_closeall
mfilepath=fileparts(mfilename('fullpath'));
f=fopen([mfilepath,'/../bin/closeme.tmp'],'wb');
if (f>=0) fclose(f); end;
end