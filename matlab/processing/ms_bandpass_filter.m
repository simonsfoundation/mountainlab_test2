function Y=ms_bandpass_filter(X,opts)
%MS_FILTER - Bandpass filter using smooth roll-offs in Fourier space
%
%Consider using mscmd_bandpass_filter
%
% Syntax:  [Y] = ms_bandpass_filter(X,opts)
%
% Inputs:
%    X - MxN array of raw data
%    opts.samplefreq - the sampling frequency corresponding to X, e.g.
%                      30000
%    opts.freq_min - the lower end of the bandpass filter
%    opts.freq_max - the upper end of the bandpass filter
%
% Outputs:
%    Y - MxN array of filtered data
%
% Example:
%    Y=ms_bandpass_filter(X,struct('samplefreq',30000,'freq_min',300,'freq_max',6000));
%
% Other m-files required: none
%
% See also: mscmd_bandpass_filter, spikespy

% Author: Jeremy Magland and Alex Barnett
% Oct 2015; Last revision: 13-Feb-2016
% name changed to ms_bandpass_filter on 3/18/16 - jfm

% TODO: test routine

Y=freqfilter(X,opts.samplerate,opts.freq_min,opts.freq_max);
end

function X = freqfilter(X,fs,flo,fhi)
% A snapshot of ahb's freqfilter on 5/22/2015
% SS_FREQFILTER - filter rows of X using smooth roll-offs in Fourier space
%
% X = freqfilter(X,fs,flo,fhi) where X is M*N matrix returns a matrix of same
%  size where each row has been filtered in a bandpass filter with soft rolloffs
%  from flo to fhi (in Hz). fs is the sampling freq in Hz. If flo is empty,
%  a lo-pass is done; if fhi is empty, a hi-pass.
%
% Note: MATLAB's fft is single-core even though claims multicore, for fft(X,[],2)
%  only. fft(X) is multicore.
%
% Hidden parameters: fwid - width of roll-off (Hz). todo: make options.
%
% todo: This could act on EC data object instead?
%
% Barnett 11/14/14.
% 3/11/15: transpose to fft cols (is multicore), blocking (was slower!)

if nargin<3, flo = []; end
if nargin<4, fhi = []; end
if ~isempty(fhi) & ~isempty(flo) & fhi<=flo, warning('fhi<=flo: are you sure you want this??'); end

[M N] = size(X);

Nbig = inf; %2^22;  % do it in blocks (power of 2 = efficient for FFT)
if N>2*Nbig
  pad = 1e3;   % only good if filters localized in time (smooth in k space)
  B = Nbig-2*pad;  % block size
  Xpre = zeros(M,pad);
  ptr = 0;
  while ptr+B<N  % so last block can be at most 2B wide
    if ptr+2*B<N, j = ptr+(1:B+pad);
    else, j = ptr+1:N; end  % final block is to end of data
    Y = [Xpre X(:,j)];  % for all but last time Y has Nbig cols, fast
    Xpre = X(:,ptr+B+(-pad+1:0));  % before gets overwritten get next left-pad
    size(Y)
    Yf = freqfilter(Y,fs,flo,fhi);
    X(:,j) = Yf(:,pad+1:end); % right-pad region will get overwritten, fine
    ptr = ptr + B;
  end
  return              % !
end                   % todo: figure out why this was slower than unblocked
                      % even for Nbig = 2^22

T = N/fs;  % total time
df = 1/T; % freq grid...
if mod(N,2)==0, f=df*[0:N/2 -N/2+1:-1];else, f=df*[0:(N-1)/2 -(N-1)/2:-1]; end

a = ones(size(f));
fwidlo = 100;       % roll-off width (Hz). Sets ringing timescale << 10 ms
if ~isempty(flo), a = a .* (1+tanh((abs(f)-flo)/fwidlo))/2; end
fwidhi = 1000;       % roll-off width (Hz). Sets ringing timescale << 1 ms
if ~isempty(fhi), a = a .* (1-tanh((abs(f)-fhi)/fwidhi))/2; end
X = ifft(bsxfun(@times, fft(X'), a'))'; % filter: FFT fast along fast
% storage direction, transposing at input & output

end