function jsqmain(query) {
	var context={};

    if (query.test=='1') {
        var mlproxy_url='http://datalaboratory.org:8020';
        var base_url='http://datalaboratory.org:8020/mdaserver/franklab/results/20160426_r1_nt16/ms_20160605';
        var timeseries_url=base_url+'/pre2.mda';
        var firings_url=base_url+'/firings_new.mda';

        //var timeseries_url='http://datalaboratory.org:8020/mdaserver/franklab/results/20160426_r1_nt16/ms_20160605/pre2.mda';
        //var firings_url='http://datalaboratory.org:8020/mdaserver/franklab/results/20160426_r1_nt16/ms_20160605/firings_new.mda';
        //var mlproxy_url='http://datalaboratory.org:8020';

        var mvcontext=new MVContext();
        mvcontext.setTimeseries(new RemoteReadMda(timeseries_url));
        mvcontext.setFirings(new RemoteReadMda(firings_url));
        mvcontext.setMLProxyUrl(mlproxy_url);
        mvcontext.setOption('clip_size',150);
        mvcontext.setOption('cc_max_dt_msec',100);
        mvcontext.setSampleRate(30000);

        var WW=new MVMainWindow(0,mvcontext);
        WW.showFullBrowser();

        var VV=new MVAmpHistView(0,mvcontext);
        WW.addView('north','Amplitudes',VV);

        var VV=new MVTemplatesView(0,mvcontext);
        WW.addView('north','Templates',VV);

        var VV=new MVCrossCorrelogramsView(0,mvcontext,'All_Auto_Correlograms');
        WW.addView('south','Auto-Correlograms',VV);

        var GCW=new GeneralControlWidget(0,mvcontext,WW);
        WW.addControlWidget(GCW);
    }
    else if (query.test=='2') {
        var mvcontext=new MVContext();
        var MW=new MVMainWindow(0,mvcontext);
        MW.showFullBrowser();
        MW.setControlPanelVisible(false);

        var url=query.smvfile;
        if (!url) {
            alert('Missing url parameter: smvfile.');
            return;
        }

        $.getJSON(url,function(data) {
            console.log(data);
            mvcontext.setStaticMode(true);
            mvcontext.setFromMVFileObject(data.mvcontext);
            var static_views=data['static-views'];
            for (var i in static_views) {
                var SV=static_views[i];
                var VV=create_static_view(mvcontext,SV.data);    
                MW.addView(SV.container||get_container_from_index(i),SV.data['view-type'],VV);
            }            
        });
    }
    else if (query.test=='3') {
        var mvcontext=new MVContext();
        var MW=new MVMainWindow(0,mvcontext);
        MW.showFullBrowser();
        MW.setControlPanelVisible(false);

        var filebasket_url=query.filebasket;
        if (!filebasket_url) {
            alert('Missing url parameter: filebasket.');
            return;
        }
        var file_id=query.file_id;
        if (!file_id) {
            alert('Missing url parameter: file_id.');
            return;
        }

        var url=filebasket_url+'/?a=download&file_id='+file_id;
        $.getJSON(url,function(data) {
            console.log(data);
            mvcontext.setStaticMode(true);
            mvcontext.setFromMVFileObject(data.mvcontext);
            var static_views=data['static-views'];
            for (var i in static_views) {
                var SV=static_views[i];
                var VV=create_static_view(mvcontext,SV.data);    
                MW.addView(SV.container||get_container_from_index(i),SV.data['view-type'],VV);
            }            
        });
    }
    function get_container_from_index(i) {
        if (i%2==0) return 'north';
        else return 'south';
    }
    function create_static_view(mvcontext,obj) {
        var X;
        if (obj['view-type']=="MVCrossCorrelogramsWidget") {
            X=new MVCrossCorrelogramsView(0,mvcontext,obj.options.mode);
        }
        else if (obj['view-type']=="MVClusterDetailWidget") {
            X=new MVTemplatesView(0,mvcontext);
        }
        else {
            alert('Unknown view-type: '+obj['view-type']);
            return 0;
        }    
        X.loadStaticView(obj);
        return X;
    }

    //var firings_url='http://localhost:8020/mdaserver/franklab/2016_04_08/sort_dl12_20151208_NNF_r1_tet16_17/output_tet16/firings.mda';

    /*
    var B=new RemoteReadMda();
    
    B.setPath(firings_url,function(res) {
    	B.readChunk(0,20,function(res) {
    		var chunk=res.chunk;
    	});
    });
    */


    /*
    {
        var X=new MountainProcessRunner();
        X.setProcessorName("mv_compute_templates");

        var params={};
        params["timeseries"] = timeseries_url;
        params["firings"] = firings_url;
        params["clip_size"] = 100;
        X.setInputParameters(params);
        X.setMLProxyUrl(mlproxy_url);

        var templates_fname = X.makeOutputFileUrl("templates");
        var stdevs_fname = X.makeOutputFileUrl("stdevs");

        X.runProcess(function(res) {
            var templates=new RemoteReadMda();
            templates.setPath(templates_fname);
            templates.toMda(function(res) {
                var templates0=res.mda;
                WW.setTemplates(templates0);
            });

            //templates_out.setPath(templates_fname);
            //stdevs_out.setPath(stdevs_fname);

            //templates_out.setRemoteDataType("float32_q8");
            //stdevs_out.setRemoteDataType("float32_q8");
        });
    }
    */




}