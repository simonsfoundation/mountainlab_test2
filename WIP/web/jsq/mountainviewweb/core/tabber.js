function Tabber(O) {
	O=O||this;
	JSQObject(O);

	O.createTabWidget=function(container_name) {return createTabWidget(container_name);};
	O.addWidget=function(container_name,label,W) {addWidget(container_name,label,W);};

	var m_tab_widgets={};
	var m_widgets=[];

	function createTabWidget(container_name) {
		var X=new JSQTabWidget();
		m_tab_widgets[container_name]=X;
		return X;
	}
	function addWidget(container_name,label,W) {
		var X={
			frame:new TabberFrame(0,W),
			widget:W,
			label:label
		};
		m_widgets.push(X);
		put_widget_in_container(container_name,X.widget);
		O.emit('widgets-changed');
	}
	function find_tabber_widget(W) {
		for (var i=0; i<m_widgets.length; i++) {
			if (m_widgets[i].widget.objectId()==W.objectId()) {
				return m_widgets[i];
			}
		}
		return null;
	}
	function put_widget_in_container(container_name,W) {
		var X=find_tabber_widget(W);
		if (!X) return;
		if (container_name) {
			if (!(container_name in m_tab_widgets)) {
				console.error('Unable to find container: '+container_name);
				return;
			}
			m_tab_widgets[container_name].addTab(X.frame,X.label);
		}
		else {
			console.error('Container name is empty');
			return;
		}
		X.container_name=container_name;
		X.frame.setContainerName(container_name);
	}
}