function PriptView() {
	this.setClient=function(client) {m_client=client;};
	this.loadScript=load_script;
	this.loadProcess=load_process;
	this.div=function() {return m_div;};
	this.setGeom=setGeom;

	var m_div=$('<div></div>');
	
	setGeom(800,800);
	function setGeom(x,y,W,H) {
		absgeom(m_div,x,y,W,H);
	}

	m_div.css('overflow','scroll');

	var m_client=null;
	var m_pript={};

	function load_script(key) {
		console.log('test A');
		if (!m_client) return;
		console.log('test B');
		m_client.getScript(key,function(resp) {
			console.log('test C');
    		if (resp.success) {
    			m_pript=resp.script;
        		refresh();
        	}
        	else {
        		m_pript={error:'ERR: '+resp.error};
        	}
    	});
	}

	function load_process(key) {
		if (!m_client) return;
		m_client.getProcess(key,function(resp) {
    		if (resp.success) {
    			m_pript=resp.process;
        		refresh();
        	}
        	else {
        		m_pript={error:'ERR: '+resp.error};
        	}
    	});
	}

	function refresh() {
		m_div.html('TEST: '+JSON.stringify(m_pript));
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