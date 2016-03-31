function [X tcens]=ms_extract_clips2(Y,times,T,beta,betaonesnap)
%MS_EXTRACT_CLIPS2 - Extract clips from raw/preprocessed data array centered
%                    at a collection of event times, possibly upsampling/shifting
%
%Consider using mscmd_extract_clips
%
% Syntax:  [X tcens] = ms_extract_clips2(Y,times,clip_size,beta,betaonesnap)
%
% For any output clip X(:,:,j), the "center" gridpoint is floor((beta.T+1)/2)
%
% If beta=1 and if all times are integer, a simple extraction of values is done.
% If beta=1 and some times are non-integer, resampling onto shifted grids is
%   done. By default this is via a new interpolation for each clip, which is
%   slow. If betaonesnap is set, this is used to upsample and pull out the
%   best downsampled data for each clip, which is fast. Time-shift accuracy is
%   then +-0.5/betaonesnap.
%
% If beta=2,3,... then times are rounded to their nearest integer.
%  Output grid has beta.T timepoints in it, and the "center" point has the above
%  definition and matches the nearest integer time.
%
% Windowed-sinc interpolation is used, although FFT could be faster in the end.
%
% Inputs:
%    Y - MxN array of raw or pre-processed data (M channels, N timepoints)
%    times - 1xNC array of integer timepoints (event times)
%    T - integer clip size, e.g. 100 (in original timepoints)
%    beta - (optional) integer upsampling factor (1 assumed if absent)
%    betaonesnap - (optional) integer upsampling factor used internally when
%                 beta=1 to do faster interpolation to shifted grid (default inf)
%                 A sensible value is 10.
%
% Outputs:
%    X - Mx(beta*T)xNC array of extracted clips
%    tcens - 1xNC time values of centers of extracted time grids in the original
%            1-offset time grid units (1:N). The jth output time grid is
%            therefore t = tcens + ((1:beta*T)-floor((beta*T+1)/2))/beta
%
% Example:
%    clips=ms_extract_clips2(X,times0,100);
%    FF=ms_event_features(clips,3);
%    labels=isosplit2(FF)
%    ms_view_clusters(FF,labels);
% See also the self-tests at bottom of m-file.
%
% Other m-files required: none
%
% See also: ms_extract_clips, mscmd_extract_clips, ms_event_features, spikespy

% Author: Jeremy Magland. started Jan 2016.
% Upsampling Alex Barnett 3/10/16-3/11/16

if nargin==0, test_ms_extract_clips2; return; end
[M,N]=size(Y);
C=length(times);
if nargin<4 || isempty(beta), beta=1; end          % default
if nargin<5, betaonesnap=inf; end
integertimes = (sum(times==round(times))==C);   % all times are integers
kerpars.Tf = 5;       % half-width of kernel support

if beta==1 && integertimes
  X = extract_int_clips(Y,times,T);   % simple, done
  tcens = times;
