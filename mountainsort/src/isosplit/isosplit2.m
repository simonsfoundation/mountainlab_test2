function [labels,info]=isosplit2(X,opts)

if (nargin<1)
    test_isosplit2;
	return;
end;

if (isstr(X))&&(strcmp(X,'test'))
    run_misc_tests;
    return;
end;

if (nargin<2)
    opts=struct;
end;

if (~isfield(opts,'return_iterations'))
    opts.return_iterations=0;
end
if (opts.return_iterations)
    info.iterations={};
end;

if (~isfield(opts,'isocut_threshold')) opts.isocut_threshold=1.5; end;
if (~isfield(opts,'K_init')) opts.K_init=25; end;
if (isfield(opts,'K')) opts.K_init=opts.K; end;
if (~isfield(opts,'verbose')) opts.verbose=0; end;
if (~isfield(opts,'verbose3')) opts.verbose3=0; end;
if (~isfield(opts,'whiten_at_each_comparison')) opts.whiten_at_each_comparison=1; end;
if (~isfield(opts,'repeat_tolerance')) opts.repeat_tolerance=0.2; end;

if numel(size(X))~=2, error('X must be a 2D array'); end
[M,N]=size(X);
compile_isocut_if_needed;

info.num_iterations=0;

if N==0
    labels=[];
    return;
end;

labels_init=local_kmeans_sorber(X,opts.K_init);
labels=labels_init;
active_labels=ones(1,opts.K_init);

centers=zeros(M,opts.K_init);
counts=zeros(1,opts.K_init);
for k=1:opts.K_init
    inds_k=find(labels==k);
    centers(:,k)=compute_cluster_center(X(:,inds_k));
    counts(k)=length(inds_k);
end;

attempted_comparisons.centers1=zeros(M,0);
attempted_comparisons.centers2=zeros(M,0);
attempted_comparisons.counts1=[];
attempted_comparisons.counts2=[];

while 1
    info.num_iterations=info.num_iterations+1;
    [k1,k2]=find_next_comparison(active_labels,centers,counts,attempted_comparisons,opts.repeat_tolerance);
    if (k1<0) break; end;
    if (opts.return_iterations)
        iteration_info=struct;
        iteration_info.labels_before=labels;
        iteration_info.k1=k1;
        iteration_info.k2=k2;
    end;
    if (opts.verbose)
        fprintf('Iteration %d: Comparing %d (%d) with %d (%d)...',info.num_iterations,k1,counts(k1),k2,counts(k2));
    end;
    inds1=find(labels==k1);
    inds2=find(labels==k2);
    inds12=[inds1,inds2];
    attempted_comparisons.centers1=cat(2,attempted_comparisons.centers1,centers(:,k1));
    attempted_comparisons.centers2=cat(2,attempted_comparisons.centers2,centers(:,k2));
    attempted_comparisons.counts1(end+1)=length(inds1);
    attempted_comparisons.counts2(end+1)=length(inds2);
    attempted_comparisons.centers1=cat(2,attempted_comparisons.centers1,centers(:,k2));
    attempted_comparisons.centers2=cat(2,attempted_comparisons.centers2,centers(:,k1));
    attempted_comparisons.counts1(end+1)=length(inds2);
    attempted_comparisons.counts2(end+1)=length(inds1);
    [do_merge,labels0,info0]=test_redistribute(X(:,inds1),X(:,inds2),opts);
    if (opts.return_iterations)
        iteration_info.projection=info0.projection;
        iteration_info.projection_cutpoint=info0.cutpoint;
        iteration_info.projection_labels=info0.labels;
    end
    if (do_merge)||(max(labels0)==1)
        if (opts.verbose)
            fprintf('Merging\n',k1,counts(k1),k2,counts(k2));
        end;
        labels(find(labels==k2))=k1;
        centers(:,k1)=compute_cluster_center(X(:,inds12));
        counts(k1)=length(inds12);
        counts(k2)=0;
        active_labels(k2)=0;
    else
        indsA=inds12(find(labels0==1));
        indsB=inds12(find(labels0==2));
        if (opts.verbose)
            fprintf('Redistributing (%d) (%d)\n',length(indsA),length(indsB));
        end;
        labels(indsA)=k1;
        labels(indsB)=k2;
        centers(:,k1)=compute_cluster_center(X(:,indsA));
        centers(:,k2)=compute_cluster_center(X(:,indsB));
        counts(k1)=length(indsA);
        counts(k2)=length(indsB);
    end;
    if (opts.return_iterations)
        iteration_info.labels=labels;
        info.iterations{end+1}=iteration_info;
    end
end;

labels_map=zeros(1,opts.K_init);
kk=1;
for j=1:opts.K_init
    if (active_labels(j))
        labels_map(j)=kk; kk=kk+1;
    end;
