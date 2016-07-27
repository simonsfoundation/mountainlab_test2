function firings2=ms_subfirings(firings,opts)

K=max(firings(3,:));
L=size(firings,2);

if (~isfield(opts,'clusters'))
    opts.clusters=[];
end;

to_use=zeros(1,L);
for ii=1:length(opts.clusters)
    inds0=find(firings(3,:)==opts.clusters(ii));
    to_use(inds0)=1;
end

inds1=find(to_use);
firings2=firings(:,inds1);

end