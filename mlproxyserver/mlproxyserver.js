console.log ('Running mlproxyserver...');

//// requires
var	url=require('url');
var http=require('http');
var fs=require('fs');
var querystring=require('querystring');

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
config.listen_port=CLP.namedParameters.listen_port||config.listen_port;

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
	reqinfo.headers=REQ.headers;

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
	else if (reqinfo.path=='/favicon.ico') {
		RESP.writeHeader(500, {'content-type': 'text/plain'});
		RESP.end('No response.');
		return;
	}
	else { //GET or POST
		// Display a log message
		console.log (REQ.method+': '+REQ.url);

		//Check to see if we should proceed, and who we should pipe the request to
		preprocess_request(reqinfo,function(reqinfo2,result) {
			if (!result.success) {
				console.log ('Error in request: '+result.error);
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
			options.path=reqinfo2.path+'?'+querystring.stringify(reqinfo2.query);
			options.method=reqinfo2.method;
			options.headers=reqinfo2.headers;
			if (options.port==config.listen_port) {
				var errstr='Error: Cannot forward to own port: '+options.port;
				console.log (errstr);
				RESP.writeHeader(500, {'content-type': 'text/plain'});
        		RESP.end(errstr);
        		return;
			}
			console.log ('--------> Forwarding request to port: '+options.port);
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
}).listen(config.listen_port);
SERVER.timeout=1000*60*60*24; //give it 24 hours!
console.log ('Listening on port '+config.listen_port);

var HANDLERS={
	'/prv':new PrvHandler()
};
function preprocess_request(reqinfo,callback) {
	var path0=reqinfo.path;
	var ind=path0.slice(1).indexOf('/');
	if (ind>=0) path0=path0.slice(0,ind+1);
	if (path0 in HANDLERS) {
		HANDLERS[path0].preprocess_request(reqinfo,callback);
	}
	else {
		callback(null,{success:false,error:'No handler for path: '+reqinfo.path});
	}
}

function PrvHandler() {
	var port=config.prvfileserver_port||null;
	this.preprocess_request=function(reqinfo,callback) {
		if (!port) {
			callback(null,{success:false,error:'No port configured for prvfileserver'});
			return;
		}
		var action=reqinfo.query.a||'download';
		if (action in preprocess_handlers) {
			preprocess_handlers[action](reqinfo,callback);
		}
		else {
			callback(null,{success:false,error:'Action not supported: '+action});		
		}
	};
	var preprocess_handlers={
		download:preprocess_download,
		stat:preprocess_stat,
		locate:preprocess_locate,
		"concat-upload":preprocess_concat_upload,
		"list-subservers":preprocess_list_subservers,
		upload:preprocess_upload
	};
	function preprocess_download(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback(null,{success:false,error:'Method must be GET for this action'});
			return;
		}
		callback(reqinfo,{success:true,destination_port:port});
	}
	function preprocess_stat(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback(null,{success:false,error:'Method must be GET for this action'});
			return;
		}
		callback(reqinfo,{success:true,destination_port:port});
	}
	function preprocess_locate(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback(null,{success:false,error:'Method must be GET for this action'});
			return;
		}
		if ((reqinfo.path!='/prv')&&(reqinfo.path!='/prv/')) {
			callback(null,{success:false,error:'Invalid path for this action'});
			return;	
		}
		callback(reqinfo,{success:true,destination_port:port});
	}
	function preprocess_concat_upload(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback(null,{success:false,error:'Method must be GET for this action'});
			return;
		}
		if ((reqinfo.path!='/prv')&&(reqinfo.path!='/prv/')) {
			callback(null,{success:false,error:'Invalid path for this action'});
			return;	
		}
		callback(reqinfo,{success:true,destination_port:port});	
	}
	function preprocess_list_subservers(reqinfo,callback) {
		if (reqinfo.method!='GET') {
			callback(null,{success:false,error:'Method must be GET for this action'});
			return;
		}
		if ((reqinfo.path!='/prv')&&(reqinfo.path!='/prv/')) {
			callback(null,{success:false,error:'Invalid path for this action'});
			return;	
		}
		callback(reqinfo,{success:true,destination_port:port});	
	}
	function preprocess_upload(reqinfo,callback) {
		if (reqinfo.method!='POST') {
			callback(null,{success:false,error:'Method must be POST for this action'});
			return;
		}
		if ((reqinfo.path!='/prv')&&(reqinfo.path!='/prv/')) {
			callback(null,{success:false,error:'Invalid path for this action'});
			return;	
		}
		callback(reqinfo,{success:true,destination_port:port});
	}

}

function CLParams(argv) {
	this.unnamedParameters=[];
	this.namedParameters={};

	var args=argv.slice(2);
	for (var i=0; i<args.length; i++) {
		var arg0=args[i];
		if (arg0.indexOf('--')===0) {
			arg0=arg0.slice(2);
			var ind=arg0.indexOf('=');
			if (ind>=0) {
				this.namedParameters[arg0.slice(0,ind)]=arg0.slice(ind+1);
			}
			else {
				this.namedParameters[arg0]=args[i+1]||'';
				i++;
			}
		}
		else if (arg0.indexOf('-')===0) {
			arg0=arg0.slice(1);
			this.namedParameters[arg0]='';
		}
		else {
			this.unnamedParameters.push(arg0);
		}
	}
}
