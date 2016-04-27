function [firings_out info]=ms_merge_across_channels(templates,firings,opts)
% MS_MERGE_ACROSS_CHANNELS - merge clusters from the same neuron but detected
%                            on different channels
%
% [firings_out info] = ms_merge_across_channels(templates,firings,opts)
%
% Inputs:
%    templates - MxTxK array of templates
%    firings - JxL array of firing events. channels in first row, times in the
%    second row, labels in the third row
%    opts - (optional) including
%        min_peak_ratio - the minimum ratio between the peaks on two channels in
%                         order to even consider merging
%        max_dt - the maximum time shift between peaks on different channels to
%                 consider for merging
%        min_coinc_frac - the minimum fraction of events that coincidently occur
%                 to consider for merging
%        min_coinc_num - the minimum number of events that coincidently occur to
%                 consider for merging
%        max_corr_stdev - the maximum standard deviation in cross-correlation
%                 distribution (in samples)
%        min_template_corr_coef - the minimum correlation coefficient between
%                 template waveforms to consider merging
%
% Outputs:
%    firings_out - the new array of firing events
%                 (labels and times will have been modified)
%    info - diagnostic struct containing fields:
%           Sinfo - K*K cell array of structs giving diagnostic info for all the
%                   decisions used for S matrix elements.
%           premergelabelchans - the peak channel numbers for each label
%                                pre-merge.
%
% Two clusters (detected on two distinct channels) are merged if ALL of the
% following criteria are met:
%    (Let W1,W2 be the templates and m1,m2 be the channels)
%    (a) The peak value of W1 on channel m2 is at least opts.min_peak_ratio
%        times its peak value on channel m1
%    (b) The correlation between waveforms W1 and W2 is at least
%        opts.min_template_corr_coef
%    (c) The number of coincident firing events is at least min_coinc_frac
%        times the lesser of the number of firing events and at least
%        min_coinc_num in number
%    (d) The standard deviation of the distibution of firing time offsets is
%        no more than opts.max_corr_stdev  (this ensures offsets repeatable).
%
% Other m-files required: none.
%
% Created 4/21/16 by ahb and jfm (simultaneously on etherpad).
% jfm debugged 4/22/16, ahb diagnostic output; max corr_coef over t-shift 4/25/16
% Todo: 0) self test
%       1) check bestdt altering in make_...
%       2) should maxdt in remove... be opts.max_dt ?

if nargin==0, test_ms_merge_across_channels; return; end
if nargin<3, opts=[]; end
if ~isfield(opts,'min_peak_ratio'), opts.min_peak_ratio = 0.5; end
if ~isfield(opts,'max_dt'), opts.max_dt = 10; end      % max difference between peaks of same event peak on different channels
if ~isfield(opts,'min_coinc_frac'), opts.min_coinc_frac = 0.1; end 
if ~isfield(opts,'max_corr_stddev'), opts.max_corr_stddev = 3; end     % in sample units
if ~isfield(opts,'min_template_corr_coef'), opts.min_template_corr_coef = 0.5; end    % waveform corr coeff
if ~isfield(opts,'min_coinc_num'), opts.min_coinc_num = 10; end

info = []; diagnose = (nargout>1);

firings=sort_by_time(firings);

peakchans=firings(1,:);    % must be same across all events with same label (since no merging done yet)
labels=firings(3,:);
times=firings(2,:);

K=max(labels);
S=zeros(K,K);  % score matrix between pairs
best_dt=zeros(K,K); % best time shifts between pairs

if diagnose, info.Sinfo = cell(K,K); % diagnostics output S cell array (ahb)
  info.premergelabelchans = nan(1,K);
  for k=1:K          % make a list of which peak chan each label had...
    inds=find(labels==k); if ~isempty(inds)
      info.premergelabelchans(k) = peakchans(inds(1));
    end
  end
end

