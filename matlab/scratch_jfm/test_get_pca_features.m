function test_get_pca_features
close all;
M=400;
N=1000;
tt=linspace(0,1,N);
X=randn(M,N);
X(1,:)=X(1,:)+sin(tt*2*pi);
X(2,:)=X(2,:)-sin(tt*2*pi);
X(3,:)=X(3,:)+sin(2*tt*2*pi);
X(3,:)=X(4,:)-sin(2*tt*2*pi);
X(3,:)=X(5,:)+3*sin(2*tt*2*pi);
FF1=method1(X);
figure; plot(FF1(1,:));
figure; plot(FF1(2,:));

end

function z=method1(X)
[U,D] = eig(X*X');   % takes O(MM^2*(MM+Ns)). Note eig faster than svd.
[d,I] = sort(diag(D),'descend'); U = U(:,I);  % sort eigenvectors
U = bsxfun(@times, U, std_signs(U));   % std signs of col vecs
z = U'*X;   % get all components in O(MM^2*Ns).   sing vals = sqrt(diag(D))
end

function s = std_signs(U)
% standardized signs from col vecs of U. Barnett 12/23/14
[m n] = size(U);
s = nan(1,n);
for j=1:n
  [~,i] = max(abs(U(:,j)));   % index of max entry in jth col vec of U
  s(j) = sign(U(i,j));
end
end