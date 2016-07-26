function HistogramView(O) {
	O=O||this;
	JSQCanvasWidget(O);

	O.setData=function(data) {m_data=data; m_update_required=true;};
	O.setSecondData=function(data) {m_second_data=data.slice();};
	O.setBins=function(bin_min,bin_max,num_bins) {setBins(bin_min,bin_max,num_bins);};
	O.autoSetBins=function(num_bins) {autoSetBins(num_bins);};
    O.setDrawVerticalAxisAtZero=function(val) {m_draw_vertical_axis_at_zero=val;};
    O.setXRange=function(min0,max0) {m_xrange=[min0,max0]; O.update();};
    O.autoCenterXRange=function() {autoCenterXRange();};
    O.setTitle=function(title) {m_title=title; O.update();};

    O.onMouseRelease(function(evt) {O.emit('clicked',evt.modifiers);});
	
	O.onPaint(paint);

	var m_data=[];
	var m_second_data=[];
	var m_num_bins=0;
	var m_bin_centers=[];
	var m_bin_counts=[];
	var m_second_bin_counts=[];
	var m_update_required=false;
	var m_max_bin_count=0;
	var m_margin_left=5,m_margin_right=5,m_margin_top=5,m_margin_bottom=15;
	var m_xrange=[-1,-1];
	var m_fill_color=[100,100,150];
	var m_line_color=[150,150,150];
	var m_hovered_bin_index=-1;
	var m_title='';
	var m_caption='';
	var m_draw_vertical_axis_at_zero=false;

	function setBins(bin_min,bin_max,num_bins) {
		m_bin_centers=[];
		m_bin_counts=[];
		m_second_bin_counts=[];
		if (num_bins<=1) return;

		m_num_bins=num_bins;
	    for (var i=0; i<num_bins; i++) {
	    	m_bin_counts.push(0);
	    	m_second_bin_counts.push(0);
	    	m_bin_centers.push(bin_min+(bin_max-bin_min)*i/(num_bins-1));
	    }
	    m_update_required=true;
	}
	function autoSetBins(num_bins) {
		if (m_data.length<=1) return;
    	var data_min=m_data[0],data_max=m_data[0];
    	for (var i=0; i<m_data.length; i++) {
    		if (m_data[i]<data_min) data_min=m_data[i];
    		if (m_data[i]>data_max) data_max=m_data[i];
    	}
    	if (data_max==data_min) {
    		data_min-=1;
    		data_max+=1;
    	}
    	var bin_min=data_min;
    	var bin_max=data_max;
    	O.setBins(bin_min,bin_max,num_bins);
    }

	function update_bin_counts() {
        if (m_bin_centers.length<2) {
            return;
        }
		for (var i=0; i<m_num_bins; i++) {
			m_bin_counts[i]=0;
			m_second_bin_counts[i]=0;
		}
		m_max_bin_count=0;
    	for (var pass=1; pass<=2; pass++) {
    		var list=[];
    		if (pass==1) {
    			for (var i=0; i<m_data.length; i++) {
    				list.push(m_data[i]);
    			}
    		}
    		else {
    			list=m_second_data.slice();
    		}
    		//list.sort(); //javascript does an alphabetical sort by default, even when data are numbers, which is horrible!!!!!! (wasted half a day)
            JSQ.numSort(list);
    		if (m_num_bins<1) return;
    		var spacing=m_bin_centers[1]-m_bin_centers[0];
    		var jj=0;
    		for (var i=0; i<list.length; i++) {
    			var val=list[i];
    			while ((jj+1<m_num_bins)&&(m_bin_centers[jj]+spacing/2<val)) jj++;
    			if ((val>=m_bin_centers[jj]-spacing/2)&&(val<=m_bin_centers[jj]+spacing/2)) {
    				if (pass==1) m_bin_counts[jj]++;
    				else m_second_bin_counts[jj]++;
    			}
    		}
    		for (var i=0; i<m_num_bins; i++) {
    			if (pass==1) {
    				if (m_bin_counts[i]>m_max_bin_count) m_max_bin_count=m_bin_counts[i];
    			}
    			else {
    				if (m_second_bin_counts[i]>m_max_bin_count) m_max_bin_count=m_second_bin_counts[i];
    			}
    		}
    	}
    }

	function paint(painter) {
		var W=O.width();
		var H=O.height();
		if (m_update_required) {
			update_bin_counts();
			m_update_required=false;
		}
		if (m_num_bins<=1) return;
		var spacing=m_bin_centers[1]-m_bin_centers[0];
    	for (var pass=1; pass<=2; pass++) {
    		var bin_counts=[];
    		if (pass==1) bin_counts=m_bin_counts;
    		else if (pass==2) bin_counts=m_second_bin_counts;
    		var col=m_fill_color;
    		var line_color=m_line_color;
    		if (pass==2) {
    			col=modify_color_for_second_histogram(col);
    			line_color=modify_color_for_second_histogram(line_color);
    		}
    		for (var i=0; i<m_num_bins; i++) {
    			var pt1=coord2pix([m_bin_centers[i]-spacing/2,0],W,H);
    			var pt2=coord2pix([m_bin_centers[i]+spacing/2,bin_counts[i]],W,H);
    			var R=make_rect(pt1,pt2);
    			if (i==m_hovered_bin_index)
    				painter.fillRect(R[0],R[1],R[2],R[3],lighten(col,25,25,25));
    			else
    				painter.fillRect(R[0],R[1],R[2],R[3],col);
    			var pen=painter.pen();
    			pen.color=line_color;
    			painter.setPen(pen);
    			painter.drawRect(R[0],R[1],R[2],R[3]);
    		}
    	}
        if (m_title.length>0) {
            var text_height=14;
            var R=[m_margin_left,5,W-m_margin_left-m_margin_right,text_height];
            var font=painter.font();
            font.family='Arial'; font["pixel-size"]=text_height;
            painter.setFont(font);
            var pen=painter.pen();
            pen.color=[100,60,60];
            painter.setPen(pen);
            painter.drawText(R,{AlignLeft:1,AlignTop:1},m_title);
        }
    }

    function coord2pix(coord,W,H) {
    	if (!W) W=O.width();
    	if (!H) H=O.height();
        if (m_num_bins<=1) return [0,0];
        if ((!W)||(!H)) return [0,0];

    	if (W<=m_margin_left+m_margin_right+5) return [0,0];
    	if (H<=m_margin_top+m_margin_bottom+5) return [0,0];
    	var spacing=m_bin_centers[1]-m_bin_centers[0];
    	var xmin=m_bin_centers[0]-spacing/2;
    	var xmax=m_bin_centers[m_num_bins-1]+spacing/2;
    	var ymin=0;
    	var ymax=m_max_bin_count;
    	if (m_xrange[0]!=m_xrange[1]) {
    		xmin=m_xrange[0];
    		xmax=m_xrange[1];
    	}

    	var xfrac=0.5;
    	if (xmax>xmin) xfrac=(coord[0]-xmin)/(xmax-xmin);
    	var yfrac=0.5;
    	if (ymax>ymin) yfrac=(coord[1]-ymin)/(ymax-ymin);

    	var x0=m_margin_left+xfrac*(W-m_margin_left-m_margin_right);
    	var y0=H-(m_margin_bottom+yfrac*(H-m_margin_top-m_margin_bottom));

    	return [x0,y0];
    }

    function compute_mean(X) {
        if (X.length==0) return 0;
        var sum=0;
        for (var i=0; i<X.length; i++) {
            sum+=X[i];
        }
        return sum/X.length;
    }

    function autoCenterXRange() {
        var mean_value=compute_mean(m_data);
        var xrange=JSQ.clone(m_xrange);
        var center1=(xrange[0]+xrange[1])/2;
        xrange=[xrange[0]+mean_value-center1,xrange[1]+mean_value-center1];
        O.setXRange(xrange[0],xrange[1]);
    }

    function modify_color_for_second_histogram(col) {
	    var r=col[0],g=col[1],b=col[2];
	    g=Math.min(255,g+30); //more green
	    var ret=[r,g,b];
	    return lighten(ret,-20,-20,-20); //darker
	}

    function lighten(col,dr,dg,db) {
    	var r=Math.min(255,col[0]+dr);
    	var g=Math.min(255,col[1]+dg);
    	var b=Math.min(255,col[2]+db);
    	return [r,g,b];
    }

	function make_rect(p1,p2) {
		var x=Math.min(p1[0],p2[0]);
		var y=Math.min(p1[1],p2[1]);
		var w=Math.abs(p2[0]-p1[0]);
		var h=Math.abs(p2[1]-p1[1]);
		return [x,y,w,h];
	}
}