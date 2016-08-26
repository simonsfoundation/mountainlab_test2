function synthesize_timeseries_001(timeseries_path,firings_true_path,waveforms_path)

run_mountainlab_setup;

show_figures=0;

%rng(1); %not available in octave

M=5;
T=800;
K=20;
N=5e7;
noise_level=1;
synth_opts.upsamplefac=13;
refr=10;

samplerate=30000;
firing_rates=2*ones(1,K);
amp_variations=ones(2,K); amp_variations(1,:)=0.9; amp_variations(2,:)=1.1; %amplitude variation

opts.geom_spread_coef1=0.4;
opts.upsamplefac=synth_opts.upsamplefac;
waveforms=synthesize_random_waveforms(M,T,K,opts);

% force equal on all channels
% for k=K:K
%     waveforms(1,:,k)=waveforms(end,:,k);
%     waveforms(2,:,k)=waveforms(end,:,k);
%     waveforms(3,:,k)=waveforms(end,:,k);
%     waveforms(4,:,k)=waveforms(end,:,k);
%     waveforms(5,:,k)=waveforms(end,:,k);
% end;

if (show_figures)
    figure; ms_view_templates(waveforms);
end;

% events/sec * sec/timepoint * N
populations=ceil(firing_rates/samplerate*N);
times=zeros(1,0);
labels=zeros(1,0);
ampls=zeros(1,0);
for k=1:K
    refr_timepoints=refr/1000*samplerate;
    
%     a=refr_timepoints;
%     b=2*N/populations(k)-a; %(a+b)/2=N/pop so b=2*N/pop-a
%     isi=rand_uniform(a,b,[1,populations(k)*2]); %x2 to be safe
%     times0=cumsum(isi);
%     times0=times0((times0>=1)&(times0<=N));
    
    times0=rand(1,populations(k))*(N-1)+1;
    times0=[times0,times0+rand_distr2(refr_timepoints,refr_timepoints*20,size(times0))];
    times0=times0(randsample(length(times0),ceil(length(times0)/2)));
    times0=enforce_refractory_period(times0,refr_timepoints);
    times0=times0((times0>=1)&(times0<=N));
    
    times=[times,times0];
    labels=[labels,k*ones(size(times0))];
    amp1=amp_variations(1,k);
    amp2=amp_variations(2,k);
    ampls=[ampls,rand_uniform(amp1,amp2,size(times0)).*ones(size(times0))];
end;

firings_true=zeros(3,length(times));
firings_true(2,:)=times;
firings_true(3,:)=labels;

X=synthesize_timeseries(waveforms,N,times,labels,ampls,synth_opts);
X=X+randn(size(X))*noise_level;

if (~isempty(timeseries_path))
    writemda32(X,timeseries_path);
end;
if (~isempty(firings_true_path))
    writemda64(firings_true,firings_true_path);
end;
if (~isempty(waveforms_path))
    disp(size(waveforms));
    disp(waveforms_path);
    writemda32(waveforms,waveforms_path);
end;

%mv.raw=pathify32(X);
%mv.samplerate=30000;
%mountainview(mv);

end

function X=rand_distr2(a,b,sz)
X=rand(sz);
X=a+(b-a)*X.^2;
end

function X=rand_uniform(a,b,sz)
X=rand(sz);
X=a+(b-a)*X;
end

function times0=enforce_refractory_period(times0,refr)
if (length(times0)==0) return; end;
times0=sort(times0);
done=0;
while ~done
    diffs=times0(2:end)-times0(1:end-1);
    diffs=[diffs,inf]; %hack to make sure we handle the last one
    inds0=find((diffs(1:end-1)<=refr)&(diffs(2:end)>refr)); %only first violator in every group
    if (length(inds0)>0)
        times0(inds0)=-1; %kind of a hack, what's the better way?
        times0=times0(times0>=0);
    else
        done=1;
    end;
end
end
