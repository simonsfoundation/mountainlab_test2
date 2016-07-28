console.log('Running filebasket...');

//// requires
var	url=require('url');
var http=require('http');
var fs=require('fs');

var args=process.argv.slice(2);
var listen_port=args[0]||8041;

var listen_port=listen_port;
var storage_path='/tmp/filebasket';

mkdir_if_needed(storage_path);

http.createServer(function (REQ, RESP) {
	var url_parts = url.parse(REQ.url,true);	
	var path=url_parts.pathname;
	var query=url_parts.query;
	var method=query.a||'';
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
		if (method=="download") {
			var file_id=query.file_id||'';
			if (valid_file_id(file_id)) {
				var fname=storage_path+'/'+file_id;
				serve_file(fname,RESP);
			}
			else {
				send_text_response('invalid file id: '+file_id);
			}
		}
		else if (method=="") {
			try {
				var txt=fs.readFileSync('./'+path);
				if (path.endsWith('.js'))
					send_javascript_response(txt);
				else if (path.endsWith('.html'))
					send_html_response(txt);
			}
			catch(err) {
				send_text_response('file probably does not exist.');
			}
		}
		else {
			send_text_response('invalid method: '+method);
		}
	}
	else if(REQ.method=='POST') {
		console.log('POST: '+REQ.url);
		if (method=='upload') {
			var file_id=query.file_id||'';
			if (valid_file_id(file_id)) {
				var fname=storage_path+'/'+file_id;
				var ok=true;
				if (fs.existsSync(fname)) {
					ok=false;
					send_json_response({success:false,error:'file already exists with id: '+file_id});
					return;
				}
				var write_stream;
				write_stream=fs.createWriteStream(fname);
				write_stream.on('error',function(err) {
					console.log ('ERROR: '+JSON.stringify(err));
					ok=false;
					send_json_response({success:false,error:JSON.stringify(err)});
					return;
				});
				var num_bytes_received=0;
				REQ.on('data',function(chunk) {
					if (!ok) return;
					num_bytes_received+=chunk.length;
					write_stream.write(chunk,'binary');
				});
				REQ.on('end',function() {
					if (!ok) return;
					write_stream.end();
					send_json_response({success:true,message:'received '+num_bytes_received+' bytes'});
				});
			}
			else {
				send_json_response({success:false,error:'invalid file id: '+file_id});	
			}
		}
		else {
			send_json_response({success:false,error:'invalid method: '+method});	
		}
	}
	
	function send_json_response(obj) {
		RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"application/json"});
		RESP.end(JSON.stringify(obj));
	}
	function send_text_response(text) {
		RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"text/plain"});
		RESP.end(text);
	}
	function send_html_response(text) {
		RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"text/html"});
		RESP.end(text);
	}
	function send_javascript_response(text) {
		RESP.writeHead(200, {"Access-Control-Allow-Origin":"*", "Content-Type":"text/javascript"});
		RESP.end(text);
	}
}).listen(listen_port);
console.log ('Listening on port '+listen_port);

function valid_file_id(file_id) {
    return /^[a-z0-9\.\_]+$/i.test(file_id);
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
				response.writeHead(500, {"Content-Type": "application/octet-stream"});
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

if (!String.prototype.endsWith) {
  String.prototype.endsWith = function(searchString, position) {
      var subjectString = this.toString();
      if (typeof position !== 'number' || !isFinite(position) || Math.floor(position) !== position || position > subjectString.length) {
        position = subjectString.length;
      }
      position -= searchString.length;
      var lastIndex = subjectString.indexOf(searchString, position);
      return lastIndex !== -1 && lastIndex === position;
  };
}
