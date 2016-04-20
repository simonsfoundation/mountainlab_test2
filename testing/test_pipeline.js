function main() {
	test_pipeline();
}

function test_pipeline() {
	var clip_size=100;
	var detect_threshold=3.5;
	var detect_interval=10;
	var shell_increment=1.5;
	var min_shell_size=150;
	var samplerate=30000;
	var o_filter={
		samplerate:samplerate,
		freq_min:300,
		freq_max:6000
	};
	var o_mask_out_artifacts={
		threshold:3,
		interval_size:200
	};
	var o_whiten={
	};
	var o_detect={
		detect_threshold:detect_threshold,
		detect_interval:detect_interval,
		clip_size:clip_size,
		sign:0
	};
	var o_branch_cluster={
		clip_size:clip_size,
		min_shell_size:min_shell_size,
		shell_increment:shell_increment,
		num_features:3,
		detect_interval:detect_interval
	};
	var o_compute_detectability_scores={
		clip_size:clip_size,
		shell_increment:shell_increment,
		min_shell_size:min_shell_size,
	};
	var o_compute_outlier_scores={
		clip_size:clip_size,
		shell_increment:shell_increment,
		min_shell_size:min_shell_size
	};
	var o_fit_stage={
		clip_size:clip_size,
		min_shell_size:min_shell_size,
		shell_increment:shell_increment
	};

	var pre0='pre0.mda';

	var P=new MSPipeline();

	P.addProcess(bandpass_filter(pre0,'@pre1',o_filter));
	P.addProcess(mask_out_artifacts('@pre1','@pre1b',o_mask_out_artifacts));
	P.addProcess(whiten('@pre1b','@pre2',o_whiten));
	P.addProcess(detect('@pre2','@detect',o_detect));

	P.addProcess(branch_cluster_v2('@pre2','@detect','@firings1',o_branch_cluster));
	P.addProcess(fit_stage('@pre2','@firings1','@firings2',o_fit_stage));
	P.addProcess(compute_outlier_scores('@pre2','@firings2','@firings3',o_compute_outlier_scores));
	P.addProcess(compute_detectability_scores('@pre2','@firings3','@firings4',o_compute_detectability_scores));

	P.addProcess(copy('@firings4','firings.mda'));

	P.run();
}

function MSPipeline() {
	this.addProcess=function(P) {m_processes.push(clone(P));}
	this.run=_run;

	var m_processes=[];

	function sort_processes() {
		var new_processes=[];
		for (var j=0; j<m_processes.length; j++) m_processes[j].handled=false;
		var something_changed=true;
		while (something_changed) {
			something_changed=false;
			for (var j=0; j<m_processes.length; j++) {
				var PP=m_processes[j];
				if (!PP.handled) {
					var ready_to_go=true;
					for (var pname in PP.inputs) {
						var tmp0=find_process_and_pname_for_output(PP.inputs[pname]);
						if (tmp0.process) {
							if (!tmp0.process.handled) ready_to_go=false;
						}
					}
					if (ready_to_go) {
						something_changed=true;
						PP.handled=true;
						new_processes.push(PP);
					}
				}
			}
		}
		if (new_processes.length!=m_processes.length) return false;
		m_processes=new_processes;
		return true;
	}

	function compute_process_codes() {
		//assume sorted
		for (var j=0; j<m_processes.length; j++) m_processes[j].process_code='';
		for (var j=0; j<m_processes.length; j++) {
			var PP=m_processes[j];
			var input_codes={};
			for (var pname in PP.inputs) {
				var fname=PP.inputs[pname];
				var tmp=find_process_and_pname_for_output(fname);
				if (tmp.process) {
					if (!tmp.process.process_code) {
						console.log('Unexpected problem in compute_process_code');
						return false;
					}
					input_codes[pname]=tmp.process.process_code+'--'+tmp.pname;
				}
				else {
					if (fname) {
						input_codes[pname]=MS.fileChecksum(fname);
						if (!input_codes[pname]) {
							console.log('Checksum is empty, perhaps file does not exist for '+pname+': '+fname);
							return false;
						}
					}
					else {
						input_codes[pname]='<empty>';
					}
				}
			}
			var code_obj={};
			for (var pname in PP.inputs) {
				code_obj[pname]=input_codes[pname];
			}
			for (var pname in PP.parameters) {
				code_obj[pname]=clone(PP.parameters[pname]);
			}
			PP.process_code=MS.stringChecksum(JSON.stringify(code_obj));
		}
		return true;
	}

	function _run() {

		//sort so that each process only depends on output from earlier processes
		if (!sort_processes()) {
			console.log('Problem sorting processes. There may be cyclic dependencies.');
			return false;
		}

		//compute the process codes, which should depend on the parameters and the provenance (or checksums) of the input files
		if (!compute_process_codes()) {
			console.log('Problem computing process codes.');
			return false;
		}

		//create the fname mapping
		var fname_map={}; //maps @name to /the/corresponding/temporary/path
		for (var j=0; j<m_processes.length; j++) {
			var PP=m_processes[j];
			if (!PP.process_code) {
				console.log('Unexpected problem. Process code is empty.');
				return false;
			}
			for (var pname in PP.outputs) {
				if (PP.outputs[pname].indexOf('@')==0) {
					fname_map[PP.outputs[pname]]=MS.createTemporaryFileName(PP.process_code+'-'+pname);
				}
			}
		}
		//apply the fname mapping
		for (var j=0; j<m_processes.length; j++) {
			var PP=m_processes[j];
			for (var pname in PP.inputs) {
				var fname=PP.inputs[pname];
				if (fname.indexOf('@')==0) {
					if (fname in fname_map) {
						PP.inputs[pname]=fname_map[fname];
					}
				}
			}
			for (var pname in PP.outputs) {
				var fname=PP.outputs[pname];
				if (fname.indexOf('@')==0) {
					if (fname in fname_map) {
						PP.outputs[pname]=fname_map[fname];
					}
				}
			}
		}

		//run the processes (recall that they were sorted earlier)
		for (var j=0; j<m_processes.length; j++) {
			var PP=m_processes[j];
			var processor_name=PP.processor_name;
			var parameters=clone(PP.parameters);
			for (var pname in PP.inputs) {
				parameters[pname]=PP.inputs[pname];
			}
			for (var pname in PP.outputs) {
				parameters[pname]=PP.outputs[pname];
			}
			if (!MS.runProcess(processor_name,JSON.stringify(parameters))) {
				console.log('Error running process: '+processor_name);
				return false;
			}
		}
		return true;
	}	

	function find_process_and_pname_for_output(fname) {
		for (var j=0; j<m_processes.length; j++) {
			var PP=m_processes[j];
			for (var pname in PP.outputs) {
				if (PP.outputs[pname]==fname) {
					return {process:m_processes[j],pname:pname};
				}
			}
		}
		return {process:null,pname:''};
	}
}

