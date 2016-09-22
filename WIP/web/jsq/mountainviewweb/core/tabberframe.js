function TabberFrame(O,W) {
	O=O||this;
	JSQWidget(O);
	O.div().addClass('TabberFrame');
	W.setParent(O);
	JSQ.connect(O,'sizeChanged',O,update_layout);

	O.view=function() {return W;};
	O.setContainerName=function(container_name) {m_container_name=container_name;};

	update_layout();
	function update_layout() {
		W.setSize(O.size());
		W.setPosition(0,0);
	}
}