else
  tpad = kerpars.Tf-1;        % extra integer # timepts at each end  
  tcens = round(times);
  Z = extract_int_clips(Y,tcens,T+2*tpad);   % get wider clips to allow interp
  Tcenw = floor((T+1)/2) + tpad;           % center in the wider time list
  if beta>1                     % upsample
    Tcenup = floor((beta*T+1)/2);         % center in upsampled time coords
    t = Tcenw + ((1:beta*T)-Tcenup)/beta;  % out time grid rel to wider clip
    X = upsample(Z,t,kerpars);
  else           % beta=1 but times non-integer
    tf = times - tcens;      
    if isinf(betaonesnap)   % individual exact time-shifts (slow)
      X = zeros(M,T,C);
      for c=1:C             % slowness
        X(:,:,c) = upsample(Z(:,:,c),tf(c)+tpad+(1:T),kerpars);
      end
      tcens = times;
    else                 % extract to internal upsampled grid then pull out
      beta = betaonesnap;
      Tcenup = floor((beta*T+1)/2);         % center in upsampled time coords
      maxjit = floor(beta/2);               % maximum jitter in 1/beta units
      t = ((tpad+1)*beta-maxjit:(tpad+T)*beta+maxjit)/beta;
      Z = upsample(Z,t,kerpars);
      ti = round(beta*tf);                  % # upsamp t-pt offsets for all clips
      ti = max(-maxjit,min(maxjit,ti));     % in case rounding different
      inds = 1+maxjit+(0:T-1)*beta;         % indices in the list t if no jitter
      for c=1:C
        X(:,:,c) = Z(:,ti(c)+inds,c);
      end
      tcens = tcens + ti/beta;              % the actual grid centers used
    end
  end  
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function X = extract_int_clips(Y,times,T)
% integer case, pulled from ms_extract_clips, 3/10/16
[M,N]=size(Y);
C=length(times);
X=zeros(M,T,C);   % so that unwritten clips are 0
Tmid=floor((T+1)/2);
tt1=-Tmid+1; %changed by jfm on 2/26/2016 to conform with docs
tt2=tt1+T-1;
inds=find((times+tt1>=1)&(times+tt2<=N));
for ii=1:length(inds)
    j=inds(ii); %fixed by jfm 3/30/16
	X(:,:,j)=Y(:,times(j)+tt1:times(j)+tt2);
end;


function Y = upsample(X,t,kerpars)
% UPSAMPLE - interpolate rows of matrix onto possibly finer time grid
%
% Y = upsample(X,tout,kerpars) interpolates rows of matrix X assumed to be
%  sampled on time grid 1:T, into rows of matrix Y, which are sampled on the
%  time values tout. The values of tout should lie in [Tf, T-Tf+1],
%  where Tf is the kernel width given in kerpars.Tf. If kerpars is omitted,
%  a default halfwidth-5 interpolator is used.
%
% If X is a 3D array, the interpolation is done along the 2nd dimension, and
%  Y is also a 3D array.
%
% Inputs:
%   X = timeseries row vector, matrix, or 3D array.
%   tout = vector of output times
%   kerpars = kernel parameters struct with fields:
%             Tf = half-width of filter in units of sample timesteps
% Outputs:
%   Y = matrix with same 1st (and 3rd) dimensions as X, but size(Y,2)=numel(tout)

% based on validspike [Barnett 11/26/14. default kerpars 6/10/15. faster 7/20/15]
% Barnett 3/10/16

if nargin==0, test_upsample; return; end
if nargin<3, kerpars.Tf = 5; end

T = size(X,2);                              % # input time samples
dt = bsxfun(@minus, t, (1:T)');             % matrix of time differences
PT = kernel(dt, kerpars);                   % transpose interp matrix
if ndims(X)==2                              % row vec or regular matrix
  Y = X*PT;                                 % right-mult to interp rows of X
else                                        % 3D case; loop it seems fast...
  Ns = size(X,3);
  Y = nan(size(X,1),numel(t),Ns);
  M = size(X,1); X = reshape(permute(X,[1 3 2]), [M*Ns T]); % fast avoids loops
  Y = X*PT;                         % single matvec to interp all rows of X
  Y = ipermute(reshape(Y, [M Ns numel(t)]),[1 3 2]);
end
%%%%%%%%%%%%%%


function f = kernel(t,pars)
% KERNEL - evaluate interpolation kernel at set of ordinates t-t'
%
% f = kernel(t,pars) where t is an array of time displacements and pars
% a parameter struct with fields:
%   pars.Tf  - half-width of support of kernel in original (input) samples
%
% Hann-windowed sinc for now. Barnett 11/26/14
f = sin(pi*t)./(pi*t) .* cos((pi/2/pars.Tf)*t).^2; % compute even outside win
f(abs(t)>=pars.Tf) = 0;  % enforce compact support
f(t==0) = 1.0;           % fix nans in sinc at origin
%%%%%%%%%%%%%%

function test_kernel
pars.Tf = 7;
t = -10:0.1:10;
figure; plot(t,kernel(t,pars),'-'); hold on; t = -10:10; plot(t,t==0,'.');
title('interpolation kernel for Tf=7');


function test_upsample
% i)
test_kernel

