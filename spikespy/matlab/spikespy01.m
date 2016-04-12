function H=spikespy01(varargin) 

%first loop through the arguments and get the options
opts=struct;
for j=1:length(varargin)
    arg=varargin{j};
    if (isstruct(arg))
        opts=arg;
    end;
end;
%default options
if (~isfield(opts,'sampling_freq')) opts.sampling_freq=0; end;
if (isfield(opts,'sampfreq')) opts.sampling_freq=opts.sampfreq; end;

script={};
script{end+1}='var W=SPIKESPY.createTimeSeriesWidget();';

verbose=0;
data=struct;
for j=1:length(varargin)
	arg=varargin{j};
	if (isnumeric(arg))
		Xvar=sprintf('X%d',j); data.(Xvar)=arg;
		script{end+1}='var V=SPIKESPY.createTimeSeriesView();';
        script{end+1}=sprintf('V.setSamplingFrequency(%f);',opts.sampling_freq);
		script{end+1}=sprintf('var X=SPIKESPY.loadArray($%s$);',Xvar);
		script{end+1}='V.setData(X);';
		script{end+1}='W.addView(V);';	
	elseif (iscell(arg))&&(length(arg)==1)
		Xvar=sprintf('X%d',j); data.(Xvar)=arg{1};
		script{end+1}='var V=SPIKESPY.createTimeSeriesView();';
        script{end+1}=sprintf('V.setSamplingFrequency(%f);',opts.sampling_freq);
		script{end+1}=sprintf('var X=SPIKESPY.loadArray($%s$);',Xvar);
		script{end+1}='V.setData(X);';
		script{end+1}='W.addView(V);';	
	elseif (iscell(arg))&&(length(arg)==2)
		if (isstr(arg{2}))
			Xvar=sprintf('X%d',j); data.(Xvar)=arg{1};
			script{end+1}='var V=SPIKESPY.createTimeSeriesView();';
            script{end+1}=sprintf('V.setSamplingFrequency(%f);',opts.sampling_freq);
			script{end+1}=sprintf('var X=SPIKESPY.loadArray($%s$);',Xvar);
			script{end+1}='V.setData(X);';
			script{end+1}='W.addView(V);';	
			script{end+1}=sprintf('V.setTitle(''%s'');',arg{2});
		else
			if (~isrow(arg{1})) arg{1}=arg{1}'; end; %handle column vector
			if (~isrow(arg{2})) arg{2}=arg{2}'; end; %handle column vector
			TLvar=sprintf('T%d',j); data.(TLvar)=[arg{1};arg{2}];
			%Lvar=sprintf('L%d',j); data.(Lvar)=arg{2};
			script{end+1}='var V=SPIKESPY.createLabelView();';
			script{end+1}=sprintf('var TL=SPIKESPY.readArray($%s$);',TLvar);
			%script{end+1}=sprintf('var L=SPIKESPY.readArray($%s$);',Lvar);
			script{end+1}='V.setLabels(TL);';
			script{end+1}='W.addView(V);';
		end;
	elseif (iscell(arg))&&(length(arg)>=3)
		Xvar=sprintf('X%d',j); data.(Xvar)=arg{1};
		if (~isrow(arg{2})) arg{2}=arg{2}'; end; %handle column vector
		if (~isrow(arg{3})) arg{3}=arg{3}'; end; %handle column vector
		TLvar=sprintf('T%d',j); data.(TLvar)=[arg{2};arg{3}];
		%Lvar=sprintf('L%d',j); data.(Lvar)=arg{3};
		script{end+1}='var V=SPIKESPY.createTimeSeriesView();';
        script{end+1}=sprintf('V.setSamplingFrequency(%f);',opts.sampling_freq);
		script{end+1}=sprintf('var X=SPIKESPY.loadArray($%s$);',Xvar);
		script{end+1}=sprintf('var TL=SPIKESPY.readArray($%s$);',TLvar);
		%script{end+1}=sprintf('var L=SPIKESPY.readArray($%s$);',Lvar);
		script{end+1}='V.setData(X);';
		script{end+1}='V.setLabels(TL);';
		script{end+1}='W.addView(V);';
		if (length(arg)>=4)
			script{end+1}=sprintf('V.setTitle(''%s'');',arg{4});
		end;
	elseif (isstr(arg))
		verbose=1;
	end;
end;

if (~exist(spikespy_exec,'file'))
    error(sprintf('Unable to find spikespy executable (%s). Mostly likely, you need to compile spikespy. See the README if available.',spikespy_exec));
end;

Htmp=run_java_script(spikespy_exec,script,data,verbose);
if (nargout>0) H=Htmp; end;

end
