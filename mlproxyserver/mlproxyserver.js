console.log ('Running mlproxyserver...');

//// requires
var	url=require('url');
var http=require('http');
var fs=require('fs');

// npm install extend
var extend=require('extend');

CLP=new CLParams(process.argv);

// Read the mlproxyserver.default.json for configuration
// Then extend it by the prvfileserver.user.json, if it exists
var config=JSON.parse(fs.readFileSync(__dirname+'/mlproxyserver.default.json','utf8'));
try {
	var config_user=JSON.parse(fs.readFileSync(__dirname+'/mlproxyserver.user.json','utf8'));
	config=extend(true,config,config_user);
}
catch(err) {
}

// The listen port either comes from the configuration or the command-line options
config.listen_port=CLP.namedParameters['listen_port']||config.listen_port;

// Exit this program when the user presses ctrl+C
process.on('SIGINT', function() {
    process.exit();
});

// Create the web server!
var SERVER=http.createServer(function (REQ, RESP) {

	// Pause the request until we can determine that it should proceed
	REQ.pause();

	//parse the url of the request
	var reqinfo={};
	var url_parts = url.parse(REQ.url,true);
	reqinfo.path=url_parts.pathname;
	reqinfo.query=url_parts.query;
	reqinfo.method=REQ.method;

	if (reqinfo.method == 'OPTIONS') {
		var headers = {};
		
		//allow cross-domain requests
		/// TODO refine this
		
		headers["Access-Control-Allow-Origin"] = "*";
		headers["Access-Control-Allow-Methods"] = "POST, GET, PUT, DELETE, OPTIONS";
		headers["Access-Control-Allow-Credentials"] = false;
		headers["Access-Control-Max-Age"] = '86400'; // 24 hours
		headers["Access-Control-Allow-Headers"] = "X-Requested-With, X-HTTP-Method-Override, Content-Type, Accept";
		RESP.writeHead(200, headers);
		RESP.end();
	}
	else { //GET or POST
		//Check to see if we should proceed, and who we should pipe the request to
		handle_request(reqinfo,function(result) {
			if (!result.success) {
				//no, we should not proceed, send back an error
				RESP.writeHeader(500, {
          			'content-type': 'text/plain'
        		});
        		RESP.end(result.error);
        		//TODO: do we need to close the request or will that be done automatically?
        		return;
			}

			//prepare the request to be sent on to the subserver
			var options={};
			options.hostname='localhost';
			options.port=result.destination_port;
			options.path=reqinfo.path;
			options.query=reqinfo.query;
			options.method=REQ.method;
			options.headers=REQ.headers;
			var connector=http.request(options,function(subserver_response) {
				subserver_response.pause();
				subserver_response.headers['access-control-allow-origin']='*';
				RESP.writeHeader(subserver_response.statusCode,subserver_response.headers);
				//pipe the subserver response -> proxy response
				subserver_response.pipe(RESP,{end:true});
				subserver_response.resume();
			});
			//pipe the proxy request -> subserver request
			REQ.pipe(connector,{end:true});
			//The request may now resume
			REQ.resume();
		});
	}
});

var HANDLERS={
	'/prv':new PrvHandler()
};
function handle_request(reqinfo,callback) {
	var path0=reqinfo.path;
	var ind=path0.slice(1).indexOf('/');
	if (ind>=0) path0=path0.slice(0,ind+1);
	if (reqinfo.path in HANDLERS) {
		HANDLERS[reqinfo.path].handle(reqinfo,callback);
	}
	else {
		callback({success:false,error:'No handler for path: '+reqinfo.path});
	}
}

function PrvHandler() {
	var port=config.prvfileserver_port||null;
	this.handle=function(reqinfo,callback) {
		if (!port) {
			callback({success:false,error:'No port configured for prvfileserver'});
			return;
		}
		var action=reqinfo.query.a||'download';
		if (action in action_handlers) {
			action_handlers[action](reqinfo,callback);
		}
		else {
			callback({success:false,error:'Action not supported: '+action});		
		}
	};
	var action_handlers={
		download:download_handler,
		stat:stat_handler,
		locate:locate_handler,
		"concat-upload":concat_upload_handler,
		"list-subservers":list_subservers_handler,
		upload:upload_handler
	};
	function download_handler(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback({success:false,error:'Method must be GET for this action'});
			return;
		}
		callback({success:true,destination_port:port});
	}
	function stat_handler(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback({success:false,error:'Method must be GET for this action'});
			return;
		}
		callback({success:true,destination_port:port});
	}
	function locate_handler(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback({success:false,error:'Method must be GET for this action'});
			return;
		}
		callback({success:true,destination_port:port});
	}
	function concat_upload_handler(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback({success:false,error:'Method must be GET for this action'});
			return;
		}
		callback({success:true,destination_port:port});	
	}
	function list_subservers_handler(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback({success:false,error:'Method must be GET for this action'});
			return;
		}
		callback({success:true,destination_port:port});	
	}
	function upload_handler(reqinfo,callback) {
		if (reqinfo.method!='POST') {
			callback({success:false,error:'Method must be POST for this action'});
			return;
		}
		callback({success:true,destination_port:port});
	}

}


