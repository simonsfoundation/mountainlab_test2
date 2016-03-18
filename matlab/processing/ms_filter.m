function Y=ms_filter(X,opts)
warning('Please use ms_bandpass_filter rather than m_filter.');
if (nargin<2) opts=struct; end;
Y=ms_bandpass_filter(X,opts);
end