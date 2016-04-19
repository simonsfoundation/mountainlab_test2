function test_pipeline {
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

	var m_processes();

	function _run() {
		var fname_map={};
		for (var j=0; j<m_processes.length; j++) {
			var PP=m_processes[j];
			PP.process_code=compute_process_code(PP);
			if (!PP.process_code) {
				console.log('Problem computing process code.');
				return false;
			}
			for (var pname in PP.outputs) {
				if (PP.outputs[pname].indexOf('@')==0) {
					fname_map[PP.outputs[pname]]='tmp.'+PP.process_code+'-'+pname;
				}
			}
		}
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
			if (!MS.run_process(processor_name,parameters)) {
				console.log('Error running process: '+processor_name);
				return false;
			}
		}
		return true;
	}	

	function compute_file_code(fname) {
		return MS.fileChecksum(fname);
	}

	function compute_object_code(obj) {
		return MS.stringChecksum(JSON.stringify(obj));
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

	function compute_process_code(P) {
		if (P.process_code) return P.process_code;
		P.computing_code=true;
		var input_codes={};
		for (var pname in P.inputs) {
			var fname=P.inputs[pname];
			var tmp=find_process_and_pname_for_output(fname);
			var P0=tmp.process;
			var pname0=tmp.pname;
			if (P0) {
				if (P0.computing_code) {
					console.log('Cyclic process dependency.');
					return '';
				}
				var code0=compute_process_code(P0)
				if (!code0) {
					console.log('Problem computing process code');
					return '';
				}
				input_codes[pname]=code0+'--'+pname0;
			}
			else {
				var file_code=compute_file_code(fname);
				if (!file_code) {
					console.log('Unable to compute file code: '+fname);
					return '';
				}
				input_codes[pname]=file_code;

			}
		}
		for (var pname in P.parameters) {
			input_codes[pname]=P.parameters[pname];
		}
		P.computing_code=false;
		P.process_code=compute_object_code(input_codes);
		return P.process_code;
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