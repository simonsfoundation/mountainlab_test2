function jsqmain(query) {
	var context={};

    if (!query.filebasket) query.filebasket='http://datalaboratory.org:8041'

    {
        var mvcontext=new MVContext();

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

        var MW=new MVMainWindow(0,mvcontext);
        MW.showFullBrowser();
        MW.setControlPanelVisible(false);
        MW.setStatus('load-main','Loading data from '+url+'.... please wait...');

        $.getJSON(url,function(data) {
            MW.setStatus('load-main','Loaded data from '+url);
            console.log(data);
            mvcontext.setStaticMode(true);
            mvcontext.setFromMVFileObject(data.mvcontext);
            MW.setTitle(file_id);
            MW.setSMVObject(data);
            MW.setupActions(); //must be done after mvcontext has been set up
            var static_views=data['static-views'];
            MW.setStatus('load-static-views','Loading '+static_views.length+' static views');
            for (var i in static_views) {
                var SV=static_views[i];
                var VV=create_static_view(mvcontext,SV.data);    
                MW.addView(SV.container||get_container_from_index(i),SV.title||SV.data['view-type'],VV);
            }  
            MW.setStatus('load-static-views','Loaded '+static_views.length+' static views');
        });
    }
    function get_container_from_index(i) {
        if (i%2===0) return 'north';
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