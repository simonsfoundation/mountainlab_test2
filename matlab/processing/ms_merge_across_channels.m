function firings_out=ms_merge_across_channels(templates,firings,opts)
% MS_MERGE_ACROSS_CHANNELS - merge clusters corresponding to the same neuron but detected on different channels
%
% [firings_out] = ms_merge_across_channels(templates,firings,opts)
%
% Inputs:
%    templates - MxTxK array of templates
%    firings - JxL array of firing events. channels in first row, times in the second row, labels in the third row
%    opts - (optional) including
%        min_peak_ratio - the minimum ratio between the peaks on two channels in order to even consider merging
%        max_dt - the maximum time shift between peaks on different channels to consider for merging
%        min_coinc_frac - the minimum fraction of events that coincidently occur to consider for merging
%        min_coinc_num - the minimum number of events that coincidently occur to consider for merging
%        max_corr_stdev - (ahb will describe this)
%        min_template_corr_coef - the minimum correlation between template waveforms to consider merging
%        
%
% Outputs:
%    firings_out - the new array of firing events (labels and times will have been modified)
%
% Two clusters (detected on two distinct channels) are merged if ALL of the following criteria are met:
%    (Let W1,W2 be the templates and m1,m2 be the channels)
%    (a) The peak value of W1 on channel m2 is at least opts.min_peak_ratio times its peak value on channel m1
%    (b) The correlation between waveforms W1 and W2 is at least opts.min_template_corr_coef
%    (c) The number of coincident firing events is at least min_coinc_frac times [ahb will describe this]
%
% This procedure is forced to be 
%
% Other m-files required:
%
% Created 4/21/16 by ahb and jfm (simultaneously on etherpad)

if nargin<3, opts=[]; end
if ~isfield(opts,'min_peak_ratio'), opts.min_peak_ratio = 0.7; end
if ~isfield(opts,'max_dt'), opts.max_dt = 10; end      % max difference between peaks of same event peak on different channels
if ~isfield(opts,'min_coinc_frac'), opts.min_coinc_frac = 0.1; end 
if ~isfield(opts,'max_corr_stddev'), opts.max_corr_stddev = 3; end     % in sample units
if ~isfield(opts,'min_template_corr_coef'), opts.min_template_corr_coef = 0.5; end    % waveform corr coeff
if ~isfield(opts,'min_coinc_num'), opts.min_coinc_num = 10; end

peakchans=firings(1,:);    % must be same across all events with same label (since no merging done yet)
labels=firings(3,:);
times = firings(2,:);

K=max(labels);

S=zeros(K,K);  % score matrix between pairs
best_dt=zeros(K,K); % best time shifts between pairs

for k1=1:K
for k2=1:K
    inds1=find(labels==k1);
    inds2=find(labels==k2);
    if ((length(inds1)>0)&&(length(inds2)>0))
      peakchan1=peakchans(inds1(1));
      peakchan2=peakchans(inds2(1));
      k1,k2
      [S(k1,k2),best_dt(k1,k2)]=compute_score(squeeze(templates(:,:,k1)),squeeze(templates(:,:,k2)),times(inds1),times(inds2),peakchan1,peakchan2,opts); % Boolean
    end;
end;
end;

S
best_dt    % should be antisymm

%make the matrix transitive
[S,best_dt]=make_transitive(S,best_dt)


%now we merge based on the above scores
new_labels=labels;
new_times=times;
for k1=1:K
for k2=k1+1:K
    if (S(k1,k2))
      %Now we merge
      inds_k2=find(labels==k2);
      new_labels(inds_k2)=k1;
      new_times(inds_k2)=times(inds_k2)+best_dt(k1,k2);  % check sign!
    end;
end;
end;

%keyboard

%remove empty labels
%new_labels=remove_unused_labels(new_labels);

firings_out=firings;
firings_out(3,:)=new_labels;
firings_out(2,:)=new_times;

end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




function [S,best_dt]=make_transitive(S,best_dt)
% makes S(i,j) <= S(i,k) && S(k,j)    for all i,j,k
K=size(S,1);
something_changed=1;
while something_changed
    something_changed=0;
    for k1=1:K
    for k2=1:K
    for k3=1:K
    if (S(k1,k2))&&(S(k2,k3))&&(~S(k1,k3))
    something_changed=1;
    S(k1,k3)=(S(k1,k2)+S(k2,k3))/2;
    best_dt(k1,k3)=best_dt(k1,k2)+best_dt(k2,k3);
    end;
    end;
    end;
    end;
end;
end

function labels=remove_unused_labels(labels)
K=max(labels);
used_labels=zeros(1,K);
used_labels(labels)=1;
used_label_numbers=find(used_labels); %is there a better way to do this?
label_map=zeros(1,K);
for a=1:length(used_label_numbers)
    label_map(used_label_numbers(a))=a;
end;
labels=label_map(labels);
end


function [s bestdt]  = compute_score(template1,template2,t1,t2,peakchan1,peakchan2,opts)
% returns merge score for two labels (using their firing events and templates), based on various criteria

s = 0; bestdt = nan;     % values to return if no merge

template1selfpeak = max(abs(template1(peakchan1,:)));         % bipolar for now
template1peakonc2 = max(abs(template1(peakchan2,:)));
if template1peakonc2  < opts.min_peak_ratio*template1selfpeak           % drop out if peaks not close enough in size
  return
end
% drop out if not correlated enough...
w1 = template1(:); w2 = template2(:);
r12 = dot(w1,w2)/norm(w1)/norm(w2);
if r12 < opts.min_template_corr_coef
  return
end

if (numel(t1)==0)||(numel(t2)==0) return; end;

% compute firing cross-corr
cco=[]; cco.dtau=1; cco.taumax = opts.max_dt;
[C taus] = crosscorr([1+0*t1, 2+0*t2],[t1,t2],[],cco);    % from ms_devel/view/
C = squeeze(C(2,1,:))';  % hack to get the t1 vs t2 corr. row vec
% ** check why diag auto-corr is zero
C
coincfrac = sum(C)/min(numel(t1),numel(t2));
meanC = sum(taus.*C) / sum(C);
stddevC = sqrt(sum((taus-meanC).^2.*C)/sum(C));
s = coincfrac > opts.min_coinc_frac && sum(C)>=opts.min_coinc_num  && stddevC < opts.max_corr_stddev;
bestdt = meanC;

end
