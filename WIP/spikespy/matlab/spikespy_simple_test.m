function H=spikespy_simple_test(M,N,num_labels)
% SPIKESPY_SIMPLE_TEST  call spikespy with random data as self test.
%
% H=spikespy_simple_test(M,N,num_labels), all inputs are optional
% (if not given, defaults used)

rng(1); %so we get the same array every time --> efficient loading

if (nargin<1) M=6; end;
if (nargin<2) N=1e6; end;
if (nargin<3) num_labels=800; end;

X=rand(M,N);
tt=linspace(-1,1,N);
for ch=1:M
	X(ch,:)=X(ch,:).*sin(tt*2*pi*10*ch^2);
end;
tj=randi(size(X,2),[1 num_labels 1]);  % removed dependence on stats toolbox
lj=ceil(rand(size(tj))*size(X,1));

opts.sampling_freq=20000;
Htmp=spikespy({X,tj,lj,'This is the title.'},{X,tj,lj,'This is the title2.'},opts);

if (nargout>0) H=Htmp; end;

end
