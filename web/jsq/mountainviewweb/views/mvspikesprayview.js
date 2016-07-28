function MVSpikeSprayView(O,mvcontext) {
	O=O||this;
	MVAbstractView(O,mvcontext);
	O.div().addClass('MVTemplatesView');

	O.prepareCalculation=function() {prepareCalculation();};
	O.runCalculation=function(opts,callback) {runCalculation(opts,callback);};
	O.onCalculationFinished=function() {onCalculationFinished();};

	O.loadStaticView=function(X) {loadStaticView(X);};

	JSQ.connect(O,'sizeChanged',O,update_layout);
	JSQ.connect(mvcontext,'optionsChanged',O,O.recalculate);
	JSQ.connect(mvcontext,'currentClusterChanged',O,update_highlighting);
	JSQ.connect(mvcontext,'selectedClustersChanged',O,update_highlighting);

	var m_panel_widget=new MVPanelWidget();
	m_panel_widget.setParent(O);
	var m_template_panels=[];
	var m_vscale_factor=2;
	var m_total_time_sec=0;

	m_panel_widget.onPanelClicked(panelClicked);

	var m_cluster_data=[];

	function update_layout() {
		var ss=O.size();
		m_panel_widget.setPosition([5,5]);
		m_panel_widget.setSize([ss[0]-10,ss[1]-10]);
	}
	function update_highlighting() {
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
	}

	function setTemplates(templates) {
		var M=templates.N1();
		var T=templates.N2();
		var K=templates.N3();
		m_cluster_data=[];
		for (var k=0; k<K; k++) {
			var CD={
				template0:templates.subArray(0,0,k,M,T,1),
				k:k+1
			};
			m_cluster_data.push(CD);
		}
		update_panels();
	}
	function update_panels() {
		m_panel_widget.clearPanels();
		m_template_panels=[];
		for (var k=0; k<m_cluster_data.length; k++) {
			var CD=m_cluster_data[k];
			if (m_total_time_sec)
				CD.firing_rate=CD.num_events/m_total_time_sec;
			else
				CD.firing_rate=0;
			var Y=new MVTemplatesViewPanel();
			Y.setProperty('k',CD.k);
			Y.setChannelColors(mvcontext.channelColors());
			Y.setClusterData(CD);
			m_panel_widget.addPanel(0,k,Y);
			m_template_panels.push(Y);
		}
		update_scale_factors();
	}
	function panelClicked(ind,modifiers) {
		if (ind in m_template_panels) {
			O.mvContext().clickCluster(m_template_panels[ind].property('k'),modifiers);
		}

	}
	function min_template_value() {
		var ret=0;
		for (var i=0; i<m_cluster_data.length; i++) {
			var CD=m_cluster_data[i];
			ret=Math.min(ret,CD.template0.minimum());
		}
		return ret;
	}
	function max_template_value() {
		var ret=0;
		for (var i=0; i<m_cluster_data.length; i++) {
			var CD=m_cluster_data[i];
			ret=Math.max(ret,CD.template0.maximum());
		}
		return ret;
	}
	function update_scale_factors() {
		var min0=min_template_value();
		var max0=max_template_value();
		var maxabs=Math.max(Math.abs(min0),Math.abs(max0));
		if (!maxabs) maxabs=1;
		var factor=1/maxabs*m_vscale_factor;
		for (var i=0; i<m_template_panels.length; i++) {
			m_template_panels[i].setVerticalScaleFactor(factor);
		}
	}
	function Calculator() {
		var that=this;

    	//inputs
    	this.mlproxy_url='';
    	this.firings='';
    	this.timeseries='';
    	this.clip_size=100;

    	//outputs
    	this.cluster_data=[];

    	this.loadStaticOutput=function(X) {loadStaticOutput(X);};

    	this.run=function(opts,callback) {
    		that.cluster_data=[];

	    	var X=new MountainProcessRunner();
	        X.setProcessorName("mv_compute_templates");
	        var params={};
	        params.timeseries = that.timeseries.path();
	        params.firings = that.firings.path();
	        params.clip_size = that.clip_size;
	        X.setInputParameters(params);
	        X.setMLProxyUrl(that.mlproxy_url);

	        var templates_fname = X.makeOutputFileUrl("templates");
	        var stdevs_fname = X.makeOutputFileUrl("stdevs");

	        X.runProcess(function(res) {
	            var templates=new RemoteReadMda();
	            templates.setPath(templates_fname);
	            templates.toMda(function(res) {
	                var templates0=res.mda;
	                var M=templates0.N1();
		            var T=templates0.N2();
		            var K=templates0.N3();
		            for (var k=0; k<K; k++) {
		            	var template0=templates0.subArray(0,0,k,M,T,1);
		            	var CD={};
		            	CD.template0=template0;
		            	CD.k=k+1;
		            	that.cluster_data.push(CD);
		            }
	                callback({success:true});
	            });
	        });
	    };
	    function loadStaticOutput(X) {
	    	that.cluster_data=[];
	    	for (var i=0; i<X.cluster_data.length; i++) {
	    		var CD=X.cluster_data[i];
	    		CD.stdev0=mda_from_base64(CD.stdev0);
	    		CD.template0=mda_from_base64(CD.template0);
	    		that.cluster_data.push(CD);
	    	}
	    }
	    function mda_from_base64(X) {
	    	var buf=Base64Binary.decode(X).buffer;
	    	var A=new Mda();
	    	A.setFromArrayBuffer(buf);
	    	return A;
	    }
    }
    var m_calculator=new Calculator();
    function prepareCalculation() {
    	if (!mvcontext.staticMode()) {
	    	m_calculator.mlproxy_url=mvcontext.mlProxyUrl();
	    	m_calculator.firings=mvcontext.firings();
	    	m_calculator.timeseries=mvcontext.currentTimeseries();
	    	m_calculator.clip_size=mvcontext.option('clip_size');
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
    	m_cluster_data=m_calculator.cluster_data;
    	update_panels();
    	//setTemplates(m_calculator.templates);
    }
    function loadStaticView(X) {
    	var calculator_output=X["calculator-output"]||{};
		m_calculator.loadStaticOutput(calculator_output);
		var info=X.info||{};
		m_total_time_sec=info.total_time_sec||0;
    	O.recalculate();
    }

	update_layout();
}

function MVTemplatesViewPanel(O) {
	O=O||this;
	JSQCanvasWidget(O);
	O.div().addClass('MVTemplatesViewPanel');

	this.setChannelColors=function(list) {m_channel_colors=JSQ.clone(list);};
	this.setClusterData=function(D) {m_CD=D;}; //don't clone because it has mda's
	this.setVerticalScaleFactor=function(factor) {
		m_vert_scale_factor=factor;
		O.update();
	};

	var m_vert_scale_factor=1;
	var m_channel_colors=[];
	var m_CD={};
	var m_top_rect = [0,0,0,0]; 
	var m_template_rect = [0,0,0,0];
	var m_bottom_rect = [0,0,0,0];

	O.onPaint(paint);

	function paint(painter) {
		var template0=m_CD.template0;
		var M=template0.N1();
		var T=template0.N2();
		var W0=O.width();
		var H0=O.height();
		var Tmid = Math.floor((T + 1) / 2) - 1;
		var top_height=20;
		var bottom_height=30;
		var xmargin=1,ymargin=8;

		var rect2=[0+xmargin,0+ymargin,W0-xmargin*2,H0-ymargin*2];
		m_top_rect=[rect2[0],rect2[1],rect2[2],top_height];
		m_template_rect=[rect2[0],rect2[1]+top_height,rect2[2],rect2[3]-bottom_height-top_height];		
		m_bottom_rect=[rect2[0],rect2[1]+rect2[3]-bottom_height,rect2[2],bottom_height];

		//firing rate disk
	    {
	    	var disksize=get_disksize_for_firing_rate(m_CD.firing_rate);
	    	console.log(m_CD.firing_rate);
	    	painter.setBrush({color:'lightgray'});
	    	var ww=Math.min(m_bottom_rect[2],m_bottom_rect[3])*disksize;
	    	var tmp=[m_bottom_rect[0],m_bottom_rect[1]+m_bottom_rect[3]-ww,ww,ww];
	    	tmp[0]=tmp[0]+(m_bottom_rect[2]-ww)/2;
	    	tmp[1]=tmp[1]-(m_bottom_rect[3]-ww)/2;
	    	console.log(tmp);
	    	painter.fillEllipse(tmp);
	    }

	    // group label
		{
			painter.setBrush({color:'gray'});
			var group_label=m_CD.k;
			var txt=group_label;
			var font=painter.font();
	        font["pixel-size"]=12;
	        painter.setFont(font);

	        var pen=painter.pen();
	        pen.width=1;
	        pen.color='darkblue';
	        painter.setPen(pen);
	        painter.drawText(m_top_rect, {AlignCenter:1,AlignBottom:1}, txt);
	    }

		{
			//the midline
			var view_background=[245,245,245];
			var midline_color=lighten(view_background,0.9);
			var pt0=coord2pix(0,Tmid,0);
			var pen=painter.pen(); pen.color=midline_color; pen.width=1;
			painter.setPen(pen);
			painter.drawLine(pt0[0],0,pt0[0],H0);
		}

		// render the templates
		for (var m=0; m<M; m++) {
			var col=get_channel_color(m+1);
			var pen=painter.pen(); pen.color=col; pen.width=1;
			painter.setPen(pen);
			{
				//the template
				var ppath=new JSQPainterPath();
				for (var t=0; t<T; t++) {
					var val=template0.value(m,t);
					var pt=coord2pix(m,t,val);
					if (t===0) ppath.moveTo(pt);
					else ppath.lineTo(pt);
				}
				painter.drawPath(ppath);
			}
		}

		
    }

    function get_disksize_for_firing_rate(firing_rate) {
    	return Math.min(1,Math.sqrt(firing_rate/10));
    }

    function coord2pix(m,t,val) {
    	var template0=m_CD.template0;
    	var M=template0.N1();
    	var T=template0.N2();
    	var W0=O.size()[0];
    	var H0=O.size()[1];
    	var pctx=0,pcty=0;
    	if (T) pctx=(t+0.5)/T;
    	if (M) pcty=(m+0.5-val*m_vert_scale_factor)/M;
    	var margx=4,margy=5;
    	var x0=m_template_rect[0]+margx+pctx*(m_template_rect[2]-margx*2);
    	var y0=m_template_rect[1]+margy+pcty*(m_template_rect[3]-margy*2);
    	return [x0,y0];
    }

	function lighten(col,val) {
		var ret=[col[0]*val,col[1]*val,col[2]*val];
		ret=[Math.min(255,ret[0]),Math.min(255,ret[1]),Math.min(255,ret[2])];
		return ret;
	}

	function get_channel_color(m) {
		if (m <= 0)
	        return [0,0,0];
	    if (m_channel_colors.length===0)
	        return [0,0,0];
	    return m_channel_colors[(m - 1) % m_channel_colors.length];	
	}
}


/*
function MVTemplatesViewPanel(O) {
	O=O||this;
	JSQCanvasWidget(O);
	O.div().addClass('MVTemplatesViewPanel');

	this.setChannelColors=function(list) {m_channel_colors=JSQ.clone(list);};
	this.setTemplate=function(template) {m_template=template; O.update();};
	this.setVerticalScaleFactor=function(factor) {
		m_vert_scale_factor=factor;
		O.update();
	};

	O.onPaint(paint);

	function paint(painter) {
		var M=m_template.N1();
		var T=m_template.N2();
		var W0=O.width();
		var H0=O.height();
		var Tmid = Math.floor((T + 1) / 2) - 1;

		{
			//the midline
			var view_background=[245,245,245];
			var midline_color=lighten(view_background,0.9);
			var pt0=coord2pix(0,Tmid,0);
			var pen=painter.pen(); pen.color=midline_color; pen.width=1;
			painter.setPen(pen);
			painter.drawLine(pt0[0],0,pt0[0],H0);
		}

		for (var m=0; m<M; m++) {
			var col=get_channel_color(m+1);
			var pen=painter.pen(); pen.color=col; pen.width=1;
			painter.setPen(pen);
			{
				//the template
				var ppath=new JSQPainterPath();
				for (var t=0; t<T; t++) {
					var val=m_template.value(m,t);
					var pt=coord2pix(m,t,val);
					if (t===0) ppath.moveTo(pt);
					else ppath.lineTo(pt);
				}
				painter.drawPath(ppath);
			}
		}
    }

    function coord2pix(m,t,val) {
    	var M=m_template.N1();
    	var T=m_template.N2();
    	var W0=O.size()[0];
    	var H0=O.size()[1];
    	var pctx=0,pcty=0;
    	if (T) pctx=(t+0.5)/T;
    	if (M) pcty=(m+0.5-val*m_vert_scale_factor)/M;
    	var margx=4,margy=5;
    	var x0=margx+pctx*(W0-margx*2);
    	var y0=margy+pcty*(H0-margy*2);
    	return [x0,y0];
    }

	var m_template=new Mda();
	var m_vert_scale_factor=1;
	var m_channel_colors=[];

	function lighten(col,val) {
		var ret=[col[0]*val,col[1]*val,col[2]*val];
		ret=[Math.min(255,ret[0]),Math.min(255,ret[1]),Math.min(255,ret[2])];
		return ret;
	}

	function get_channel_color(m) {
		if (m <= 0)
	        return [0,0,0];
	    if (m_channel_colors.length===0)
	        return [0,0,0];
	    return m_channel_colors[(m - 1) % m_channel_colors.length];	
	}
}

*/