fprintf('Merging across channels (compute score matrix)...\n');
for k1=1:K
  fprintf('.');
  inds1=find(labels==k1);
  for k2=[1:k1-1, k1+1:K]        % omit the diagonal
    inds2=find(labels==k2);
    if ((length(inds1)>0)&&(length(inds2)>0))
      peakchan1=peakchans(inds1(1));
      peakchan2=peakchans(inds2(1));
      if diagnose
        [S(k1,k2),best_dt(k1,k2),info.Sinfo{k1,k2}]=compute_score(squeeze(templates(:,:,k1)),squeeze(templates(:,:,k2)),times(inds1),times(inds2),peakchan1,peakchan2,opts);
      else
        [S(k1,k2),best_dt(k1,k2)]=compute_score(squeeze(templates(:,:,k1)),squeeze(templates(:,:,k2)),times(inds1),times(inds2),peakchan1,peakchan2,opts);   
      end
    end;
  end;
end;
fprintf('\n');

%make the matrix reflexive and transitive
[S,best_dt]=make_reflexive_and_transitive(S,best_dt);

%now we merge based on the above scores
new_labels=labels;
new_times=times;
for k1=1:K
for k2=k1+1:K
    if (S(k1,k2))
      %Now we merge
      inds_k2=find(new_labels==k2);
      new_labels(inds_k2)=k1;
      new_times(inds_k2)=times(inds_k2)-best_dt(k1,k2);  % check sign! (I think I got it right...., ahb please confirm)
    end;
end;
end;

%remove empty labels
new_labels=remove_unused_labels(new_labels);

firings_out=firings;
firings_out(3,:)=new_labels;
firings_out(2,:)=new_times;

%Now we will have a bunch of redundant events! So let's remove them!
maxdt=5;     % shouldn't this be opts.max_dt ?
firings_out=remove_redundant_events(firings_out,maxdt);

fprintf('Merge: %d -> %d clusters, %d -> %d events.\n',K,max(firings_out(3,:)),size(firings,2),size(firings_out,2));

end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function firings=sort_by_time(firings)
times=firings(2,:);
[~,sort_inds]=sort(times);
firings=firings(:,sort_inds);
end

function firings2=remove_redundant_events(firings,maxdt)

times=firings(2,:);
labels=firings(3,:);
L=size(firings,2);
K=max(labels);

to_use=ones(1,L);
for k=1:K
    inds_k=find(labels==k);
    times_k=times(inds_k);
    %the bad indices are those whose times occur too close to the previous times
    bad_inds=1+find(times_k(2:end)<=times_k(1:end-1)+maxdt); 
    to_use(inds_k(bad_inds))=0;
end;

inds_to_use=find(to_use);
firings2=firings(:,inds_to_use);

end

function [S,best_dt]=make_reflexive_and_transitive(S,best_dt)
% makes S(i,j) true whenever S(j,i)              for all i,j
% makes S(i,j) true whenever S(i,k) && S(k,j)    for all i,j,k
%
% ** ahb not sure of need for changing bestdt here?
K=size(S,1);
something_changed=1;
while something_changed
    something_changed=0;
    %reflexive
    for k1=1:K
    for k2=1:K
        if (S(k1,k2))&&(~S(k2,k1))
            something_changed=1;
            S(k2,k1)=S(k1,k2);
            best_dt(k2,k1)=-best_dt(k1,k2);
        end;
    end;
    end;
    %transitive
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


function [s bestdt info] = compute_score(template1,template2,t1,t2,peakchan1,...
                                         peakchan2,opts)
% returns merge score for two labels (using their firing events and templates),
% based on various criteria. Templates are each M*T.
% Outputs:  s is Boolean, true for merge.
%           bestdt is estimate of time offset between the merged events
%           info is a struct giving diagnostic info about decisions made

s = 0; bestdt = nan; info = []; info.s = 0;    % values to return if no merge

template1selfpeak = max(abs(template1(peakchan1,:)));         % bipolar for now
template1peakonc2 = max(abs(template1(peakchan2,:)));
% drop out if peaks not close enough in size...
if template1peakonc2  < opts.min_peak_ratio*template1selfpeak
  return
end
info.peakratio = template1peakonc2/template1selfpeak;
% drop out if not correlated enough...
r12 = best_corr_coef(template1,template2);
info.r12 = r12;
if r12 < opts.min_template_corr_coef
  return
end

if (numel(t1)==0)||(numel(t2)==0)
  info.reason='a time list was empty'; return
end

