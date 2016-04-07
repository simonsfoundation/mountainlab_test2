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
%    opts.upsampling_factor - the upsampling factor for fractional shift detection based
%                on upsampling
%    opts.num_pca_denoise_components
%    opts.individual_channels - 0 or 1 (if 0 fills first row of output with zeros)
%    opts.pca_denoise_jiggle - if positive, eg 2, theoretically helps with the pca denoising (default 0)
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
if ~isfield(opts,'upsampling_factor') opts.upsampling_factor=1; end;
if ~isfield(opts,'num_pca_denoise_components') opts.num_pca_denoise_components=0; end;
if ~isfield(opts,'pca_denoise_jiggle') opts.pca_denoise_jiggle=0; end;
if ~isfield(opts,'individual_channels') opts.individual_channels=1; end;

cmd=sprintf('%s detect --timeseries=%s --detect_out=%s.1 ',mscmd_exe,timeseries_path,detect_out_path);
cmd=[cmd,sprintf('--clip_size=%d ',opts.clip_size)];
cmd=[cmd,sprintf('--detect_threshold=%g --detect_interval=%d ',opts.detect_threshold,opts.detect_interval)];
cmd=[cmd,sprintf('--sign=%d ',opts.sign)];
cmd=[cmd,sprintf('--individual_channels=%d ',opts.individual_channels)];

fprintf('\n*** DETECT ***\n');
fprintf('%s\n',cmd);
status=system(cmd);
if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

cmd=sprintf('%s adjust_times --timeseries=%s --detect=%s.1 --detect_out=%s ',mscmd_exe,timeseries_path,detect_out_path,detect_out_path);
cmd=[cmd,sprintf('--upsampling_factor=%d ',opts.upsampling_factor)];
cmd=[cmd,sprintf('--num_pca_denoise_components=%d ',opts.num_pca_denoise_components)];
cmd=[cmd,sprintf('--pca_denoise_jiggle=%d ',opts.pca_denoise_jiggle)];
cmd=[cmd,sprintf('--sign=%d ',opts.sign)];

fprintf('\n*** ADJUST TIMES ***\n');
fprintf('%s\n',cmd);
status=system(cmd);
if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end

function times=run_mscmd_detect3(X,opts)
path1=[tempdir,'/test1.mda'];
path2=[tempdir,'/test2.mda'];
writemda32(X,path1);
mscmd_detect3(path1,path2,opts);
detect=readmda(path2);
times=detect(2,:);
end

function ret=test_function(a,num_cycles)
ret=cos(a*pi*num_cycles)*20;
end

function test_mscmd_detect3

%close all; % nah

% single-channel test...
N=400;
num_cycles=40;
noise_level=3;
opts.detect_threshold=8;
opts.detect_interval=6;
opts.clip_size=10;
opts.sign=1; opts.polarity='p';
%opts.num_pca_denoise_components=0;
opts.pca_denoise_jiggle=3;

a=linspace(-1,1,N);
X=test_function(a,num_cycles); X=X+randn(size(X))*noise_level;
a2=linspace(-1,1,N*10);
X2=test_function(a,num_cycles)*1.5;

for use_mscmd=0:1

    comps = [0,0,0,3,3,3];
    betas = [1,2,10,1,2,10];           % sub-sampling factors (1: plain integer times)
    all_errs=[];
    fA=figure; set(gcf,'position',[100 100 2000 1200]);  % nice wide figure
    for i=1:numel(betas)

      if use_mscmd
          opts.upsampling_factor = betas(i);
          opts.num_pca_denoise_components=comps(i);
          times=run_mscmd_detect3(X,opts);
      else
          if (comps(i)) opts.meth='p';
          else opts.meth='x';
          end;
          opts.beta = betas(i);
          times=ms_detect3(X,opts);
      end;

      figure(fA);
      subplot(numel(betas),1,i);
      plot(linspace(1,length(X),length(X2)),X2,'g'); hold on;
      plot(1:length(X),X,'k'); hold on;
      for j=1:length(times), plot(times(j)*[1 1],ylim,'r'); end   % times as vlines
      pct=(times-1)/(length(a)-1);
      points_per_cycle=N/num_cycles;
      errs=abs(pct*num_cycles-round(pct*num_cycles));
      errs_timepoints=errs*points_per_cycle;
      title0=sprintf('use-mscmd=%d, beta=%d, comps=%d: mean error (timepoints): %g',use_mscmd,betas(i),comps(i),mean(errs_timepoints));
      title(title0);
      
    end
end;
end
