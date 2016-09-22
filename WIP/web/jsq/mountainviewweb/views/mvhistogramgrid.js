function MVHistogramGrid(O,mvcontext,pair_mode) {
	O=O||this;
	MVAbstractView(O,mvcontext);

	O.prepareCalculation=function() {console.log ('prepareCalculation() should be overloaded.');};
	O.runCalculation=function(opts,callback) {console.log ('runCalculation() should be overloaded.');};
	O.onCalculationFinished=function() {console.log ('onCalculationFinished() should be overloaded.');};

	JSQ.connect(O,'sizeChanged',O,update_layout);
	JSQ.connect(mvcontext,'optionsChanged',O,O.recalculate);
	if (pair_mode) {
		JSQ.connect(mvcontext,'currentClusterPairChanged',O,do_highlighting_and_captions);
		JSQ.connect(mvcontext,'selectedClusterPairsChanged',O,do_highlighting_and_captions);
	}
	else {
		JSQ.connect(mvcontext,'currentClusterChanged',O,do_highlighting_and_captions);
		JSQ.connect(mvcontext,'selectedClustersChanged',O,do_highlighting_and_captions);	
	}

	//protected methods
	O.setHorizontalScaleAxis=function(opts) {setHorizontalScaleAxis(opts);};
	O.setHistogramViews=function(views) {setHistogramViews(views);};
	O.histogramViews=function() {return m_histogram_views;};

	var m_panel_widget=new MVPanelWidget();
	m_panel_widget.setParent(O);
	var m_num_columns=-1;
	var m_horizontal_scale_axis_data={use_it:false,label:'for exampe 100 ms'};
	var m_histogram_views=[];

	function update_layout() {
		var ss=O.size();
		m_panel_widget.setPosition([5,5]);
		m_panel_widget.setSize([ss[0]-10,ss[1]-10]);
	}
	function do_highlighting_and_captions() {
		var k=O.mvContext().currentCluster();
		var ks=O.mvContext().selectedClusters();
		for (var i=0; i<m_histogram_views.length; i++) {
			var HV=m_histogram_views[i];
			var k0=HV.property('k');
			if (k0==k) HV.div().addClass('current');
			else HV.div().removeClass('current');
			if (k0 in ks) HV.div().addClass('selected');
			else HV.div().removeClass('selected');
		}

		//TODO: highlight the current and selected pairs and set the captions
		/*
		var k=O.mvContext().currentCluster();
		var ks=O.mvContext().selectedClusters();
		for (var i=0; i<m_template_panels.length; i++) {
			var Y=m_template_panels[i];
			var k0=Y.property('k');
			if (k0==k) Y.div().addClass('current');
			else Y.div().removeClass('current');
			if (k0 in ks) Y.div().addClass('selected');
			else Y.div().removeClass('selected');
		}
		*/
	}
	function histogram_clicked(sender,modifiers) {
		var k=sender.property('k');
		var k1=sender.property('k1');
		var k2=sender.property('k2');
		if (!pair_mode)
			O.mvContext().clickCluster(k,modifiers);
		else {
			//O.mvContext().clickClusterPair(k1,k2,modifiers);
			O.mvContext().setCurrentCluster(-1);
			//O.mvContext().setCurrentClusterPair([k1,k2]);
			O.mvContext().setSelectedClusters(JSQ.toSet([k1,k2]));
		}
	}
	function setHorizontalScaleAxis(opts) {
		m_horizontal_scale_axis_data=JSQ.clone(opts);
	}
	function setHistogramViews(views) {
		m_histogram_views=views.slice();
		m_panel_widget.clearPanels();
		var NUM=views.length;
		var num_rows = Math.floor(Math.sqrt(NUM));
		if (num_rows < 1) num_rows=1;
		var num_cols=Math.floor((NUM+num_rows-1)/num_rows);
		m_num_columns=num_cols;

		for (var jj=0; jj<NUM; jj++) {
			var HV=views[jj];
			JSQ.connect(HV,'clicked',O,histogram_clicked);
			var row0=Math.floor(jj/num_cols);
			var col0=jj-row0*num_cols;
			m_panel_widget.addPanel(row0,col0,HV);
		}

		if (m_horizontal_scale_axis_data.use_it) {
			//TODO: do the scale axis
		}

		do_highlighting_and_captions();
	}

}