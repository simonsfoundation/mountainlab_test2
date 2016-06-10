function spikespy2(varargin)

% SPIKESPY2 - Launch MountainView in spikespy mode (mv needs to be compiled
% separately)
%
% SpikeSpyallows interactive visualization of a
% raw/preprocessed dataset with a collection of times/labels.
%
% Syntax:  
% H=spikespy2(X) -- X is the raw timeseries data (2D raw, #channels x #timepoints)
% H=spikespy2(X1,X2,...) -- Same, except with vertically arranged synchronized views
% H=spikespy2({X,T,L}) -- X is the raw timeseries data, T are the spike
%         timepoints, and L are the corresponding labels. (tj and lj are vectors of length #labels)
% H=spikespy2({X1,T1,L1},{X2,T2,L2},...) -- ya know
%
% H=spikespy2(...,opts) -- options structure can be added at the end
%      opts.samplerate - the sampling rate in Hz, eg 20000
%
% Other m-files required: requires compilation of mountainview
%
% See also: mountainview

% Author: Jeremy Magland
% June 2016;
% Original spikespy: Jan 2015;


%first loop through the arguments and get the options
opts=struct;
for j=1:length(varargin)
    arg=varargin{j};
    if (isstruct(arg))
        opts=arg;
    end;
end;
%default options
if (~isfield(opts,'samplerate')) opts.samplerate=0; end;
disp(opts);

views={};
for j=1:length(varargin)
	arg=varargin{j};
    VV=struct;
	if (isnumeric(arg))
		VV.timeseries=pathify32(arg);
        views{end+1}=VV;
	elseif (iscell(arg))&&(length(arg)==1)
        VV.timeseries=pathify32(arg{1});
        views{end+1}=VV;
	elseif (iscell(arg))&&(length(arg)==2)
        VV.timeseries=pathify32(arg{1});
        VV.firings=pathify64(arg{2});
        views{end+1}=VV;
	elseif (iscell(arg))&&(length(arg)==3)
        VV.timeseries=pathify32(arg{1});
        VV.times=arg{2};
        VV.labels=arg{3};
        views{end+1}=VV;
	end;
end;

timeseries_paths={};
firings_paths={};
for jj=1:length(views)
    VV=views{jj};
    if (isfield(VV,'timeseries'))
        timeseries_paths{end+1}=VV.timeseries;
    end;
    if (isfield(VV,'firings'))
        firings_paths{end+1}=VV.firings;
    elseif (isfield(VV,'times'))
        times0=arrayify(VV.times);
        labels0=arrayify(VV.labels);
        firings0=zeros(3,length(times0));
        firings0(2,:)=times0;
        firings0(3,:)=labels0;
        firings_paths{end+1}=pathify64(firings0);
    else
        firings_paths{end+1}='';
    end;
end;

timeseries_str=strjoin(timeseries_paths,',');
firings_str=strjoin(firings_paths,',');

mfilepath=fileparts(mfilename('fullpath'));
mountainview_exec=[mfilepath,'/../../mountainview/bin/mountainview'];
if (~exist(mountainview_exec,'file'))
    error('Unable to find mountainview executable (%s). Mostly likely, you need to compile mountainview. See the README if available.',mountainview_exe);
end;

cmd=sprintf('%s --mode=spikespy --timeseries=%s --firings=%s --samplerate=%g',mountainview_exe,timeseries_str,firings_str,opts.samplerate);
disp(cmd);
system(cmd);

end

function path=pathify32(X)
if (ischar(X))
    path=X;
    return;
end;
path=make_temp_mda_path(X,'32');
writemda32(X,path);
end

function path=pathify64(X)
if (ischar(X))
    path=X;
    return;
end;
path=make_temp_mda_path(X,'64');
writemda64(X,path);
end

function X=arrayify(path)
if (isnumeric(path))
    X=path;
    return;
end;
X=readmda(path);
end

function ret=make_temp_mda_path(X,str)
%mfilepath=fileparts(mfilename('fullpath'));  % bad to use NFS for tmp; local /tmp/ is better
%path0=[mfilepath,'/../../tmp/tmp_short_term'];
path0 = '/tmp/mountainlab/tmp_short_term';         % for unix systems only!
if (~exist(path0,'dir'))
    mkdir(path0);
end;
ret=[path0,'/',efficient_hash(X),'-',str,'.mda'];
end

function h=local_hash(str)
% return hex string encoding a matlab variable, plain method 3rd-party code.
h = DataHash(str,struct('Method','MD5'));
end

function h=efficient_hash(X)
% hex string encoding a huge array in RAM, using hack speedups.
% idea is it's very unlikely X could change without changing the output h.
% AHB 6/10/16
tic
s = '';
s = [s, local_hash(sum(X,2))];   % channel sums
s = [s, local_hash(X(:,1e3))];   % start of data
s = [s, local_hash(X(:,end-1e3:end))];   % end of data
[M N] = size(X);
y = X(1:(10*M+1):end);      % stride so hits each channel every 10M t-pts
off = 0;           % index offset
big = 1e7;         % max size to send to local_hash (uses RAM)
for i=1:ceil(numel(y)/big)        % hash y in contiguous chunks if needed
  inds = off+1:min(numel(y),off+big);
  s = [s, local_hash(y(inds))];
  off = off+big;
end
h = DataHash(s,struct('Method','MD5'));
elapsed=toc;
if (elapsed>0.5)
	fprintf('Time to compute the md5 sum: %.2f seconds\n',elapsed);
end;

end
