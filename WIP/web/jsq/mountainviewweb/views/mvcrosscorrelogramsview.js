function MVCrossCorrelogramsView(O,mvcontext,mode) {
	O=O||this;

	var pair_mode=false;
	if (mode=='Matrix_Of_Cross_Correlograms') pair_mode=true;

	MVHistogramGrid(O,mvcontext,pair_mode);
	O.div().addClass('MVCrossCorrelogramsView');

	O.prepareCalculation=function() {prepareCalculation();};
	O.runCalculation=function(opts,callback) {runCalculation(opts,callback);};
	O.onCalculationFinished=function() {onCalculationFinished();};

	O.loadStaticView=function(X) {loadStaticView(X);};

	var m_correlograms=[];

	var m_calculator=new MVCrossCorrelogramsViewCalculator();
	function prepareCalculation() {
		if (!mvcontext.staticMode()) {
			m_calculator.mlproxy_url=mvcontext.mlProxyUrl();
			m_calculator.firings=mvcontext.firings();
			m_calculator.max_dt=Number(mvcontext.option('cc_max_dt_msec',100))/1000*mvcontext.sampleRate();
			m_calculator.mode=mode;
			m_calculator.ks=JSQ.numSet2List(mvcontext.selectedClusters());
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
		m_correlograms=m_calculator.correlograms;

		var bin_max=max2(m_correlograms);
		var bin_min=-bin_max;
		var bin_size=20;
		var num_bins=Math.floor((bin_max-bin_min)/bin_size);
		if (num_bins<100) num_bins=100;
		if (num_bins>2000) num_bins=2000;
		var sample_freq=mvcontext.sampleRate();
		var time_width=(bin_max-bin_min)/sample_freq*1000;

		var histogram_views=[];
		for (var ii=0; ii<m_correlograms.length; ii++) {
			var k1=m_correlograms[ii].k1;
			var k2=m_correlograms[ii].k2;
			var HV=new HistogramView();
			HV.setData(m_correlograms[ii].data);
			//set colors
			HV.setBins(bin_min,bin_max,num_bins);
			HV.setProperty('k',k1);
			HV.setProperty('k1',k1);
			HV.setProperty('k2',k2);
			if (pair_mode) {
				HV.setTitle(k1+'/'+k2);
			}
			else {
				HV.setTitle(k1);
			}
			histogram_views.push(HV);
		}
    	O.setHistogramViews(histogram_views);
	}

	function loadStaticView(X) {
		var calculator_output=X["computer-output"]||{};
		m_calculator.loadStaticOutput(calculator_output);
	    //CrossCorrelogramOptions3 opts;
	    //opts.fromJsonObject(X["options"].toObject());
	    //this->setOptions(opts);
	    //HistogramView::TimeScaleMode tsm;
	    //from_string(tsm,X["time-scale-mode"].toString());
	    //this->setTimeScaleMode(tsm);
    	O.recalculate();
	}

	function max2(correlograms) {
		var ret=0;
		for (var i=0; i<correlograms.length; i++) {
			var H=correlograms[i];
			for (var j=0; j<H.data.length; j++) {
				ret=Math.max(ret,H.data[j]);
			}
		}
		return ret;
	}
	
}

function MVCrossCorrelogramsViewCalculator() {
	var that=this;

	//input
	this.mlproxy_url='';
	this.firings=new RemoteReadMda();
	this.max_dt=0;
	this.mode='All_Auto_Correlograms';
	this.ks=[];

	//output
	this.loaded_from_static_output=false;
	this.correlograms=[];

	this.loadStaticOutput=function(X) {loadStaticOutput(X);};

	this.run=function(opts,callback) {
		that.correlograms=[];
		that.firings.toMda(function(res) {
			if (!res.success) {
				console.log (that.firings.path());
				console.error(res.error);
				return;
			}
			var firings0=res.mda;
			var L=firings0.N2();
			var K=1;
			var times=[],labels=[];
			for (var i=0; i<L; i++) {
				var time0=firings0.value(1,i);
				var label0=firings0.value(2,i);
				if (label0>K) K=label0;
				times.push(time0);
				labels.push(label0);
			}

			//Assemble the correlogram objects depending on mode
		    if (that.mode == 'All_Auto_Correlograms') {
		        for (var k = 1; k <= K; k++) {
		            var CC={k1:k,k2:k};
		            that.correlograms.push(CC);
		        }
		    }
		    else if (that.mode == 'Matrix_Of_Cross_Correlograms') {
		    	for (var i=0; i<that.ks.length; i++) {
		    		for (var j=0; j<that.ks.length; j++) {
		    			var CC={k1:that.ks[i],k2:that.ks[j]};
		    			that.correlograms.push(CC);
		    		}
		    	}
		    }

		    //assemble the times organized by k
		    var the_times=[];
		    for (var k=0; k<=K; k++) {
		    	the_times.push([]);
		    }
		    for (var ii=0; ii<labels.length; ii++) {
		    	var k=labels[ii];
		    	if (k<=the_times.length) {
		    		the_times[k].push(times[ii]);
		    	}
		    }

		    //compute the cross-correlograms
		    for (var j=0; j<that.correlograms.length; j++) {
		    	var k1=that.correlograms[j].k1;
		    	var k2=that.correlograms[j].k2;
		    	that.correlograms[j].data=compute_cc_data(the_times[k1],the_times[k2],that.max_dt,(k1==k2));
		    }

			callback({success:true});
		});
	};

	function compute_cc_data(times1_in,times2_in,max_dt,exclude_matches) {
		var ret=[];
		var times1=times1_in.slice();
		var times2=times2_in.slice();
		JSQ.numSort(times1);
		JSQ.numSort(times2);

		if ((times1.length===0)||(times2.length===0)) return ret;

		var i1=0;
		for (var i2=0; i2<times2.length; i2++) {
			while ((i1+1<times1.length)&&(times1[i1]<times2[i2]-max_dt)) i1++;
			var j1=i1;
			while ((j1<times1.length)&&(times1[j1]<=times2[i2]+max_dt)) {
				var ok=true;
				if ((exclude_matches)&&(j1==i2)&&(times1[j1]==times2[i2])) ok=false;
				if (ok) {
					ret.push(times1[j1]-times2[i2]);
				}
				j1++;
			}
		}
		return ret;
	}

	function loadStaticOutput(X) {
		that.correlograms=JSQ.clone(X.correlograms||[]);
		for (var i=0; i<that.correlograms.length; i++) {
			var buf=Base64Binary.decode(that.correlograms[i].data).buffer;
			var data0=new Float64Array(buf.byteLength/8);
			var view=new DataView(buf);
			for (var j=0; j<data0.length; j++) { // can this be improved?
				data0[j]=view.getFloat64(j*8, false); //little/big endian? (true/false)
			}
			that.correlograms[i].data=data0;
		}
    	that.loaded_from_static_output=true;
	}
}
