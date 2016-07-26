function MountainProcessRunner() {
	var that=this;

	this.setProcessorName=function(pname) {m_processor_name=pname;}
	this.setInputParameters=function(params) {m_input_parameters=JSQ.clone(params);}
	this.setMLProxyUrl=function(url) {m_mlproxy_url=url;}
	this.makeOutputFileUrl=function(pname) {return makeOutputFileUrl(pname);}
	this.runProcess=function(callback) {runProcess(callback);}

	var m_processor_name='';
	var m_input_parameters={};
	var m_output_parameters={};
	var m_mlproxy_url='';
	var m_detach=0;

	function makeOutputFileUrl(pname) {
		var ret=create_temporary_output_url(m_mlproxy_url,m_processor_name,m_input_parameters,pname);
		m_output_parameters[pname]=ret;
		return ret;
	}
	function runProcess(callback) {
		var all_parameters=JSQ.clone(m_input_parameters);
		for (var key in m_output_parameters) {
			all_parameters[key]=m_output_parameters[key];
		}
		var pp_process={};
		pp_process.processor_name=m_processor_name;
		pp_process.parameters=all_parameters;

		var pp_processes=[];
		pp_processes.push(pp_process);

		var pp={};
		pp.processes=pp_processes;

		var pipeline_json=JSON.stringify(pp);

		var script='';
		script+="function main(params) {\n";
		script+="  MP.runPipeline('"+pipeline_json+"');\n";
		script+="}\n";

		var req={};
		req.action="queueScript";
		req.script=script;
		if (m_detach) req.detach=1;

		var url=m_mlproxy_url+'/mpserver';
		$.post(url,JSON.stringify(req),function(resp) {
			callback(resp);
		});
	}
}

function create_temporary_output_url(mlproxy_url,procesor_name,params,pname) {
	var str=procesor_name+":";
	var keys=[];
	for (key in params) {
		keys.push(key);
	}
	keys.sort();
	for (var i=0; i<keys.length; i++) {
		str+=key+"="+params[key]+"&";
	}
	var file_name=JSQ.computeSha1SumOfString(str)+'_'+pname+'.tmp';
	var ret=mlproxy_url+'/mdaserver/tmp_long_term/'+file_name;
	return ret;
}