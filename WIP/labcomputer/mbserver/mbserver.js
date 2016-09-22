console.log('Running mbserver...');

/// TODO RENAME mbserver to moserver

//// requires
var	url=require('url');
var http=require('http');
var fs=require('fs');

//// configuration
var config_fname='../labcomputer.json';
var config_path=require('path').dirname(config_fname);
var config = JSON.parse(fs.readFileSync(config_fname, 'utf8'));

config.mbserver_base_path=require('path').resolve(config_path,config.mbserver_base_path);

console.log(JSON.stringify(config));

var mbserver_listen_port=config.mbserver_listen_port;
var mbserver_base_path=config.mbserver_base_path;

http.createServer(function (REQ, RESP) {
	var url_parts = url.parse(REQ.url,true);	
	if (REQ.method == 'OPTIONS') {
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
	else if (REQ.method=='GET') {
		var path=url_parts.pathname;
		console.log('GET: '+REQ.url);
		var query=url_parts.query;
		var method=query.a||'';
		if (method=="readJson") {
			/// TODO this needs to be made secure -- only allow extension = .mo?
			var obj=safe_read_json_file(mbserver_base_path+'/'+path); 
			send_json_response(obj);
		}
		else {
			send_json_response({success:false,error:"Unexpected method: "+method});
		}
	}
	else if(REQ.method=='POST') {
		send_text_response("POST not supported!");
	}
	
	function send_json_response(obj) {
		RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"application/json"});
		RESP.end(JSON.stringify(obj));
	}
	function send_text_response(text) {
		RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"text/plain"});
		RESP.end(text);
	}
}).listen(mbserver_listen_port);
console.log ('Listening on port '+mbserver_listen_port);


function safe_read_json_file(fname) {
	try {
		var json=fs.readFileSync(fname, 'utf8');
		return JSON.parse(json);
	}
	catch(err) {
		console.log('ERROR parsing or reading json file: '+fname);
		console.log(JSON.stringify(err));
		return {};
	}
}
