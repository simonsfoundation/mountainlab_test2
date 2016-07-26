var jsqmake=require('../jsqmake/jsqmake').jsqmake;

var opts={PROJECTPATH:__dirname, SOURCEPATH:['.'], SCRIPTS:[], STYLESHEETS:[]};
opts.TARGET = 'mountainviewweb.html';

opts.SCRIPTS.push('mountainviewweb.js');
opts.STYLESHEETS.push('mountainviewweb.css');

opts.SOURCEPATH.push('../jsqcore')
opts.SCRIPTS.push(
	'jquery.min.js','jsq.js','jsqobject.js','jsqwidget.js','jsqcanvaswidget.js'
);
opts.STYLESHEETS.push(
	'jsq.css'
);

opts.SOURCEPATH.push('../jsqwidgets')
opts.SCRIPTS.push(
	'jsqcanvaswidget.js','jsqtabwidget.js'
);

opts.SOURCEPATH.push('common/mda')
opts.SCRIPTS.push(
	'mda.js','remotereadmda.js'
);
opts.STYLESHEETS.push(
	'jsq.css'
);

opts.SOURCEPATH.push('core')
opts.SCRIPTS.push(
	'mountainprocessrunner.js','mvabstractview.js','mvcontext.js',
	'mvcontrolpanel.js','mvmainwindow.js','mvabstractcontrolwidget.js',
	'mvpanelwidget.js','tabber.js','tabberframe.js'
);

opts.SOURCEPATH.push('views')
opts.SCRIPTS.push(
	'mvtemplatesview.js','histogramview.js','mvhistogramgrid.js','mvamphistview.js','mvcrosscorrelogramsview.js'
);

opts.SOURCEPATH.push('controlwidgets')
opts.SCRIPTS.push(
	'generalcontrolwidget.js'
);

jsqmake(opts);
