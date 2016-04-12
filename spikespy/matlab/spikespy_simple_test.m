function H=spikespy_simple_test(M,N,num_labels)

rng(1); %so we get the same array every time --> efficient loading in ssviewqt C++ program

if (nargin<1) M=6; end;
if (nargin<2) N=1e6; end;
if (nargin<3) num_labels=800; end;

X=rand(M,N);
tt=linspace(-1,1,N);
for ch=1:M
	X(ch,:)=X(ch,:).*sin(tt*2*pi*10*ch^2);
end;
tj=randsample(size(X,2),num_labels)';
lj=ceil(rand(size(tj))*size(X,1));

opts.sampling_freq=20000;
Htmp=spikespy({X,tj,lj,'This is the title.'},{X,tj,lj,'This is the title2.'},opts);

if (nargout>0) H=Htmp; end;

end