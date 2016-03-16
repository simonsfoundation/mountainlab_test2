function [FF subspace] = ms_event_features(clips,num_features,opts)
%MS_EVENT_FEATURES - Extract PCA features from an array of events
%
%Consider using mscmd_features
%
% Syntax:  [FF, subspace] = ms_event_features(clips,num_features,opts)
%
% Inputs:
%    clips - MxTxNC array of clips (see ms_extract_clips)
%    num_features - the number of PCA features to extract
%    opts - don't use any options for now, please
%
% Outputs:
%    FF - num_features x NC array of features
%    subspace - MxTxnum_features array of principal components
%
% Example:
%    clips=ms_extract_clips(X,times0,100);
%    FF=ms_event_features(clips,3);
%    labels=isosplit2(FF)
%    ms_view_clusters(FF,labels);
%
% Other m-files required: none
%
% See also: mscmd_features, ms_extract_clips, isosplit2, ms_view_clusters

% Author: Jeremy Magland and Alex Barnett
% Oct 2015; Last revision: 13-Feb-2016

if nargin<3, opts = []; end
if ~isfield(opts,'fmethod'), opts.fmethod='pca'; end  % default
[M T Ns] = size(clips);
tic;
% call requested alg...
if strcmp(opts.fmethod,'pca'), [FF U] = features_pca(clips);
  subspace = reshape(U,[size(clips,1),size(clips,2),size(U,2)]);
elseif strcmp(opts.fmethod,'pcachan')
  FF = zeros(M*T,Ns); U = zeros(M*T,M*T);
  for m=1:M                % PCA for each channel separately
    r = m+(0:T-1)*M;  % row indices to write to: all 1st dims together, etc.
    [FF(r,:) U(r,r)]= features_pca(clips(m,:,:));
  end
  subspace = reshape(U,[size(clips,1),size(clips,2),size(U,2)]);
elseif strcmp(opts.fmethod,'cen'), FF = features_spatial_centroid(clips,opts.d);
elseif strcmp(opts.fmethod,'raw')
  FF = reshape(permute(clips,[2 1 3]),[M*T Ns]); % matrix: events are cols
else, error('unknown fmethod in features!');
end
%fprintf('features done in %.3g s\n',toc)
%%%%%

FF=FF(1:num_features,:);
subspace=subspace(:,:,1:num_features);

function [z U] = features_pca(X)
% FEATURES_PCA - get principal component analysis feature vectors, for Ns large
% Jeremy's version using X X^T, faster for large Ns, but limits to 8 digits?
% Assumes that M*Nt < Ns otherwise it's slower than plain SVD on X.
% Tries to standardize signs of z.
% todo: investigate QR or LQ for highly fat matrix SVD case.

[M Nt Ns] = size(X);           % Get some dimensions
MM=M*Nt; X=reshape(X,MM,Ns);   % collapse channel and time dimensions
[U,D] = eig(X*X');   % takes O(MM^2*(MM+Ns)). Note eig faster than svd.
[d,I] = sort(diag(D),'descend'); U = U(:,I);  % sort eigenvectors
U = bsxfun(@times, U, std_signs(U));   % std signs of col vecs
z = U'*X;   % get all components in O(MM^2*Ns).   sing vals = sqrt(diag(D))
% sqrt(d(1:10)), U(1:10,1)   % few singular values & 1st left vec

function z = features_pca_crude(X)
% FEATURES_PCA_CRUDE - get principal component analysis feature vectors.
% Alex's version, plain SVD on A, slower for large Ns, full e_mach.
% Tries to standardize signs of z.
[M T Ns] = size(X);
X = reshape(permute(X,[2 1 3]),[M*T Ns]); % matrix: events are cols
% (the permute has no effect for PCA but is my standard way to unfold to matrix)
[U S V] = svd(X,'econ');     % O((M*T)^2 * Ns) ? not sure for a fat matrix
S = bsxfun(@times, S, std_signs(U));   % std signs of col vecs
z = S*V';  % silly, could use repmat to make O(M*T*Ns)

function s = std_signs(U)
% standardized signs from col vecs of U. Barnett 12/23/14
[m n] = size(U);
s = nan(1,n);
for j=1:n
  [~,i] = max(abs(U(:,j)));   % index of max entry in jth col vec of U
  s(j) = sign(U(i,j));
end

function z = features_spatial_centroid(X,d)
% electrode location features, idea from Prentice et al 2011. Barnett 12/22/14
% Inputs:
%  X - usual 3D single-event array
%  d - raw EC data struct, for electrode loc info
% Outputs:
%  z - feature vectors as 3*Ns array
[M T Ns] = size(X);
z = nan(3,Ns);  % allocate output
for i=1:Ns         % loop over events...
  w = max(-X(:,:,i),[],2);  % weight by negative voltage peak - Prentice
  z(3,i) = max(w);          % 3rd coord is max peak height
  w = w(:)/sum(w);        % normalized weights col vec
  z(1:2,i) = d.electrodelocs * w;  % weighted spatial electrode locs in xy
end


