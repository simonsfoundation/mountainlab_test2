function MVMainWindow(O,mvcontext) {
	O=O||this;
	JSQWidget(O);

	O.addControlWidget=function(W) {m_control_panel.addControlWidget(W);};
	O.addView=function(container_name,label,V) {addView(container_name,label,V);};
	O.setControlPanelVisible=function(val) {m_control_panel_visible=val; update_layout();};
	O.setStatus=function(name,txt) {m_statuses[name]=txt; update_status();};
	O.setTitle=function(title) {m_title=title;};
	O.setSMVObject=function(obj) {m_smv_object=JSQ.clone(obj);};
	O.setupActions=function() {setupActions();};

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
	var m_menu_bar=new MVMenuBar();
	m_menu_bar.setParent(O);
	m_north_tab_widget.setParent(O);
	m_south_tab_widget.setParent(O);
	var m_title='blank.smv';
	var m_smv_object={};

	function setupActions() {
		m_menu_bar.clearActions();
		{
			var download_url='data:text/plain;charset=UTF-8,'+encodeURIComponent(JSON.stringify(m_smv_object));
			m_menu_bar.addAction({text:'Download .smv',url:download_url,file_name:m_title});
		}
		{
			var download_url='data:text/plain;charset=UTF-8,'+encodeURIComponent(JSON.stringify(mvcontext.getMVFileObject()));
			m_menu_bar.addAction({text:'Download .mv',url:download_url,file_name:m_title+'.mv'});
		}
	}

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
		var status_bar_height=20;
		var menu_bar_height=16;
		var W0=O.width();
		var H0=O.height()-status_bar_height-menu_bar_height;

		var W1=W0/3;
		if (W1<250) W1=250;
		if (W1>800) W1=800;
		if (!m_control_panel_visible) W1=0;
		var W2=W0-W1;

		m_control_panel.setSize(W1,H0);
		m_control_panel.setPosition(0,menu_bar_height);

		m_north_tab_widget.setSize(W2,H0/2);
		m_north_tab_widget.setPosition(W1,menu_bar_height);

		m_south_tab_widget.setSize(W2,H0/2);
		m_south_tab_widget.setPosition(W1,menu_bar_height+H0/2);

		m_status_bar.setSize(W0,status_bar_height-5);
		m_status_bar.setPosition(0,menu_bar_height+H0+5);

		m_menu_bar.setSize(W0,menu_bar_height);
		m_menu_bar.setPosition(0,0);
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
	function download_smv(evt) {
		console.log('download_smv');
		evt.preventDefault();  //stop the browser from following
    	window.location.href = m_download_url;
	}
}

function MVStatusBar(O) {
	O=O||this;
	JSQWidget(O);
	O.div().addClass('MVStatusBar');

	O.setStatus=function(txt) {O.div().html(txt);};
}

function MVMenuBar(O) {
	O=O||this;
	JSQWidget(O);
	O.div().addClass('MVMenuBar');

	O.clearActions=function() {m_actions=[]; update_content();};
	O.addAction=function(act) {m_actions.push(act); update_content();};

	var m_actions=[];

	function update_content() {
		O.div().empty();
		var table=$('<table></table>'); O.div().append(table);
		var tr=$('<tr></tr>'); table.append(tr);
		for (var i=0; i<m_actions.length; i++) {
			var elmt=create_action_element(m_actions[i]);
			var td=$('<td></td>'); tr.append(td);
			td.append(elmt);
		}
	}

	function create_action_element(A) {
		console.log(A.url);
		var a=$('<a href="'+A.url+'" download="'+A.file_name+'"></a>');
		a.html(A.text);
		return a;
	}
}