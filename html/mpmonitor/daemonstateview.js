function DaemonStateView() {
	this.setClient=function(client) {m_client=client;};
	this.loadState=load_state;
	this.div=function() {return m_div;};
	this.onProcessItemClicked=function(callback) {
		m_div.on('process_item_clicked',callback);
	};
	this.onScriptItemClicked=function(callback) {
		m_div.on('script_item_clicked',callback);
	};
	this.setGeom=setGeom;

	var m_div=$('#templates #daemonstateview').clone();

	var daemon_section_height=150;
	
	setGeom(800,800);
	function setGeom(x,y,W,H) {
		absgeom(m_div,x,y,W,H);
		absgeom(m_div.find('#daemon'),0,0,W,daemon_section_height);
		absgeom(m_div.find('#scripts'),0,daemon_section_height,W/2,H-daemon_section_height);
		absgeom(m_div.find('#processes'),W/2,daemon_section_height,W/2,H-daemon_section_height);
	}

	m_div.find('#scripts').css('overflow','scroll');
	m_div.find('#processes').css('overflow','scroll');

	var m_client=null;
	var m_state={};
	var m_error='';

	function set_state(state) {
		m_state=state;
		m_error='';
		refresh();
	}

	function set_error(err) {
		m_error=err; m_state={}; refresh();
	}

	function load_state() {
		if (!m_client) return;
		m_client.getDaemonState(function(resp) {
    		if (resp.success) {
        		set_state(resp.state);
        	}
        	else {
        		set_error('ERR: '+resp.error);
        	}
    	});
	}

	function refresh() {
		var running_str='';
		if (m_error) running_str='MountainProcess daemon: '+m_error; /// TODO show uptime
		else if (m_state.is_running) running_str='MountainProcess daemon is running';
		else running_str='MountainProcess is NOT running';

		m_div.find('#daemon #running').html(running_str);

		m_div.find('#scripts #running').empty();
		m_div.find('#scripts #running').append('<tr><th>Script ID</th><th>Script Names</th></tr>');
		var scripts=m_state.scripts||{};
		for (var key in scripts) {
			var SS=scripts[key];
			var a0=$('<a href=# class=script_item data-key='+key+'>'+key+'</a>');
			var tr=$('#templates #dsv_script_row').clone();
			tr.find('#pript_id').append(a0);
			tr.find('#script_names').html(get_script_names(SS));
			m_div.find('#scripts #running').append(tr);
		}

		m_div.find('#processes #running').empty();
		m_div.find('#processes #running').append('<tr><th>Process ID</th><th>Processor Name</th></tr>');
		var processes=m_state.processes||{};
		for (var key in processes) {
			var PP=processes[key];
			var a0=$('<a href=# class=process_item data-key='+key+'>'+key+'</a>');
			var tr=$('#templates #dsv_process_row').clone();
			tr.find('#pript_id').append(a0);
			tr.find('#processor_name').html(PP.processor_name)
			m_div.find('#processes #running').append(tr);
		}

		m_div.find('.script_item').click(function(evt) {
			m_div.trigger('script_item_clicked',[$(evt.target).attr('data-key')]);
		});
		m_div.find('.process_item').click(function(evt) {
			m_div.trigger('process_item_clicked',[$(evt.target).attr('data-key')]);
		});
	}
	function get_script_names(SS) {
		var ret='';
		var aa=SS.script_names||[];
		return aa.join(', ');
	}
	function absgeom(div,x,y,w,h) {
		div.css({
			position:'absolute',
			width:w,
			height:h,
			left:x,
			top:y
		});
	}
}