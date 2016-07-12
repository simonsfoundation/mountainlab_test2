function test1() {
	var X=new JQBrowserWindow();
	JQCallback(X,'sizeChanged',function(sender,params) {
		console.log(sender);
		console.log(params);
		console.log(X.size());
	});

	var Y=new JQWidget();
	Y.div().append('<h3>Testing!!!</h3>');
	Y.showFullBrowser();
}