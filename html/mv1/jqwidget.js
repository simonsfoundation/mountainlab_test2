function JQWidget(O) {
	if (!O) O=this;
	JObject(O);
	
	O.div=function() {return m_div;}
	O.size=function() {return m_size;}
	O.setSize=function(size) {setSize(size);}
	O.position=function() {return m_position;}
	O.setPosition=function(pos) {setPosition(pos);}
	O.showFullBrowser=function() {showFullBrowser();}

	function setSize(size) {
		if ((size[0]==m_size[0])&&(size[1]==m_size[1])) {
			return;
		}
		m_size[0]=size[0];
		m_size[1]=size[1];
		set_div_geom();
		O.emit('sizeChanged');
	}
	function setPosition(pos) {
		if ((m_position[0]==pos[0])&&(m_position[1]==pos[1])) {
			return;
		}
		m_position[0]=pos[0];
		m_position[1]=pos[1];
		set_div_geom();
		O.emit('positionChanged');
	}
	function showFullBrowser() {
		var X=new JQBrowserWindow();
		JQCallback(X,'sizeChanged',function(sender,params) {
			O.setSize(X.size());
		});
	}

	O._set_is_widget(true);
	var m_div=$('<div></div>');
	var m_position=[0,0];
	var m_size=[0,0];
	set_div_size();

	function set_div_geom() {
		m_div.css({
			position:'absolute',
			left:m_position[0],
			top:m_position[1],
			width:m_size[0],
			height:m_size[1]
		})
	}
}