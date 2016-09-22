function GeneralControlWidget(O,mvcontext,main_window) {
	O=O||this;
	MVAbstractControlWidget(O,mvcontext,main_window);
	O.div().addClass('GeneralControlWidget');
	O.setSize(0,200);

	O.updateControls=function() { //override
		O.setControlValue('clip_size',mvcontext.option('clip_size'));
		O.setControlValue('cc_max_dt_msec',mvcontext.option('cc_max_dt_msec'));
	};

	var table0=$('<table></table>');
	O.div().append(table0);
	{ //clip_size
		var X=O.createIntControl('clip_size');
		var tr=$('<tr><td>Clip size:</td><td id=tmp></td></tr>');
		tr.find('#tmp').append(X); table0.append(tr);
	}
	{ //cc_max_dt_msec
		var X=O.createIntControl('cc_max_dt_msec');
		var tr=$('<tr><td>CC max. dt (msec):</td><td id=tmp></td></tr>');
		tr.find('#tmp').append(X); table0.append(tr);
	}
	{ //Apply button
		var X=O.createButtonControl('apply','Apply',function() {
			mvcontext.setOption('clip_size',O.controlValue('clip_size'));
			mvcontext.setOption('cc_max_dt_msec',O.controlValue('cc_max_dt_msec'));
		});
		var tr=$('<tr><td></td><td id=tmp></td></tr>');
		tr.find('#tmp').append(X); table0.append(tr);
	}
	O.updateControlsOn(mvcontext,'optionsChanged');
}