% ii) accuracy for sine wave at close to Nyquist
Tfull = 40;   % # original samples
fac = 5;  % upsampling factor
Tf = 5; kerpars.Tf = Tf;   % kernel support half-width
tpad = Tf-1; T = Tfull-2*tpad;   % effective clip width
t = 1:Tfull;
Tcen = floor((Tfull+1)/2);
tcenout = floor((fac*T+1)/2);
tout = tpad:(1/fac):(Tfull-tpad+1);    % out to worst-case (1 more than allowed)
samplerate = 2e4;               % Nyquist is half this
f = 6e3; g = @(t) sin(1+2*pi*f*t/samplerate); % test sine wave (freq < Nyquist)
u = g(t);  % row vector
uout = upsample(u,tout,kerpars);
uerr = uout - g(tout);
figure; subplot(2,1,1);
plot(tout,g(tout),'r.-',tout,uout,'g.-',t,u,'k*'); xlabel('t');
title('test upsample');
subplot(2,1,2); semilogy(tout,abs(uerr),'+-'); xlabel('t'); ylabel('error');
Ns = numel(tout);
fprintf('rms error on output pts = %.3g\n',norm(uout-g(tout))/sqrt(Ns))
% should be merely algebraic decay eg 1/Tf^4 or something




%%%%%%%%%%%%%%%%%%%%%%%%%%%

function test_ms_extract_clips2
test_upsample

% plain integer case, beta=1 ...
N = 1e3;
t = 1:N; % signal time grid
t0 = 173;
Y = 0*t; Y(t0) = 1;   % single "spike"
T = 5; tcen = floor((T+1)/2);
X = ms_extract_clips2(Y,t0,T);
figure; plot(1:T,X,'+-'); title('should be centered');

% beta>1 ...
N = 1e3;
t = 1:N; % signal time grid
beta = 5;
t0 = 171.51; w0 = 2.0; f = @(t) exp(-.5*((t-t0)/w0).^2); % Gaussian center, width
Y = f(t);
T = 15; tcen = floor((T+1)/2);
[X tcens] = ms_extract_clips2(Y,t0,T,beta);
tcenup = floor((beta*T+1)/2);
t = tcens + ((1:beta*T)-tcenup)/beta;    % upsamp time grid in orig samp units
figure; plot(1:N,Y,'.','markersize',20); hold on; plot(t,[f(t);X],'.-');
plot(t0*[1 1],[0 1],'r-');
legend('Y data pts','true f','upsampled X','center t_0');
axis([min(t),max(t),-.5,1.5]);
title('\beta > 1: should match true func but only centered to within +-0.5')

% beta=1 but off-grid resampling ...
N = 1e3;
t = 1:N; % signal time grid
t0 = 171.35; w0 = 2.0; f = @(t) exp(-.5*((t-t0)/w0).^2); % Gaussian center, width
Y = f(t);
T = 15; tcen = floor((T+1)/2);
for betaonesnap = [inf 10]
  [X tcens] = ms_extract_clips2(Y,t0,T,1,betaonesnap);
  t = tcens + (1:T)-tcen;      % output time grid in orig samp units
  figure; plot(1:N,Y,'.','markersize',20); hold on; plot(t,[f(t);X],'.-');
  plot(t0*[1 1],[0 1],'r-');
  legend('Y data pts','true f','upsampled X','center t_0');
  axis([min(t),max(t),-.5,1.5]);
  title('\beta = 1: should match true func and be centered'); drawnow
end

% timing test for integer and real-valued times at beta=1...
N=1e6; Y = randn(10,N); tj = rand(1,5e4)*N; T = 50;
tic; X = ms_extract_clips2(Y,round(tj),T); toc   % int (7x faster than fast real)
tic; X = ms_extract_clips2(Y,tj,T,1,inf); toc   % slow real
tic; X = ms_extract_clips2(Y,tj,T,1,10); toc   % fast real, about 5x faster
