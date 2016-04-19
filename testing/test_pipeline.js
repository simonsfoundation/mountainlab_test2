function main() {
	test_pipeline();
}

function test_pipeline() {
	var o_bandpass_filter={samplerate:30000,freq_min:300,freq_max:3000};
	var o_whiten={};

	var X=new MSPipeline();
	X.addProcess(bandpass_filter('pre0.mda','@pre1',o_bandpass_filter));
	X.addProcess(whiten('@pre1','pre2.mda',o_whiten));
	X.run();
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
					input_codes[pname]=MS.fileChecksum(fname);
					if (!input_codes[pname]) {
						console.log('Checksum is empty, perhaps file does not exist: '+fname);
						return false;
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
					fname_map[PP.outputs[pname]]=MS.createTemporaryFile(PP.process_code+'-'+pname);
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
			if (!MS.runProcess(processor_name,parameters)) {
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
	var ret=clone(opts);
	ret.processor_name='bandpass_filter';
	ret.inputs={timeseries:timeseries};
	ret.outputs={timeseries_out:timeseries_out};
	ret.parameters=opts;
	return ret;
}

function whiten(timeseries,timeseries_out,opts) {
	var ret=clone(opts);
	ret.processor_name='whiten';
	ret.inputs={timeseries:timeseries};
	ret.outputs={timeseries_out:timeseries_out};
	ret.parameters=opts;
	return ret;
}

function clone(obj) {
	return JSON.parse(JSON.stringify(obj));
}