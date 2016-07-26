function MVAbstractView(O,mvcontext) {
	O=O||this;
	JSQWidget(O);

	O.mvContext=function() {return mvcontext;};
	O.recalculate=function() {recalculate();};

	O.prepareCalculation=function() {console.log ('prepareCalculation() should be overloaded.');};
	O.runCalculation=function(opts,callback) {console.log ('runCalculation() should be overloaded.');};
	O.onCalculationFinished=function() {console.log ('onCalculationFinished() should be overloaded.');};

	function recalculate() {
		O.prepareCalculation();
		show_calculating_message();
	    O.runCalculation({},function(res) {
	    	hide_calculating_message();
	        O.onCalculationFinished();
	    });
	}

	var calculating_div=$('<div><h2>Calculating...</h2></div>')
	function show_calculating_message() {
		if (O.parentWidget()) {
			O.parentWidget().div().append(calculating_div);
			calculating_div.css({
				position:'absolute',
				width:O.width(),
				height:O.height(),
				left:O.left(),
				top:O.top()
			});
		}
		//O.setVisible(false);
	}
	function hide_calculating_message() {
		calculating_div.remove();
		//O.setVisible(true);
	}
}
