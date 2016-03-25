function mscmd_whiten(input_path,output_path,opts)
%MSCMD_WHITEN - Channel-whiten an array of raw or preprocessed data
%
%This is a wrapper to the command-line mountainsort procedure. It has
%similar functionality to ms_whiten.
%
% Syntax:  ms_whiten(input_path,output_path,opts)
%
% Inputs:
%    input_path - path to MxN array of raw or pre-processed data
%    output_path - path to MxN output array of whitened data
%
% Outputs:
%    Y - MxN array of channel-whitened data
%
% Other m-files required: none
%
% See also: mscmd_whiten, spikespy

% Author: Jeremy Magland
% Jan 2015; Last revision: 13-Feb-2106

%if (nargin<1) test_mscmd_whiten; return; end; % test is no longer valid

if (nargin<3) opts=struct; end;

cmd=sprintf('%s whiten --timeseries=%s --timeseries_out=%s ',mscmd_exe,input_path,output_path);

fprintf('\n*** WHITEN ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end

function test_mscmd_whiten_old

close all;

path0=fileparts(mfilename('fullpath'));
path1=sprintf('%s/test_in.mda',path0);
path2=sprintf('%s/test_out.mda',path0);

M=30;
N=10000;
opts.ncomp=5;
X=randn(M,N);
comps=randn(opts.ncomp,N);
X2=X+randn(M,opts.ncomp)*comps;

tic;
writemda(X2,path1);
mscmd_whiten(path1,path2,opts);
Y=readmda(path2);
toc

figure;
subplot(1,4,1);
plot(X(:),Y(:),'b.');
subplot(1,4,2);
plot(X2(:),Y(:),'r.');

tic;
comps=pca_components(X2);
Y2=X2-comps(:,1:opts.ncomp)*comps(:,1:opts.ncomp)'*X2;
toc

subplot(1,4,3);
plot(X(:),Y2(:),'g.');
subplot(1,4,4);
plot(X2(:),Y2(:),'k.');

end

function U=pca_components(X)
[U,D] = eig(X*X');   % takes O(MM^2*(MM+Ns)). Note eig faster than svd.
[d,I] = sort(diag(D),'descend');
U = U(:,I);  % sort eigenvectors
%z = U'*X;   % get all components in O(MM^2*Ns).   sing vals = sqrt(diag(D))
end

function [z U] = features_pca(X)
[M Nt Ns] = size(X);           % Get some dimensions
MM=M*Nt; X=reshape(X,MM,Ns);   % collapse channel and time dimensions
[U,D] = eig(X*X');   % takes O(MM^2*(MM+Ns)). Note eig faster than svd.
[d,I] = sort(diag(D),'descend'); U = U(:,I);  % sort eigenvectors
U = bsxfun(@times, U, std_signs(U));   % std signs of col vecs
z = U'*X;   % get all components in O(MM^2*Ns).   sing vals = sqrt(diag(D))
% sqrt(d(1:10)), U(1:10,1)   % few singular values & 1st left vec

end
