function MVAmpHistView(O,mvcontext) {
	O=O||this;
	MVHistogramGrid(O,mvcontext);

	O.prepareCalculation=function() {prepareCalculation();};
	O.runCalculation=function(opts,callback) {runCalculation(opts,callback);};
	O.onCalculationFinished=function() {onCalculationFinished();};

	var m_histograms=[];

	var m_calculator=new MVAmpHistViewCalculator();
	function prepareCalculation() {
		if (!mvcontext.staticMode()) {
			m_calculator.firings=mvcontext.firings();
		}
	}
	function runCalculation(opts,callback) {
		if (!mvcontext.staticMode()) {
			m_calculator.run(opts,callback);
		}
		else {
			callback({success:true});
		}
	}
	function onCalculationFinished() {
		m_histograms=m_calculator.histograms;
		var bin_min=compute_min2(m_histograms);
		var bin_max=compute_max2(m_histograms);
    	var max00=Math.max(Math.abs(bin_min),Math.abs(bin_max));
    	var num_bins=200; //how to choose this?

    	var views=[];
    	for (var ii=0; ii<m_histograms.length; ii++) {
    		var k0=m_histograms[ii].k;
    		//if (mvcontext.clusterIsVisible(k0)) {
    			var HV=new HistogramView();
    			HV.setData(m_histograms[ii].data);
    			//set colors
    			HV.setBins(bin_min,bin_max,num_bins);
    			HV.setDrawVerticalAxisAtZero(true);
    			HV.setXRange(-max00,max00);
    			HV.autoCenterXRange();
    			HV.setProperty('k',k0);

    			views.push(HV);
    		//}
    	}
    	O.setHistogramViews(views);
	}

	function compute_min2(histograms) {
		var ret=0;
		for (var i=0; i<histograms.length; i++) {
			var H=histograms[i];
			for (var j=0; j<H.data.length; j++) {
				ret=Math.min(ret,H.data[j]);
			}
		}
		return ret;
	}
	function compute_max2(histograms) {
		var ret=0;
		for (var i=0; i<histograms.length; i++) {
			var H=histograms[i];
			for (var j=0; j<H.data.length; j++) {
				ret=Math.max(ret,H.data[j]);
			}
		}
		return ret;
	}
}

function MVAmpHistViewCalculator() {
	var that=this;

	//input
	this.firings=new RemoteReadMda();

	//output
	this.histograms=[];

	this.run=function(opts,callback) {
		that.histograms=[];
		that.firings.toMda(function(res) {
			if (!res.success) {
				console.log (that.firings.path());
				console.error(res.error);
				return;
			}
			var firings0=res.mda;
			var L=firings0.N2();
			var K=1;
			for (var i=0; i<L; i++) {
				var label0=firings0.value(2,i);
				if (label0>K) K=label0;
			}
			for (var k=1; k<=K; k++) {
				var HH={k:k,data:[]};
				that.histograms.push(HH);
			}
			for (var i=0; i<L; i++) {
				var label0=firings0.value(2,i);
				var amp0=firings0.value(3,i);
				if ((label0>=1)&&(label0<=K)) {
					that.histograms[label0-1].data.push(amp0);
				}
			}
			callback({success:true});
		});
	}
}