function times=ms_detect3(X,opts)
%MS_DETECT3 - Detect super-threshold events in a raw/preprocessed dataset
%
%Consider using mscmd_detect
%
% Syntax:  [times] = ms_detect3(X,opts)
%
% Inputs:
%    X - MxN array of raw or preprocessed data (M channels by N timepoints)
%    opts.detect_interval - minimum number of integer timepoints separating
%                           two detected events
%    opts.detect_threshold - detect events where the absolute value of the
%                            signal (on some channel) exceeds this
%                            threshold. (Note: not in std-dev units.)
%    opts.clip_size - this just avoids the very beginning and end of the
%                     timeseries data, with an interval clip_size/2.
%                     Makes sure we allow enough space to
%                     later extract a clip of size clip_size.
%    opts.meth - (optional) string giving method for sub-sample alignment:
%                'x' - plane upsampled signal peak
%                'p' - peak from projection to PCA space (default)
%    opts.beta - (optional, integer) sub-sampling factor for real-valued time
%                estimation.
%    opts.Tsub - clip size used for sub-sample time estimation
%    opts.polarity - 'b' (both + and -), 'p' (+ only), 'm' (- only)
%                controls which signs of peaks used. Note: the mean must be
%                close to zero for all of these to be meaningful.
%
% Outputs:
%    times - 1xL array of real-valued timepoints where an event has been
%            detected.
%
% Example:
%    times=ms_detect3(X,struct('detect_threshold',5,'detect_interval',15,'clip_size',100));
%    clips=ms_extract_clips(X,times0,100);
%    spikespy(clips);
%
% Notes: the definition of a detected event is a an above-threshold signal
%  that is also the maximum in the interval detect_interval either side of the
%  event. Eg, if there is a growing sequence of spikes separated by less than
%  detect_interval, only the last one registers as an event.
%  This differs from detection in validspike.
%
% Other m-files required: none
%
% See also: mscmd_detect, ms_extract_clips, spikespy, test_detect_accuracy

% based on ms_detect, Jeremy Magland Jan 2016. Alex Barnett 3/11/16-3/16/16
% 3/21/16: polarity options (ahb)

if nargin==0, test_ms_detect3; return; end

detect_interval=opts.detect_interval;
detect_threshold=opts.detect_threshold;
T = opts.clip_size;
if isfield(opts,'beta'), beta = opts.beta; else, beta = 10; end
if ~isfield(opts,'meth'), opts.meth = 'p'; end                  % default
if ~isfield(opts,'polarity'), opts.polarity = 'b'; end          % default

% create absX which is a "detection signal" restricted to certain polarities...
if strcmp(opts.polarity,'b')
  absX=abs(X);
elseif strcmp(opts.polarity,'p')
  absX = max(0,X);
elseif strcmp(opts.polarity,'m')
  absX = max(0,-X);
else, error('unknown opts.polarity!');
end
absX=max(absX,[],1); % now max over channels (but best to put in one channel at a time)

[M,N] = size(X);
use_it=zeros(1,N);
best_ind=1;         % index of max within previous detect_interval timepts
best_abs_val=absX(1);
endskip = ceil(T/2);   % ahb
candidates=find((absX>=detect_threshold)&((1:N)>endskip)&((1:N)<=N-endskip));
for tt=candidates   % loop over indices of above-thresh values
    if (best_ind<tt-detect_interval)   % no interference from a prev event...
        [~,best_ind_rel]=max(absX(tt-detect_interval:tt-1));
        best_ind=best_ind_rel+tt-detect_interval-1; % convert to global index
        best_abs_val=absX(best_ind);
    end;
    if (absX(tt)>best_abs_val)
        use_it(tt)=1;
        use_it(best_ind)=0;
        best_ind=tt;
        best_abs_val=absX(tt);
    end;
end;
times=find(use_it==1);

% the above code was integer t, from ms_detect. Now do sub-sample adjustments...
if beta>1
  NC = numel(times);
  maxjit = beta;  % maximum peak move allowed, in upsampled timepoints, small
  % (otherwise it can jump to another peak entirely)
  if ~isfield(opts,'Tsub')                    % default
    if strcmp(opts.meth,'x'), opts.Tsub = 5;  % v small clips for interp
    else, opts.Tsub = 10; end
  end
  Tcen = floor((beta*opts.Tsub+1)/2);
  Z = ms_extract_clips2(X,times,opts.Tsub,beta); %figure; plot(squeeze(Z));
  if strcmp(opts.meth,'p')      % PCA on upsampled clips, smoothing of that
    if ~isfield(opts,'num_features'), opts.num_features = 15; end % beats 10
    Tu = size(Z,2);
    [FF, subspace] = ms_event_features(Z,opts.num_features);  % PCA
    % now make Z a smoothed version by summing feature vectors * their coeffs...
    Z = reshape(reshape(subspace,[M*Tu opts.num_features])*FF, [M Tu NC]);
    maxjit = 2.0*beta;           % allow larger jitter adjustment (make param)
  end
  % create a detection signal vs time for each clip...
  if strcmp(opts.polarity,'b')
    Zdet = max(abs(Z),[],1);   % collapse along channel axis
  elseif strcmp(opts.polarity,'p')
    Zdet = max(max(0,Z),[],1);   % collapse along channel axis
  else
    Zdet = max(max(0,-Z),[],1);   % collapse along channel axis
  end  
  for i=1:NC    % now find location of max subsampled det signal...
    [~,ind] = max(abs(Zdet(:,i)));   % or don't use upsampled here?
    jit = max(-maxjit,min(maxjit, ind-Tcen));   % limit |jitter| to maxjit
    times(i) = times(i) + jit/beta;   % jitter by # subsamples
  end
end
%%%%%%%%%%%%%%%%%%%%%%%%%


function test_ms_detect3
% single-channel test...
rng(1); X=[rand(1,150)*20,rand(1,150)*10,rand(1,150)*20];  % white noise data

opts.detect_threshold=10;
opts.detect_interval=5;
opts.clip_size=50;
betas = [1 3 10];           % sub-sampling factors (1: plain integer times)
figure; set(gcf,'position',[100 1000 2000 500]);  % nice wide figure
for i=1:numel(betas)
  opts.beta = betas(i);
  times=ms_detect3(X,opts)
  subplot(numel(betas),1,i); plot(1:length(X),X,'k'); hold on;
  for j=1:length(times), plot(times(j)*[1 1],ylim,'r'); end   % times as vlines
  title(sprintf('detection on white noise signal: \\beta = %d\n',opts.beta))
end
