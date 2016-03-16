function writemda_if_necessary(X,fname)
% note, is 32-bit for now
if (~mda_equals_file(X,fname)) writemda32(X,fname); end;
end

function ret=mda_equals_file(X,fname)
if (exist(fname,'file'))
    Y=readmda(fname);
    ret=isequal(X,Y);
else
    ret=0;
end;
end
