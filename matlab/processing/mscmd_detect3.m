function mscmd_detect3(timeseries_path,detect_out_path,opts)
%MSCMD_DETECT3 - Detect super-threshold events in a raw/preprocessed dataset
%
%This is a wrapper to a command-line call to mountainsort. The
%functionality is similar to that of ms_detect3.
%
% Syntax:  mscmd_detect(input_path,output_path,opts)
%
% Inputs:
%    timeseries_path - path to input MxN array of raw or preprocessed data
%    detect_out_path - path to output 2xL array of detected event channels and
%                  times. The first row contains the primary channels of
%                  the detected events and the second row contains the
%                  integer timepoints at the events.
%    opts.detect_interval - minimum number of integer timepoints separating
%                           two detected events
%    opts.detect_threshold - detect events where the absolute value of the
%                            signal exceeds this threshold.
%    opts.clip_size - don't get too close to the edges!!
%    opts.sign - 0,-1,or 1 (use zero to detect both pos and neg peaks)
%    opts.beta - the upsampling factor for fractional shift detection based
%                on upsampling
%
% Other m-files required: mscmd_exe
%
% See also: ms_detect3

% Author: Jeremy Magland and Alex Barnett
% Mar 2016

if nargin<1, test_mscmd_detect3; return; end;
if (nargin<3) opts=struct; end;

if ~isfield(opts,'clip_size') opts.clip_size=100; end;
if ~isfield(opts,'sign') opts.sign=0; end;
if ~isfield(opts,'beta') opts.beta=1; end;

cmd=sprintf('%s detect3 --timeseries=%s --detect_out=%s ',mscmd_exe,timeseries_path,detect_out_path);
cmd=[cmd,sprintf('--clip_size=%d ',opts.clip_size)];
cmd=[cmd,sprintf('--detect_threshold=%g --detect_interval=%d ',opts.detect_threshold,opts.detect_interval)];
cmd=[cmd,sprintf('--sign=%d ',opts.sign)];
cmd=[cmd,sprintf('--beta=%d ',opts.beta)];

fprintf('\n*** DETECT3 ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end

function times=run_detect3(X,opts)
path1=[tempdir,'/test1.mda'];
path2=[tempdir,'/test2.mda'];
writemda32(X,path1);
mscmd_detect3(path1,path2,opts);
detect=readmda(path2);
times=detect(2,:);
end

function test_mscmd_detect3
% single-channel test...
N=39;
a=linspace(-1,1,N);
X=sin(a*2*pi*11)*20;
a2=linspace(-1,1,N*10);
X2=sin(a2*2*pi*11)*30;

opts.detect_threshold=8;
opts.detect_interval=3;
opts.clip_size=5;
betas = [1 3 10];           % sub-sampling factors (1: plain integer times)
figure; set(gcf,'position',[100 1000 2000 500]);  % nice wide figure
for i=1:numel(betas)
  opts.beta = betas(i);
  %times=ms_detect3(X,opts)
  times=run_detect3(X,opts)
  subplot(numel(betas),1,i);
  plot(linspace(1,length(X),length(X2)),X2,'g'); hold on;
  plot(1:length(X),X,'k'); hold on;
  for j=1:length(times), plot(times(j)*[1 1],ylim,'r'); end   % times as vlines
  title(sprintf('detection on white noise signal: \\beta = %d\n',opts.beta))
end

end
