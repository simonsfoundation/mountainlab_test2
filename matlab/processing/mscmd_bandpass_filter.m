function mscmd_bandpass_filter(input_path,output_path,opts)
%MSCMD_BANDPASS_FILTER - Bandpass filter using smooth roll-offs in Fourier space
%
%The is the mountainsort version of ms_filter
%
% Syntax:  mscmd_bandpass_filter(input_path,output_path,opts)
%
% Inputs:
%    input_path - mda file of MxN input timeseries data
%    output_path - mda file of MxN output filtered data
%    opts.samplerate - the sampling frequency corresponding to X, e.g. 30000
%    opts.freq_min - the lower end of the bandpass filter
%    opts.freq_max - the upper end of the bandpass filter
%    opts.freq_wid - (optional) the width of the roll-off for the upper end
%                    (default 1000; should match ms_bandpass_filter.m)
%
% Use opts.freq_min=0 for a purely high-pass filter
% Use opts.freq_max=0 or inf for a purely low-pass filter
%
% Other m-files required: mscmd_exe
%
% See also: ms_filter, spikespy

% Author: Jeremy Magland
% Jan 2016; Revised: 13-Feb-2016
% 3/3/2016: options for freq_min=0 or freq_max=0.
% 6/14/16: ahb added optional opts.freq_wid (sim to fwidhi in C code)

if nargin<1, test_mscmd_bandpass_filter; return; end;

if ~isfield(opts,'freq_wid'), opts.freq_wid = 1e3; end  % default roll-off width

if isinf(opts.freq_max), opts.freq_max=0; end; %added 3/3/2016

cmd=sprintf('%s bandpass_filter --timeseries=%s --timeseries_out=%s ',mscmd_exe,input_path,output_path);
cmd=[cmd,sprintf('--samplerate=%g --freq_min=%g --freq_max=%g --freq_wid=%g',opts.samplerate,opts.freq_min,opts.freq_max,opts.freq_wid)];

fprintf('\n*** BANDPASS FILTER ***\n');
fprintf('%s\n',cmd);
status=system(cmd);

if (status~=0)
    error('mountainsort returned with error status %d',status);
end;

end

function test_mscmd_bandpass_filter

close all;

M=5;
N=400;
X=randn(M,N);     % seems not to be used
X(:)=0; X(1,floor((N+1)/2))=1;   % Kronecker delta spike
path1=[ms_temp,'/tmp1.mda'];
path2=[ms_temp,'/tmp2.mda'];
writemda(X,path1);
opts.samplerate=30000;
opts.freq_min=200;
opts.freq_max=3000;
% (could add freq_wid here but it has a default value)
mscmd_bandpass_filter(path1,path2,opts);
Y=readmda(path2);

figure;
plot(1:N,X(1,:),'b',1:N,Y(1,:),'r');

end
