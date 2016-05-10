function DaemonStateView() {
	this.setState=function(state) {setState(state);};
	this.div=function() {return m_div;};

	var m_div=$('#templates #daemonstateview').clone();
	var m_state={};

	function setState(state) {
		console.log(state);
		m_state=state;
		refresh();
	}

	function refresh() {
		var running_str='NO';
		if (m_state.is_running) running_str='YES';

		m_div.find('#running').html(running_str);

		m_div.find('#running_scripts').empty();
		m_div.find('#running_scripts').append('<tr><th>Script ID</th><th>Script Files</th></tr>');
		var scripts=m_state.scripts||{};
		for (var key in scripts) {
			var SS=scripts[key];
			var tr=$('#templates #dsv_script_row').clone();
			tr.find('#pript_id').html(key);
			tr.find('#script_file_names').html(get_script_file_names(SS));
			m_div.find('#running_scripts').append(tr);
		}

		m_div.find('#running_processes').empty();
		m_div.find('#running_processes').append('<tr><th>Process ID</th><th>Processor Name</th></tr>');
		var processes=m_state.processes||{};
		for (var key in processes) {
			var PP=processes[key];
			var tr=$('#templates #dsv_process_row').clone();
			tr.find('#pript_id').html(key);
			tr.find('#processor_name').html(PP.processor_name)
			m_div.find('#running_processes').append(tr);
		}
	}
	function get_script_file_names(SS) {
		var ret='';
		var pp=SS.script_paths||[];
		for (var i=0; i<pp.length; i++) {
			if (i>0) ret+=' ';
			ret+=get_file_name(pp[i]);
		}
		return ret;
	}
	function get_file_name(path) {
		ind=path.lastIndexOf('/');
		if (ind>=0) {
			return path.slice(ind+1);
		}
		else return path;
	}
}