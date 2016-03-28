function mv_closeall
mfilepath=fileparts(mfilename('fullpath'));
f=fopen([mfilepath,'/../../mountainview/bin/closeme.tmp'],'wb');
if (f>=0) fclose(f); end;
end