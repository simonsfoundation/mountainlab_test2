function [CC,CCmda]=ms_cross_correlograms(times,labels,max_dt)
%MS_CROSS_CORRELOGRAMS - The slow way to generate cross correlogram data.
%Better to use mscmd_cross_correlograms
%
%MountainView can be used to view cross-correlograms. It requires the
%cross-correlograms to be prepared with a particular format, provided in
%the CCmda file.
%
% Syntax:  [CC,CCmda] = ms_cross_correlograms(times,labels,max_dt)
%
% Inputs:
%    times - 1xL array of integer timepoints
%    labels - 1xL array of corresponding labels
%    max_dt - the maximum integer timepoint interval to consider
%
% Outputs:
%    CC - KxK cell array where K=max(labels)
%         each CC{k1,k2} is a vector of event time differences between
%         clusters k1 and k2
%    CCmda - The information in CC organized into a single array that can
%            be used by MountainView when written as a file.
%
% Example: 
%    [~,CCmda]=ms_cross_correlograms(times,labels,1000)
%    writemda('cross_correlograms.mda',CCmda);
%    % Now run MountainView and point to this file
%
% Other m-files required: none
%
% See also: mscmd_cross_correlograms, ms_mountainview

% Author: Jeremy Magland
% Jan 2016; Last revision: 13-Feb-2016

[times,inds]=sort(times);
labels=labels(inds);

K=max(labels);
CC=cell(K,K);

i1=1;
for i2=1:length(times)
    if (mod(i2,10000)==0)
        fprintf('%d%% ',floor(100*i2/length(times)));
    end;
    while (times(i1)<times(i2)-max_dt) i1=i1+1; end;
    k2=labels(i2);
    t2=times(i2);
    for jj=i1:i2-1
        k1=labels(jj);
        t1=times(jj);
        CC{k1,k2}=[CC{k1,k2},t2-t1];
        CC{k2,k1}=[CC{k2,k1},t1-t2];
    end;
end;
fprintf('\n');

CCmda=cross_correlograms_to_mda(CC);

end

function ret=cross_correlograms_to_mda(CC)
K=size(CC,1);

ct=0;
for k1=1:K
for k2=1:K
ct=ct+length(CC{k1,k2});
end;
end;
ret=zeros(3,ct);

ct=0;
for k1=1:K
for k2=1:K
ret(1,ct+1:ct+length(CC{k1,k2}))=k1;
ret(2,ct+1:ct+length(CC{k1,k2}))=k2;
ret(3,ct+1:ct+length(CC{k1,k2}))=CC{k1,k2};
ct=ct+length(CC{k1,k2});
end;
end;

end