end;
labels=labels_map(labels);

end

function dists=all_dists_between_centers(centers)
[M,N]=size(centers);
dists=zeros(N,N);
for m=1:M
    [xxx,yyy]=ndgrid(centers(m,:),centers(m,:));
    dists=dists+(xxx-yyy).^2;
end;
dists=sqrt(dists);
end

function [k1,k2]=find_next_comparison(active_labels,centers,counts,attempted_comparisons,repeat_tolerance)
active_inds=find(active_labels);
centers_active=centers(:,active_inds);
counts_active=counts(active_inds);
Na=size(centers_active,2);
dists=all_dists_between_centers(centers_active);
for j=1:Na
    dists(j,j)=inf;
end
[~,ii]=sort(dists(:));
for j=1:length(ii)
    if (isinf(dists(ii(j))))
       k1=-1; k2=-1;
       return;
    end;
    [k1,k2]=ind2sub(size(dists),ii(j));
    if ((counts(active_inds(k1))>0)&&(counts(active_inds(k2))>0)) % just to make sure (this was actually happening! probably should track down why)
        if (~was_already_attempted(attempted_comparisons,centers_active(:,k1),centers_active(:,k2),counts_active(k1),counts_active(k2),repeat_tolerance))
            k1=active_inds(k1);
            k2=active_inds(k2);
            return;
        end;
    end;
end;
k1=-1; k2=-1;
end

function ret=was_already_attempted(attempted_comparisons,center1,center2,count1,count2,repeat_tolerance)
AC=attempted_comparisons;
tol=repeat_tolerance;
ii=find( ...
    (abs(AC.counts1-count1)<=tol*sqrt((AC.counts1+count1)/2)) & ...
    (abs(AC.counts2-count2)<=tol*sqrt((AC.counts2+count2)/2)) ...
    );

AC_centers1=AC.centers1(:,ii);
AC_centers2=AC.centers2(:,ii);

dists0=sqrt( sum((AC_centers1-AC_centers2).^2,1) );
dists1=sqrt( sum((AC_centers1-repmat(center1,1,length(ii))).^2,1) );
dists2=sqrt( sum((AC_centers2-repmat(center2,1,length(ii))).^2,1) );
aa=find(dists0>0);
dists0=dists0(aa); dists1=dists1(aa); dists2=dists2(aa);

fracs1=dists1./dists0;
fracs2=dists2./dists0;

jj=find( (fracs1<=tol*1/sqrt(count1)) & (fracs2<=tol*1/sqrt(count2)) );

ret=~isempty(jj);
    
end

function [X1b,X2b,V]=whiten_two_clusters(X1,X2)
M=size(X1,1);
N1=size(X1,2);
N2=size(X2,2);

% Important to subtract the two centroids before whitening!
centroid1=mean(X1,2);
centroid2=mean(X2,2);
Y1=X1-repmat(centroid1,1,N1);
Y2=X2-repmat(centroid2,1,N2);

% Combine the data
Y=cat(2,Y1,Y2);
N=N1+N2;

