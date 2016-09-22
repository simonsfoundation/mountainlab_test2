function MVAbstractControlWidget(O,mvcontext,main_window) {
	O=O||this;
	JSQWidget(O);
	O.div().addClass('MVAbstractControlWidget');

	O.mvContext=function() {return mvcontext;};
	O.mainWindow=function() {return main_window;};

	O.title=function() {return 'title should be overridden'};
	O.updateContext=function() {console.log ('updateContext should be overridden');};
	O.updateControls=function() {console.log ('updateControls should be overridden');};

	//protected
	O.controlValue=function(name) {return controlValue(name);};
	O.setControlValue=function(name,val) {setControlValue(name,val);};
	O.setControlEnabled=function(name,val) {setControlEnabled(name,val);};
	O.createIntControl=function(name) {return createIntControl(name);};
	O.createDoubleControl=function(name) {return createIntControl(name);};
	O.createButtonControl=function(name,label,callback) {return createButtonControl(name,label,callback);};

	O.updateControlsOn=function(sender,signal) {JSQ.connect(sender,signal,O,O.updateControls);};

	function controlValue(name) {
		if  (name in m_int_controls) return Number(m_int_controls[name].val());
		if  (name in m_double_controls) return Number(m_double_controls[name].val());
		return 0;
	}
	function setControlValue(name,val) {
		if (name in m_int_controls) m_int_controls[name].val(val);
		if (name in m_double_controls) m_double_controls[name].val(val);
	}
	function setControlEnabled(name,val) {
		//TODO
	}
	function createIntControl(name) {
		var ret=$('<input type=text></input>');
		m_int_controls[name]=ret;
		return ret;
	}
	function createDoubleControl(name) {
		var ret=$('<input type=text></input>');
		m_double_controls[name]=ret;
		return ret;
	}
	function createButtonControl(name,label,callback) {
		var ret=$('<button>'+label+'</button>');
		ret.click(callback);
		m_button_controls[name]=ret;
		return ret;
	}

	var m_int_controls=[];
	var m_double_controls=[];
	var m_button_controls=[];
}