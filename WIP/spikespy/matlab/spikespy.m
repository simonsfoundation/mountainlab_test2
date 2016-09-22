function H=spikespy(varargin)
%SPIKESPY - Launch the SpikeSpy viewer (needs to be compiled
%separately)
%
%The SpikeSpy program allows interactive visualization of a
%raw/preprocessed dataset with a collection of times/labels.
%
% Syntax:  
% H=spikespy(X) -- X is the raw timeseries data (2D raw, #channels x #timepoints)
% H=spikespy(X,{tj,lj}) -- X is the raw timeseries data, tj are the spike times, and lj
%                    are the corresponding labels. (tj and lj are vectors of length #labels)
% H=spikespy({X,tj,l},{tj,lj}) -- same as previous, but shows label markers on top plot
%
% H=spikespy(X,Y,Z,{t1,l1},{X2,t2,l2},...) -- see you can use any number of arguments
%
% H=spikespy(...,opts) -- options structure can be added at the end
%      opts.sampling_freq - the sampling frequency in Hz, eg 20000
%
% spikespy() -- show usage information, shortcut keys, etc.
%
% THE FOLLOWING DO NOT WORK RIGHT NOW:::::::::::::::::
% spikespy(H,'close'); -- close the view 
% spikespy('closeall'); -- close all views
% ::::::::::::::::::::::::::::::::::::::::::::::::::::
%
% Other m-files required: spikespy01, requires compilation of spikespy
%
% See also: mountainview

% Author: Jeremy Magland
% Jan 2015; Last revision: 15-Feb-2106


if nargin<1
fprintf('------ SPIKESPY ------\n');
fprintf('-- Mousewheel scroll: zoom in, zoom out\n');
fprintf('-- +/- keys also zoom in, zoom out\n');
fprintf('-- Click and drag to pan\n');
fprintf('-- "0" to zoom back to full view\n');
fprintf('-- Left/right arrow keys scroll left/right by one page\n');
fprintf('-- Ctrl + left/right arrow keys move cursor left/right\n');
fprintf('-- Right-click and drag to select a time interval\n');
fprintf('-- ENTER to zoom in to the selected time interval\n');
fprintf('-- up/down arrow keys do vertical zoom (on all views together)\n');
%fprintf('-- F (when clicked on a view) flips the channel ordering\n');
return;
end;

H=spikespy01(varargin{:});
return;

% Everything below here is on hold

if (nargin<1)
	error('Not enough input arguments.');
end;

global spikespy_all_handles;

arg1=varargin{1};
if (isstring(arg1))
	str=arg1;
	if (strcmp(str,'close'))
		run_java_script(spikespy_exec(),'W.getScene().getWindow().close();',data);
		return;
	elseif (strcmp(str,'closeall'))
		if (iscell(spikespy_all_handles))
			for jj=1:length(spikespy_all_handles)
				data.handle=spikespy_all_handles{jj};
				run_java_script(spikespy_exec(),'W.getScene().getWindow().close();',data);
			end;
			spikespy_all_handles={};
		end;
		return;
	end;
	H=0;
	return;
end;

Htmp=spikespy01(varargin{:});

if (nargout>=1) H=Htmp; end; % we don't want to return the handle if not requested.

if (~iscell(spikespy_all_handles)) spikespy_all_handles=cell(1,0); end;
spikespy_all_handles{length(spikespy_all_handles)+1}=Htmp;

end


