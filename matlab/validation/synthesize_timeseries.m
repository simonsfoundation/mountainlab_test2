function Y = synthesize_timeseries(W,N,times,labels,ampls,opts)
% SYNTHESIZE_TIMESERIES.  Make timeseries via linear addition of spikes forward model
%
% Y = synthesize_timeseries(W,T,times,labels,ampls,opts) outputs a synthetic time series
% given waveforms and firing information; ie, it applies the forward model.
% No noise is added.
%
% Inputs:
%  W - waveform (templates) array, M by T by K. (note: assumed not upsampled
%      unless opts.upsamplefac > 1).
%  N - integer number of time points to generate.
%  times - (1xL) list of firing times, integers or real,
%          where 1st output time is 1 and last is N (ie, sample index units).
%  labels - (1xL) list of integer labels in range 1 to K.
%  ampls - (optional, 1xL) list of firing amplitudes (if absent or empty,
%          taken as 1).
%  opts - (optional struct) controls various options such as
%         opts.upsamplefac : integer ratio between sampling rate of W and for
%                            the output timeseries.
% Outputs:
%  Y - (MxN, real-valued) timeseries

% Barnett 2/19/16 based on validspike/synthesis/spikemodel.m; 2/25/16 upsampled.
% name change on 3/18/16 (used to be ms_synthesize) - jfm
% todo: faster C executable acting on MDAs I/O.

if nargin==0, test_synthesize_timeseries; return; end
if nargin<5 || isempty(ampls), ampls = 1.0+0*times; end      % default ampl
if nargin<6, opts = []; end
if isfield(opts,'upsamplefac'), fac = opts.upsamplefac; else fac = 1; end
if fac~=round(fac), warning('opts.upsamplefac should be integer!'); end

[M T K] = size(W);
tcen = floor((T+1)/2);    % center firing time in waveform samples
ptsleft = tcen - 1; ptsright = T - tcen;
L = numel(times);
if numel(labels)~=L, error('times and labels must have same # elements!'); end
times = round(times*fac);    % times now in upsampled grid units
Y = zeros(M,N);
for j=1:L            % loop over spikes adding in each one
  iput = max(1,ceil((times(j)-ptsleft)/fac)):min(N,floor((times(j)+ptsright)/fac));  % indices to write to in output
  iget = tcen + fac*iput - times(j);   % inds to get from W; must be in 1 to T
  Y(:,iput) = Y(:,iput) + ampls(j)*W(:,iget,labels(j));
end

function test_synthesize_timeseries
for fac=[1 3]
  fprintf('upsamplefac = %d...\n',fac)
  % make simple variable-width Gaussian waveforms...
  M = 4;       % # channels
  T = 30*fac;  % # timepoints for waveform
  K = 5;       % # neuron types
  W = zeros(M,T,K);
  tcen = floor((T+1)/2);           % center index
  t = ((1:T) - tcen)/fac;          % offset time grid in output units
  for k=1:K
    wid = 2.0*exp(0.5*randn);         % Gaussian width in time
    pulse = exp(-0.5*t.^2/wid^2);
    W(:,:,k) = randn(M,1) * pulse;            % outer prod
  end
  % make firing info...
  N = 1e6;   % total time points
  L = 1e4;   % number of firings
  times = rand(1,L)*(N-1)+1;        % real-valued times
  labels = randi(K,1,L);
  tic
  Y = synthesize_timeseries(W,N,times,labels,[],struct('upsamplefac',fac));
  toc
  nam = sprintf('test ms_synthesize fac=%d',fac);
  spikespy({Y,round(times),labels,nam});    % only takes integer times
  figure; tmax=1e3; plot(Y(:,1:tmax)','.-'); hold on; vline(times(times<tmax));
  title(nam); xlabel('time in output samples');
end
