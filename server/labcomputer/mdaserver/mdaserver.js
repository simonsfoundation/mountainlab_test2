console.log('Running mdaserver...');

//// requires
var	url=require('url');
var http=require('http');
var fs=require('fs');

//// configuration
var config_fname='../labcomputer.json';
var config_path=require('path').dirname(config_fname);
var config = JSON.parse(fs.readFileSync(config_fname, 'utf8'));

console.log(JSON.stringify(config));

var mdaserver_listen_port=config.mdaserver_listen_port;
var mdachunk_data_path=require('path').resolve(config_path,config.mdachunk_data_path);
var mdachunk_exe=require('path').resolve(config_path,config.mdachunk_exe);
var mdaserver_base_path=require('path').resolve(config_path,config.mdaserver_base_path);

mkdir_if_needed(mdachunk_data_path);
//the following two folders are created because this is where the mountainview client will store temporary stuff
//TODO fix this -- let server choose where to put it?
mkdir_if_needed(mdaserver_base_path+'/tmp_short_term');
mkdir_if_needed(mdaserver_base_path+'/tmp_long_term');

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
		console.log('GET: '+REQ.url);
		var path=url_parts.pathname;
		var query=url_parts.query;
		var method=query.a||'';
		if (method=="info") {
			/// TODO make this secure
			var mda_fname=mdaserver_base_path+'/'+path;
			run_process_and_read_stdout(mdachunk_exe,["info",mda_fname],function(txt) {
				txt=txt.trim();
				if (query.output=="text") {
					send_text_response(txt);
					return;
				}
				var lines=txt.split('\n');
				if (lines.length!=3) {
					send_json_response({success:false,error:'Incorrect output from mdachunk: '+txt});
					return;
				}
				var dimensions=lines[0].split(',');
				if (dimensions.length!=6) {
					send_json_response({success:false,error:'Incorrect output from mdachunk **: '+txt});
					return;	
				}
				var sha1=lines[1];
				var lastModified=lines[2];
				var obj={
					dimensions:dimensions,
					sha1:sha1,
					lastModified:lastModified
				};
				send_json_response(obj);	
			});
		}
		/// TODO -- readText should not be needed in this server!
		else if (method=="readText") {
			/// TODO make this secure
			var txt_fname=mdaserver_base_path+'/'+path;
			run_process_and_read_stdout("/usr/bin/cat",[txt_fname],function(txt) {
				send_text_response(txt);
			});
		}
		else if (method=="readChunk") {
			/// TODO make this secure
			var mda_fname=mdaserver_base_path+'/'+path;

			var datatype=query.datatype||'float32';
			var index=query.index||'0';
			var size=query.size||'';
			var args=['readChunk',mda_fname,"--index="+index,"--size="+size,"--datatype="+datatype,"--outpath="+mdachunk_data_path];
			run_process_and_read_stdout(mdachunk_exe,args,function(txt) {
				txt=txt.trim();
				if (!txt) {
					send_json_response({success:false,error:"Output of mdachunk is empty."});		
					return;
				}

				if (!require('fs').existsSync(mdachunk_data_path+"/"+txt)) {
					send_json_response({success:false,error:"Chunk file does not exist in mdachunk_data_path: "+txt});
					return;	
				}
				if (query.output=='text') {
					send_text_response(txt);
					return;
				}
				var obj={
					path:txt
				};
				send_json_response(obj);
			});
		}
		else {
			/// TODO important make this secure
			var fname=mdachunk_data_path+"/"+path;
			if (!require('fs').existsSync(fname)) {
				send_json_response({success:false,error:"File does not exist in mdachunk_data_path: "+path});		
				return;	
			}
			serve_file(fname,RESP);
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
}).listen(mdaserver_listen_port);
console.log ('Listening on port '+mdaserver_listen_port);

function run_process_and_read_stdout(exe,args,callback) {
	console.log('RUNNING:'+exe+' '+args.join(' '));
	var P=require('child_process').spawn(exe,args);
	var txt='';
	P.stdout.on('data',function(chunk) {
		txt+=chunk;
	});
	P.on('close',function(code) {
		callback(txt);
	});
}

function serve_file(filename,response) {
	response.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"application/json"});
	fs.exists(filename,function(exists) {
		if (!exists) {
			response.writeHead(404, {"Content-Type": "text/plain"});
			response.write("404 Not Found\n");
			response.end();
			return;
		}

		fs.readFile(filename, "binary", function(err, file) {
			if(err) {        
				response.writeHead(500, {"Content-Type": "text/plain"});
				response.write(err + "\n");
				response.end();
				return;
			}

			//response.writeHead(200);
			response.write(file, "binary");
			response.end();
		});
	});
}

function mkdir_if_needed(path) {
	var fs=require('fs');
	if (!fs.existsSync(path)){
    	fs.mkdirSync(path);
	}
}