function bandpass_filter(timeseries,timeseries_out,opts) {
	var ret={};
	ret.processor_name='bandpass_filter';
	ret.inputs={timeseries:timeseries};
	ret.outputs={timeseries_out:timeseries_out};
	ret.parameters=clone(opts);
	return ret;
}

function whiten(timeseries,timeseries_out,opts) {
	var ret={};
	ret.processor_name='whiten';
	ret.inputs={timeseries:timeseries};
	ret.outputs={timeseries_out:timeseries_out};
	ret.parameters=clone(opts);
	return ret;
}

function mask_out_artifacts(timeseries,timeseries_out,opts) {
	var ret={};
	ret.processor_name='mask_out_artifacts';
	ret.inputs={timeseries:timeseries};
	ret.outputs={timeseries_out:timeseries_out};
	ret.parameters=clone(opts);
	return ret;
}

function detect(timeseries,detect,opts) {
	var ret={};
	ret.processor_name='detect';
	ret.inputs={timeseries:timeseries};
	ret.outputs={detect_out:detect};
	ret.parameters=clone(opts);
	return ret;
}

function branch_cluster_v2(timeseries,detect,firings,opts) {
	var ret={};
	ret.processor_name='branch_cluster_v2';
	ret.inputs={timeseries:timeseries,detect:detect,adjacency_matrix:''};
	ret.outputs={firings_out:firings};
	ret.parameters=clone(opts);
	return ret;
}

function fit_stage(timeseries,firings,firings_out,opts) {
	var ret={};
	ret.processor_name='fit_stage';
	ret.inputs={timeseries:timeseries,firings:firings};
	ret.outputs={firings_out:firings_out};
	ret.parameters=clone(opts);
	return ret;
}

function compute_outlier_scores(timeseries,firings,firings_out,opts) {
	var ret={};
	ret.processor_name='compute_outlier_scores';
	ret.inputs={timeseries:timeseries,firings:firings};
	ret.outputs={firings_out:firings_out};
	ret.parameters=clone(opts);
	return ret;
}

function compute_detectability_scores(timeseries,firings,firings_out,opts) {
	var ret={};
	ret.processor_name='compute_detectability_scores';
	ret.inputs={timeseries:timeseries,firings:firings};
	ret.outputs={firings_out:firings_out};
	ret.parameters=clone(opts);
	return ret;
}

function copy(input,output) {
	var ret={};
	ret.processor_name='copy';
	ret.inputs={input:input};
	ret.outputs={output:output};
	ret.parameters={};
	return ret;
}
function clone(obj) {
	return JSON.parse(JSON.stringify(obj));
}

var console={
	log:function(msg) {MS.log(msg);}
}