% Obtain the whitening matrix using svd
if (N>=M)
    [U,D,V] = svd(Y,'econ');
    D(D~=0)=1./D(D~=0);
    % Amd apply it to the original (non-mean subtracted) data
    X1b=sqrt(N-1)*U*D(1:M,1:M)*(U'*X1);
    X2b=sqrt(N-1)*U*D(1:M,1:M)*(U'*X2);
else
    %too few points to whiten
    X1b=X1;
    X2b=X2;
end;

% The best direction is now the one connecting the centroids.
centroid1b=mean(X1b,2);
centroid2b=mean(X2b,2);
V=centroid2b-centroid1b;
end

function [do_merge,labels,info]=test_redistribute(X1,X2,opts)
if opts.whiten_at_each_comparison
    [X1,X2,V]=whiten_two_clusters(X1,X2);
else
    V=compute_cluster_center(X2)-compute_cluster_center(X1);
end;

if (sum(V.^2)==0)
	warning('isosplit: vector V is null.');
else
    V=V/sqrt(sum(V.^2));
end;
XX=V'*cat(2,X1,X2); %Project onto the line connecting the centroids
N=length(XX);
if (N<=5) %avoid a crash - 2/22/2016 jfm
    do_merge=1;
    labels=ones(1,N);
    return;
end;
XXs=sort(XX);
cutpoint=isocut(XXs,opts.isocut_threshold); %This is the core procedure -- split based on isotonic regression
if (cutpoint~=0)
	%It was a statistically significant split -- so let's redistribute!
	ii1=find(XX<=cutpoint);
	ii2=find(XX>cutpoint);
    do_merge=0;
else
	ii1=1:N;
	ii2=[];
    do_merge=1;
end;
labels=zeros(1,N);
labels(ii1)=1;
labels(ii2)=2;
info.projection=XX;
info.cutpoint=cutpoint;
info.labels=labels;

if (opts.verbose3)
    figure;
    bins=linspace(min(XX),max(XX),100);
    if (length(ii2)>0)
        histogram(XX(find(XX<=cutpoint)),bins,'FaceColor','b','EdgeColor','b'); hold on;
        histogram(XX(find(XX>cutpoint)),bins,'FaceColor','r','EdgeColor','r');
    else
        histogram(XX,bins,'FaceColor','k','EdgeColor','k');
    end;
end;

end

function center=compute_cluster_center(X)
center=ms_geometric_median(X);
end

function test_isosplit2

close all;

seed0=randi(10000);
seed0=8415;
rng(seed0);
centers={[0,0],[5,3.8],[-4.5,3]};
pops={1000,800,400};
shapes={[1,1,0],[2,1,0],[1,2,0]};
opts=struct;

fprintf('seed = %d\n',seed0);

samples=zeros(2,0);
true_labels=zeros(1,0);

for j=1:length(centers)
	xx=randn(1,pops{j});
	yy=randn(1,pops{j});
	shape=shapes{j};
	xx2=xx*shape(1)+yy*shape(3);
	yy2=yy*shape(2)-xx*shape(3);
	center=centers{j};
	xx2=xx2+center(1);
	yy2=yy2+center(2);
	tmp=zeros(2,pops{j});
	tmp(1,:)=xx2; tmp(2,:)=yy2;
	samples=[samples,tmp];
	true_labels=[true_labels,ones(1,pops{j})*j];
end;

figure;
ms_view_clusters(samples,true_labels);
drawnow;

[labels,info]=isosplit2(samples,opts);
fprintf('isosplit: num clusters = %d, num iterations = %d\n',max(labels),info.num_iterations);
figure;
ms_view_clusters(samples,labels); title('isosplit');
drawnow;

[labels,info]=isosplit2(samples,opts);
fprintf('isosplit2: num clusters = %d, num iterations = %d\n',max(labels),info.num_iterations);
figure;
ms_view_clusters(samples,labels); title('isosplit2');
drawnow;

end

function compile_isocut_if_needed
try
    isocut(1:10,1.2);
catch
    compile_mex_isocut;
end;
end

function [L,C]=local_kmeans_sorber(X,k)
%KMEANS Cluster multivariate data using the k-means++ algorithm.
%   [L,C] = kmeans(X,k) produces a 1-by-size(X,2) vector L with one class
%   label per column in X and a size(X,1)-by-k matrix C containing the
%   centers corresponding to each class.

%   Version: 2013-02-08
%   Authors: Laurent Sorber (Laurent.Sorber@cs.kuleuven.be)
%
%   References:
%   [1] J. B. MacQueen, "Some Methods for Classification and Analysis of 
%       MultiVariate Observations", in Proc. of the fifth Berkeley
%       Symposium on Mathematical Statistics and Probability, L. M. L. Cam
%       and J. Neyman, eds., vol. 1, UC Press, 1967, pp. 281-297.
%   [2] D. Arthur and S. Vassilvitskii, "k-means++: The Advantages of
%       Careful Seeding", Technical Report 2006-13, Stanford InfoLab, 2006.

L = [];
L1 = 0;

while length(unique(L)) ~= k
    
    % The k-means++ initialization.
    C = X(:,1+round(rand*(size(X,2)-1)));
    L = ones(1,size(X,2));
    for i = 2:k
        D = X-C(:,L);
%        D = cumsum(sqrt(dot(D,D,1)));  % orig, seems to be dist (l=1)
        D = cumsum(dot(D,D,1));  % Arthur-Vassilvitskii use dist^2 (l=2)
        if D(end) == 0, C(:,i:k) = X(:,ones(1,k-i+1)); return; end
        C(:,i) = X(:,find(rand < D/D(end),1));
        [~,L] = max(bsxfun(@minus,2*real(C'*X),dot(C,C,1).'));
    end
    
    % The k-means algorithm.
    while any(L ~= L1)
        L1 = L;
        for i = 1:k, l = L==i; C(:,i) = sum(X(:,l),2)/sum(l); end
        [~,L] = max(bsxfun(@minus,2*real(C'*X),dot(C,C,1).'),[],1);
    end
    
end

end

function run_misc_tests
M=4;
[A,B]=ndgrid((0:M-1),(0:M-1));
X1=sin(A+B)+sin(A.*B);
X2=cos(A+B)-cos(A.*B);
[X1b,X2b,V]=whiten_two_clusters(X1,X2);
V
end
