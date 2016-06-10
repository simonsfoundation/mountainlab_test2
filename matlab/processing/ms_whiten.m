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
% Without arguments, runs self test including comparison for mscmd_whiten
%
% Other m-files required: none
%
% See also: mscmd_whiten

% Author: Jeremy Magland & Alex Barnett
% Jan 2015; Last revision: 5/20/16

if nargin<1, test_ms_whiten; return; end
%we are not going to subtract the mean, actually
%mu = mean(X,2); 
%X = X-repmat(mu,1,size(X,2));
X = ms_whiten_XXt(X);            % choose the method
end

function X=ms_whiten_svd(X)
% unnecessarily slow; only use if need full 16 digit accuracy
[M,N]=size(X);
[U,D,V] = svd(X,'econ');
D(D~=0)=1./D(D~=0);
X=(sqrt(N-1)*U*D(1:M,1:M)*U')*X;  % ahb rearranged for speed
end

function X=ms_whiten_XXt(X)
% the right way to do it if only care about 8 digit acc.
[M,N]=size(X);
[V,D] = eig(X*X');  % ahb, since symm
D = diag(D); j = D>1e-8;    % ahb regularization
D(j)=1./sqrt(D(j));             % regularized Lambda^{-1/2}
%D = D*(M/sum(j));           % scale so unit variance when killed eigvals
X=(sqrt(N-1)*V*diag(D)*V')*X;   % ahb rearranged for speed
end

function X=ms_whiten_mscmd(X)
% interface to C++ version
writemda(X,[tempdir,'/X_tmp.mda']);
mscmd_whiten([tempdir,'/X_tmp.mda'],[tempdir,'/X_tmp_white.mda']);
X=readmda([tempdir,'/X_tmp_white.mda']);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function test_ms_whiten
N=1e6;
M=10;
X=rand(M,N);
%X*X'/(N-1)  % est of covar before whitening
disp('three methods, timing and closeness of covar to Id:')
tic; Y1=ms_whiten(X); toc
norm(Y1*Y1'/(N-1) - eye(M))
tic; Y2=ms_whiten_svd(X); toc
norm(Y2*Y2'/(N-1) - eye(M))
tic; Y3=ms_whiten_mscmd(X); toc
norm(Y3*Y3'/(N-1) - eye(M))

disp('channel mean square signals:')
mean(Y1.^2,2)

disp('method differences:')
max(abs(Y1(:)-Y2(:)))
max(abs(Y1(:)-Y3(:)))
end
