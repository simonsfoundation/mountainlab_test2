function unit_test_eigenvalue_decomposition
%Compare with
%mountainsort unit_test eigenvalue_decomposition
%MATLAB seems to be 20x faster !!??!!
for pass=1:2
    if pass==1, M=4;
    elseif pass==2, M=1000;
    end;
    X=zeros(M,M);
    [ii,jj]=ndgrid(0:(M-1),0:(M-1));
    X=sin(sin(ii)+sin(jj)+sin(ii+jj));
    tA=tic;
    [U,Sdiag]=eig(X);
    elapsed=toc(tA);
    if pass==1
        disp('Sdiag:');
        disp(Sdiag);
        disp('U:');
        disp(U);
        X2=U*Sdiag*U';
        disp('X:');
        disp(X);
        disp('X2:');
        disp(X2);
    elseif pass==2
        fprintf('Elapsed time for M=%d: %g\n',M,elapsed);
    end;
end