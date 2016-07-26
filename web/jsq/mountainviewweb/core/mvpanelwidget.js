function MVPanelWidget(O) {
	if (!O) O=this;
	JSQWidget(O);

	O.clearPanels=function() {clearPanels();};
	O.addPanel=function(row,col,W) {addPanel(row,col,W);};
	O.rowCount=function() {return rowCount();};
	O.columnCount=function() {return columnCount();};
	O.setSpacing=function(row_spacing,col_spacing) {m_row_spacing=row_spacing; m_col_spacing=m_col_spacing; update_layout();};
	O.setMargins=function(row_margin,col_margin) {m_row_margin=row_margin; m_col_margin=col_margin;};

	O.onPanelClicked=function(handler) {JSQ.connect(O,'panel_clicked',O,function(sender,args) {handler(args.ind,args.modifiers);});};

	O.onMousePress(mousePress);
	O.onMouseRelease(mouseRelease);
	O.onMouseMove(mouseMove);
	O.onMouseEnter(mouseEnter);
	O.onMouseLeave(mouseLeave);
	O.onWheel(wheel);

	var m_panels=[];
	var m_row_margin=3;
	var m_row_spacing=3;
	var m_col_margin=3;
	var m_col_spacing=3;
	var m_viewport_geom=[0,0,1,1];
	var m_current_panel_index=-1;

	JSQ.connect(O,'sizeChanged',O,update_layout);
	function update_layout() {
		correct_viewport_geom();
		var num_rows=O.rowCount();
		var num_cols=O.columnCount();
		if (!num_rows) return;
		if (!num_cols) return;
		var VP_W=O.size()[0];
		var VP_H=O.size()[1];
		var VG=m_viewport_geom;
		vgeom=[VG[0]*VP_W,VG[1]*VP_H,VG[2]*VP_W,VG[3]*VP_H];
		var W0=(vgeom[2]-(num_cols-1)*m_col_spacing-m_col_margin*2)/num_cols;
		var H0=(vgeom[3]-(num_rows-1)*m_row_spacing-m_row_margin*2)/num_rows;
		for (var i=0; i<m_panels.length; i++) {
			var x1=vgeom[0] + m_col_margin + m_panels[i].col*(W0+m_col_spacing);
			var y1=vgeom[1] + m_row_margin + m_panels[i].row*(H0+m_row_spacing);
			
			m_panels[i].W.setSize([W0,H0]);
			m_panels[i].W.setPosition([x1,y1]);
			m_panels[i].pixel_geom=[x1,y1,W0,H0];
		}
	}
	function correct_viewport_geom() {
		if (m_viewport_geom[2]<1) {
			m_viewport_geom[0]=0;
			m_viewport_geom[2]=1;
		}
		if (m_viewport_geom[0]+m_viewport_geom[2]<1) {
			m_viewport_geom[0]=1-m_viewport_geom[2];
		}
		if (m_viewport_geom[0]>0) {
			m_viewport_geom[0]=0;
		}
		if (m_viewport_geom[3]<1) {
			m_viewport_geom[1]=0;
			m_viewport_geom[3]=1;
		}
		if (m_viewport_geom[1]+m_viewport_geom[3]<1) {
			m_viewport_geom[1]=1-m_viewport_geom[3];
		}
		if (m_viewport_geom[1]>0) {
			m_viewport_geom[1]=0;
		}
	}
	function clearPanels() {
		for (var i=0; i<m_panels.length; i++) {
			m_panels[i].W.destroy();
		}
		m_panels=[];
		update_layout();
	}
	function addPanel(row,col,W) {
		var panel={row:row,col:col,W:W,pixel_geom:[0,0,0,0]};
		m_panels.push(panel);
		O.div().append(W.div());
		W.div().addClass("mvpanelwidget_panel");
		/// TODO, use a queued signal so we don't update layout after every add or resize
		update_layout();
	}
	function rowCount() {
		var ret=0;
		for (var i=0; i<m_panels.length; i++) {
			if (m_panels[i].row+1>ret) ret=m_panels[i].row+1;
		}
		return ret;
	}
	function columnCount() {
		var ret=0;
		for (var i=0; i<m_panels.length; i++) {
			if (m_panels[i].col+1>ret) ret=m_panels[i].col+1;
		}
		return ret;
	}
	function zoom(factor) {
		var current_panel_geom=[0,0,1,1];
		if ((m_current_panel_index>=0)&&(m_current_panel_index<m_panels.length)) {
			current_panel_geom=JSQ.clone(m_panels[m_current_panel_index].pixel_geom);
		}

		if (rowCount()>1) {
			m_viewport_geom[3]=m_viewport_geom[3]/factor;
		}
		if (columnCount()>1) {
			m_viewport_geom[2]=m_viewport_geom[2]/factor;
		}
		update_layout();

		var new_current_panel_geom=[0,0,1,1];
		if ((m_current_panel_index>=0)&&(m_current_panel_index<m_panels.length)) {
			new_current_panel_geom=JSQ.clone(m_panels[m_current_panel_index].pixel_geom);
		}
		var dx=new_current_panel_geom[0]+new_current_panel_geom[2]/2-current_panel_geom[0]-current_panel_geom[2]/2;
		var dy=new_current_panel_geom[1]+new_current_panel_geom[3]/2-current_panel_geom[1]-current_panel_geom[3]/2;
		if ((dx)||(dy)) {
			m_viewport_geom[0]-=dx/O.size()[0];
			m_viewport_geom[1]-=dy/O.size()[1];
			update_layout();
		}
		correct_viewport_geom();
	}
	function set_current_panel_index(ind) {
		if (m_current_panel_index==ind) return;
		m_current_panel_index=ind;
		for (var ii=0; ii<m_panels.length; ii++) {
			if (ii==ind) {
				m_panels[ii].W.div().addClass("mvpanelwidget_current_panel");
			}
			else {
				m_panels[ii].W.div().removeClass("mvpanelwidget_current_panel");	
			}
		}

	}
	function rect_contains(rect,pt) {
		if ((pt[0]<rect[0])||(pt[0]>=rect[0]+rect[2])) return false;
		if ((pt[1]<rect[1])||(pt[1]>=rect[1]+rect[3])) return false;
		return true;
	}
	function panel_index_at(pos) {
		for (var i=0; i<m_panels.length; i++) {
			if (rect_contains(m_panels[i].pixel_geom,pos)) return i;
		}
		return -1;
	}

	var press_anchor=[-1,-1];
	var press_anchor_viewport_geom=[0,0,1,1];
	var is_dragging=false;
	function mousePress(evt) {
		press_anchor=JSQ.clone(evt.pos);
		press_anchor_viewport_geom=JSQ.clone(m_viewport_geom);
		is_dragging=false;
	}
	function mouseRelease(evt) {
		press_anchor=[-1,-1];
		if (!is_dragging) {
			var ind=panel_index_at(evt.pos);
			set_current_panel_index(ind);
			O.emit('panel_clicked',{ind:ind,modifiers:evt.modifiers});
		}
		is_dragging=false;
	}
	function mouseMove(evt) {
		if (press_anchor[0]>=0) {
			var dx=evt.pos[0]-press_anchor[0];
			var dy=evt.pos[1]-press_anchor[1];

			if ((Math.abs(dx)>=4)||(Math.abs(dy)>=4)) {
				is_dragging=true;
			}
			if (is_dragging) {
				if (columnCount()>1) {
					m_viewport_geom[0]=press_anchor_viewport_geom[0]+dx/O.size()[0];
				}
				if (rowCount()>1) {
					m_viewport_geom[1]=press_anchor_viewport_geom[1]+dy/O.size()[1];
				}
				update_layout();
			}
		}
	}
	function mouseEnter(evt) {

	}
	function mouseLeave(evt) {
		press_anchor=[-1,-1];
	}
	function wheel(evt) {
		if (evt.delta>0) {
			zoom(1/1.2);
		}
		else if (evt.delta<0) {
			zoom(1.2);
		}
	}

}