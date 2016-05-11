function DaemonStateView() {
	this.setState=function(state) {setState(state);};
	this.setError=function(err) {m_error=err; m_state={}; refresh();};
	this.div=function() {return m_div;};


	var m_div=$('#templates #daemonstateview').clone();
	var m_state={};
	var m_error='';

	function setState(state) {
		console.log('setState');
		m_state=state;
		m_error='';
		refresh();
	}

	function refresh() {
		var running_str='';
		if (m_error) running_str=m_error; /// TODO show uptime
		else if (m_state.is_running) running_str='Daemon is running';
		else running_str='Daemon is NOT running';

		m_div.find('#running').html(running_str);

		m_div.find('#running_scripts').empty();
		m_div.find('#running_scripts').append('<tr><th>Script ID</th><th>Script Names</th></tr>');
		var scripts=m_state.scripts||{};
		for (var key in scripts) {
			var SS=scripts[key];
			var tr=$('#templates #dsv_script_row').clone();
			tr.find('#pript_id').html(key);
			tr.find('#script_names').html(get_script_names(SS));
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
	function get_script_names(SS) {
		var ret='';
		var aa=SS.script_names||[];
		return aa.join(', ');
	}
}