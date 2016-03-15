function X=ms_whiten(X)
%MS_WHITEN - Channel-whiten an array of raw or preprocessed data
%
%Consider using mscmd_whiten
%
% Syntax:  [Y] = ms_whiten(X)
%
% Inputs:
%    X - MxN array of raw or pre-processed data
%
% Outputs:
%    Y - MxN array of channel-whitened data
%
% Other m-files required: none
%
% See also: mscmd_whiten, spikespy

% Author: Jeremy Magland
% Jan 2015; Last revision: 13-Feb-2106

if nargin<1, test_ms_whiten; return; end;

[M,N]=size(X);

%we are not going to subtract the mean, actually
%mu = mean(X,2); 
%X = X-repmat(mu,1,size(X,2));
[U,D,V] = svd(X,'econ');
D(D~=0)=1./D(D~=0);
X=sqrt(N-1)*U*D(1:M,1:M)*(U'*X);

end

function X=ms_whiten_XXt(X)
%used to show that we get the same result

[M,N]=size(X);

mu = mean(X,2); 
X = X-repmat(mu,1,size(X,2));
[U,D,V] = svd(X*X');
X=sqrt(N-1)*U*sqrt(inv(D))*(U'*X);
end

function X=ms_whiten_mscmd(X)
writemda(X,'X_tmp.mda');
mscmd_whiten('X_tmp.mda','X_tmp_white.mda');
X=readmda('X_tmp_white.mda');
end

function test_ms_whiten
N=1e6;
X=rand(4,N);
tic; Y1=ms_whiten(X); toc
Y1*Y1'/(N-1)
tic; Y2=ms_whiten_XXt(X); toc
Y2*Y2'/(N-1)
tic; Y3=ms_whiten_mscmd(X); toc
Y3*Y3'/(N-1)

var(Y1,[],2)'

max(abs(Y1(:)-Y2(:)))
max(abs(Y1(:)-Y3(:)))
end
