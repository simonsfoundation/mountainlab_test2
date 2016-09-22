function JQBrowserWindow(O) {
	if (!O) O=this;
	JQWidget(O);

	$(window).on('resize', function() {
		update_size();
	});
	function update_size() {
		O.setSize($(window).width(),$(window).height());
	}
	update_size();
}
