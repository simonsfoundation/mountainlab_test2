function MVMainWindow(O,mvcontext) {
	O=O||this;
	JSQWidget(O);

	O.addControlWidget=function(W) {m_control_panel.addControlWidget(W);};
	O.addView=function(container_name,label,V) {addView(container_name,label,V);};
	O.setControlPanelVisible=function(val) {m_control_panel_visible=val; update_layout();};
	O.setStatus=function(name,txt) {m_statuses[name]=txt; update_status();};

	JSQ.connect(O,'sizeChanged',O,update_layout);

	var m_control_panel=new MVControlPanel(0,mvcontext);
	m_control_panel.setParent(O);
	var m_views=[];
	var m_control_panel_visible=true;

	var m_tabber=new Tabber();
	var m_north_tab_widget=m_tabber.createTabWidget('north');
	var m_south_tab_widget=m_tabber.createTabWidget('south');	
	var m_status_bar=new MVStatusBar();
	m_status_bar.setParent(O);
	var m_statuses={};
	m_north_tab_widget.setParent(O);
	m_south_tab_widget.setParent(O);

	m_status_bar.setStatus('Ready.');

	function addView(container_name,label,V) {
		m_views.push({
			V:V,label:label
		});
		m_tabber.addWidget(container_name,label,V);
		setTimeout(function() {
			V.recalculate();
		},10);
	}

	function update_layout() {
		var status_bar_height=30;
		var W0=O.width();
		var H0=O.height()-status_bar_height;

		var W1=W0/3;
		if (W1<250) W1=250;
		if (W1>800) W1=800;
		if (!m_control_panel_visible) W1=0;
		var W2=W0-W1;

		m_control_panel.setSize(W1,H0);
		m_control_panel.setPosition(0,0);

		m_north_tab_widget.setSize(W2,H0/2);
		m_north_tab_widget.setPosition(W1,0);

		m_south_tab_widget.setSize(W2,H0/2);
		m_south_tab_widget.setPosition(W1,H0/2);

		m_status_bar.setSize(W0,status_bar_height-5);
		m_status_bar.setPosition(0,H0+5);
	}
	function update_status() {
		var txt='';
		for (var name in m_statuses) {
			var txt0=m_statuses[name];
			if (txt0) {
				if (txt) txt+=';  ';
				txt+=txt0
			}
		}
		m_status_bar.setStatus(txt);
	}
}

function MVStatusBar(O) {
	O=O||this;
	JSQWidget(O);

	O.setStatus=function(txt) {O.div().html(txt);};
}