% compute firing cross-corr
cco=[]; cco.dtau=1; cco.taumax = opts.max_dt;
[C taus] = crosscorr_local([1+0*t1, 2+0*t2],[t1,t2],[],cco);    
C = squeeze(C(2,1,:))';  % hack to get the t1 vs t2 corr. row vec
coincfrac = sum(C)/min(numel(t1),numel(t2));
meanC = sum(taus.*C) / sum(C);
stddevC = sqrt(sum((taus-meanC).^2.*C)/sum(C));
s = coincfrac > opts.min_coinc_frac && sum(C)>=opts.min_coinc_num ...
    && stddevC < opts.max_corr_stddev;
bestdt = meanC;
% diagnostic output...
info.s = s; info.coincfrac = coincfrac; info.sumC = sum(C); info.stddevC=stddevC;
end

function bestr = best_corr_coef(w1,w2,o)
% w1, w2 are templates of size M*T.  naive version, ahb 4/25/16
if nargin<3, o = []; end
% note incr maxdt shrinks available time window for cross corr computation:
if isfield(o,'maxdt'), maxdt = o.maxdt; else maxdt = 10; end
[M T] = size(w1);
t = 1+maxdt:T-maxdt;   % time points to compare on unshifted template
if numel(t)<10, warning('less than 10 time points used in corr coef calc!'); end
v1 = w1(:,t); nv1 = norm(v1(:));
dts = -maxdt:maxdt;  % shifts
rs = nan*dts;
for i=1:numel(dts)
  v2 = w2(:,t+dts(i));  % shifted
  rs(i) = dot(v1(:),v2(:))/(nv1*norm(v2(:)));
end
bestr = max(rs);
end


