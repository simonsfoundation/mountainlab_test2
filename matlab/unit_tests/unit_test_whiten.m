function unit_test_whiten
M=4;
N=12;
X=zeros(M,N);
[ii,jj]=ndgrid(0:(M-1),0:(N-1));
X=sin(sin(ii)+sin(jj)+sin(ii+jj));
Y=ms_whiten(X);
disp('Y:');
disp(Y);
end