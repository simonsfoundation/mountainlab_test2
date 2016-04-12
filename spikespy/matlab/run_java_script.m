function H=run_java_script(exec_string,script_fname_or_script,data,verbose)

if (iscell(script_fname_or_script))
	str='';
	for j=1:length(script_fname_or_script)
		str=sprintf('%s%s\n',str,script_fname_or_script{j});
	end;
	script_fname_or_script=str;
	if (verbose) disp(str); end;
end;

if (strcmp(script_fname_or_script(end-2:end),'.js'))
	js_code=read_text_file(script_fname_or_script);
else
	js_code=script_fname_or_script;
end;

the_handle=0;

datafields=fieldnames(data);
for ff=1:length(datafields)
	fieldname=datafields{ff};
	X=data.(fieldname);
	if (strcmp(fieldname,'handle'))
		the_handle=X;
	elseif (isstr(X))
		js_code=strrep(js_code,['$',fieldname,'$'],sprintf('"%s"',X));
	elseif (size(X(:))==1)
			if (isinteger(X))
				js_code=strrep(js_code,['$',fieldname,'$'],sprintf('%d',X));
			else
				js_code=strrep(js_code,['$',fieldname,'$'],sprintf('%f',X));
			end;
	else %assume an array (isnumeric doesn't seem to work for boolean)
		tmppath=[get_temp_path,'/',efficient_hash(X),'_run_java_script.mda'];
		%tmppath=[get_temp_path,'_run_java_script_variable_',fieldname,'.mda'];
		if (~exist(tmppath,'file'))
			tic;
			writemda(X,[tmppath,'.tmp']);
			movefile([tmppath,'.tmp'],tmppath); %do this so we don't run the risk of writing a partial file and then regretting that later
			elapsed=toc;
			if (toc>0.5) fprintf('Time for writing file: %.2f seconds\n',elapsed); end;
		else
			if (length(X(:))>1e6)
				if (verbose) disp('File already exists - that saves some time!'); end;
			end;
		end;
		js_code=strrep(js_code,['$',fieldname,'$'],sprintf('"%s"',tmppath));
	end;
end;

js_path=[get_temp_path,'/',efficient_hash(js_code),'_run_java_script.js'];
write_text_file(js_code,js_path);


other_params='';
if (~isstruct(the_handle))
	id=make_random_id();
	other_params=sprintf('--id=%s %s',id,other_params);
	H.id=id;
else
	other_params=sprintf('--connect_to=%s %s',the_handle.id,other_params);
	H.id=the_handle.id;
end;

cmd=sprintf('%s %s %s &',exec_string,js_path,other_params);
disp(cmd);
system(cmd);

pause(0.3); %give it some time to open so windows (almost) always open in right order

end

function str=make_random_id
SET = char(['a':'z']) ;
NSET = length(SET) ;
N = 10 ; % pick N characters
i = ceil(NSET*rand(1,N)) ; % with repeat
str = SET(i) ;
end

function write_text_file(txt,path)
F=fopen(path,'w');
fprintf(F,'%s',txt);
fclose(F);
end

function txt=read_text_file(path)
txt=fileread(path);
end

function ret=efficient_hash(X)

%we don't want to run out of memory
interval=ceil(length(X(:))/10000000);

tic;
str='';
for j=1:interval
	tmp=DataHash(X(j:interval:end),struct('Method','MD5'));
	str=[str,tmp];
end;

ret=DataHash(str,struct('Method','MD5'));
elapsed=toc;
if (elapsed>0.5)
	fprintf('Time to compute the md5 sum: %.2f seconds\n',elapsed);
end;

end

function ret=get_temp_path

%ret=sprintf('%s/../tmp',fileparts(mfilename('fullpath')));
ret=sprintf('%s/spikespy',tempdir);
if (~exist(ret,'dir'))
	mkdir(tempdir,'spikespy');
end;

end
