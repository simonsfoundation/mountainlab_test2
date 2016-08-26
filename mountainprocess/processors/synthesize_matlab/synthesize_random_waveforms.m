function waveforms=synthesize_random_waveforms(M,T,K,opts)

if (nargin<1) test_synthesize_random_waveforms; return; end;

if (nargin<4) opts=struct; end;
if (~isfield(opts,'geometry')) opts.geometry=zeros(2,M); opts.geometry(1,:)=1:M; end
if (~isfield(opts,'avg_durations')) opts.avg_durations=[200,10,30,200]; end;
if (~isfield(opts,'avg_amps')) opts.avg_amps=[0.5,10,-1,0]; end;
if (~isfield(opts,'rand_durations_stdev')) opts.rand_durations_stdev=[10,4,6,20]; end;
if (~isfield(opts,'rand_amps_stdev')) opts.rand_amps_stdev=[0.2,3,0.5,0]; end;
if (~isfield(opts,'rand_amp_factor_range')) opts.rand_amp_factor_range=[0.5,1]; end;
if (~isfield(opts,'geom_spread_coef1')) opts.geom_spread_coef1=0.2; end;
if (~isfield(opts,'geom_spread_coef2')) opts.geom_spread_coef2=1; end;
if (~isfield(opts,'average_peak_amplitude')) opts.average_peak_amplitude=10; end;
if (~isfield(opts,'upsamplefac')) opts.upsamplefac=1; end;
if (~isfield(opts,'timeshift_factor')) opts.timeshift_factor=3; end;

neuron_locations=get_default_neuron_locations(M,K,opts);

waveforms=zeros(M,T*opts.upsamplefac,K);
for k=1:K
    for m=1:M
        diff=neuron_locations(:,k)-opts.geometry(:,m);
        dist=sqrt(sum(diff.^2));
        durations0=max(ones(size(opts.avg_durations)),opts.avg_durations+randn(1,4).*opts.rand_durations_stdev)*opts.upsamplefac;
        amps0=opts.avg_amps+randn(1,4).*opts.rand_amps_stdev;
        waveform0=synthesize_single_waveform(T*opts.upsamplefac,durations0,amps0);
        waveform0=circshift(waveform0,[0,floor(opts.timeshift_factor*dist*opts.upsamplefac)]);
        waveform0=waveform0*rand_uniform(opts.rand_amp_factor_range(1),opts.rand_amp_factor_range(2),[1,1]);
        waveforms(m,:,k)=waveform0/(opts.geom_spread_coef1+dist*opts.geom_spread_coef2);
    end;
end;

peaks=max(max(abs(waveforms),[],1),[],2);
waveforms=waveforms/mean(peaks)*opts.average_peak_amplitude;

end

function test_synthesize_random_waveforms
M=5;
T=800;
K=20;
waveforms=synthesize_random_waveforms(M,T,K);
figure; ms_view_templates(waveforms);
end

function neuron_locations=get_default_neuron_locations(M,K,opts)
neuron_locations=zeros(size(opts.geometry,1),K);
for k=1:K
    if (K>1)
        ind=(k-1)/(K-1)*(M-1)+1;
        ind0=floor(ind);
        if (ind0==M)
            ind0=M-1;
            p=1;
        else
            p=ind-ind0;
        end;
        if (M>1)
            neuron_locations(:,k)=(1-p)*opts.geometry(:,ind0)+p*opts.geometry(:,ind0+1);
        else
            neuron_locations(:,k)=opts.geometry(:,1);
        end;
    else
        neuron_locations(:,m)=opts.geometry(:,1);
    end
end;
end

function X=rand_uniform(a,b,sz)
X=rand(sz);
X=a+(b-a)*X;
end