% ----------------------------------------------------------------------------
% jfm says: I put this here locally so we don't have a file dependency.
% taken from mountainlab_devel/view/crosscorr.m
function [C taus] = crosscorr_local(l,t,a,o)
% CROSSCORR - estimate cross-correlation vs time shift given times and labels
%
% [C taus] = crosscorr(l,t)
% [C taus] = crosscorr(l,t,a)
% [C taus] = crosscorr(l,t,a,opts)
%  computes K-by-K cross-correlation matrix dependent on time separation tau.
%  The mean is not subtracted, so C_{ij}(tau) has non-negative entries.
%
% Inputs:
%  l - 1D array of labels
%  t - 1D array of firing times
%  a - 1D array of firing amplitudes (set to 1 if empty or absent)
%  opts controls various options:
%   opts.dtau, taumax = tau time bin width and maximum tau, in sample units.
%   dtau should be odd
% Outputs:
%  C - K*K*Nt matrix of cross-correlations (K = max(l), Nt = # timeshift bins)
%  tau - list of timeshift bins
%
% Uses quantization onto sample time grid of spacing 1, in order to get O(N)
% scaling, where N is number of spikes.
% (a pre-sort and binary search could get O(N log N) with more bookkeeping)
%
% todo: rewrite meth=d in C as w/ mda i/o
%
% todo: another meth using K^2 FFTs and inner prods?
% or K FFTs and (ntau)*K^2 inner prods w/ exps (ie slow FT)?
%
% Barnett 3/1/15. docs & default ampls, 4/7/15. new meth=d 4/8/15

if nargin<1, show_crosscorr; return; end
if nargin<3, a = []; end
if isempty(a), a = 0*t+1; end % default ampls
if nargin<4, o = []; end
if ~isfield(o,'dtau'), o.dtau = 20; end
if ~isfield(o,'taumax'), o.taumax = 500; end   % ie 50 bins
taus = 0:o.dtau:o.taumax; taus = [-taus(end:-1:2) taus]; % tau bin centers
taue = [taus-o.dtau/2 taus(end)+o.dtau/2];  % tau bin edges
tend=taus(end)+o.dtau/2; % farthest end of tau bins
ntau = numel(taus); n = (ntau-1)/2;  % # rel time bins, max integer bin #

K = max(l);  % # types
i = l>0; l=l(i); t=t(i); a=a(i); % kill nonpositive labels
[t,i] = sort(t-min(t)+1); l = l(i); a = a(i);  % shift and sort t values
C = zeros(K,K,ntau);
shs = round((1.5-o.dtau)/2:(o.dtau-0.5)/2); % sample shifts for central bin
T = round(max(t)); N = numel(t);  % now make signal array (allow overlaps!)...

meth='d';

if meth=='a'  % version via making discrete A signal array
  %fprintf('creating quantized array, size %.3gGB...\n',K*T*8/1e9)
  LA = zeros(K,T); for j=1:N, LA(l(j),round(t(j)))=LA(l(j),round(t(j)))+a(j); end
  %LA = bsxfun(@minus,LA,mean(LA,2));  % make zero-mean - decided against
  %fprintf('computing C... ')
  for j=1:N, %if mod(j,round(N/10))==0, fprintf('%d%% ',round(100*j/N)); end
    tj = t(j); lj = l(j);
    for d=-n:n
      i = round(tj)+d*o.dtau+shs;
      if min(i)>0 & max(i)<=T  % haven't fallen off end of LA
        a = sum(LA(:,i),2);   % col vec
        if d==0, a(lj) = a(lj) - 1; end    % remove self spike
        C(:,lj,d+n+1) = C(:,lj,d+n+1) + a;
      end
    end
  end
  
elseif meth=='d'  % work directly w/ t,l: faster, 1 min 50bins N=1e6 K=10, 1core
  %fprintf('computing C... ')
  j0=1; while t(j0)<tend, j0=j0+1; end % get range 
  j1=N; while t(j1)>T-tend, j1=j1-1; end
  jm = 1;        % start index of events to include rel to jth event
  jp = j0; while t(jp)<2*tend, jp=jp+1; end % end index of same
  for j=j0:j1, %if mod(j,round(N/10))==0, fprintf('%d%% ',round(100*j/N)); end
    tj = t(j); lj = l(j);
    while t(jm)<tj-tend, jm=jm+1; end  % update [jm,jp] index range to include...
    while t(jp)<tj+tend, jp=jp+1; end
    i = [jm:j-1, j+1:jp];  % index list in window, omit the current spike j
    tir = t(i)-tj; li = l(i); % times and labels in time window around tj    
    for k=lj:K  % only do lower-tri part since C tau-rev anti-symm
      lik = li==k; if sum(lik)>0
        c = histc(tir(lik),taue); c=c(:);  % last entry from histc is zero
        C(k,lj,:) = squeeze(C(k,lj,:)) + a(j)*c(1:end-1); % annoying sizes
      end
    end
    %ii=(li>=lj);
    %if numel(i)>0  % lower-tri part of C, more vectorized than above
    %  L = zeros(numel(i),K-lj+1); L(1:numel(i),
    %  c = histc(L,taue); c = c(1:end-1,:)'; % exploit histc works on all cols
    %  C(lj:k,lj,:) = C(lj:k,lj,:) + a(j)*c;
    %end
  end
end
%fprintf('\n')
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% self-test
function test_ms_merge_across_channels   % AHB  4/25/16
dt = 6; % two waveforms with peaks separated by dt on two different channels
T = 50;
M=2;
K=2;
templates = nan(M,T,K);
Tcen = floor((T+1)/2);  % defn of center time of clip
w0=3;   % Gaussian width in samples (w0=3 makes r<0.5 for dt>=5)
t = (1:T)-Tcen;
templates(:,:,1) = [1.1*exp(-.5*t.^2/w0^2); 0.9*exp(-.5*(t-dt).^2/w0^2)];
templates(:,:,2) = [0.9*exp(-.5*(t+dt).^2/w0^2); 1.1*exp(-.5*t.^2/w0^2)]; % similar peak heights but shifted
%o = []; o.showcenter = 1; ms_view_templates(templates,o);  % to see them
N=1e7;   % duration
L = 1e4;  % how many true firings
truetimes = round(N*rand(1,L));
probdetected = 0.8;         % fake events found on two channels...
times1 = truetimes(rand(1,L)<probdetected); labels1 = 1+0*times1;
times2 = truetimes(rand(1,L)<probdetected) + dt; labels2 = 2+0*times2;
times = [times1,times2]; labels = [labels1,labels2]; peakchans = labels;
firings = [peakchans; times; labels];
o = [];
[f info]=ms_merge_across_channels(templates,firings,o);
fprintf('%d events found (should be close to expected %d)\n,',size(f,2),round((1-(1-probdetected)^2)*L))
%diagnose_merge(info);
info.Sinfo{1,2}
fprintf('correlation coeff r found: %.3g\n',info.Sinfo{1,2}.r12)
end
