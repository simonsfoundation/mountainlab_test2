console.log('Running open_server...');

/*
WARNING (to myself)!!!!
Resist the temptation to allow this server to do more than simply opening/uploading files
by creating an iframe and directing to other url's
This can be run on a bare-bones machine with nothing valuable on it (no data).
*/

//// requires
var	url=require('url');
var http=require('http');
var fs=require('fs');

var args=process.argv.slice(2);
var listen_port=args[0]||80;

http.createServer(function (REQ, RESP) {
	var url_parts = url.parse(REQ.url,true);	
	var path=url_parts.pathname;
	var query=url_parts.query;
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
		console.log('path: '+path);
		console.log(query);
		var url0='';
		if (path.startsWith('/open/')) {
			path=path.slice('/open/'.length);	
			url0=get_open_url(path,query);
		}
		else if ((path.startsWith('/upload/'))||(path=='/upload')) {
			path=path.slice('/upload/'.length);
			url0=get_upload_url(path,query);
		}
		if (!url0) {
			send_text_response('Unable to open: '+path);	
			return;
		}
		console.log(url0);
		var txt=fs.readFileSync(__dirname+'/open.html');
		txt=txt.toString().split('$iframe_url$').join(url0);
		send_html_response(txt);
	}
	else if(REQ.method=='POST') {
		console.log('POST: '+REQ.url);
		send_text_response("POST not supported");
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

function get_open_url(path,query) {
	if (path.endsWith('.smv')) {
		var url='http://datalaboratory.org:8040';
		url+='/mountainviewweb/mountainviewweb.html?';
		url+='file_id='+path;
	}
	else {
		url='';
	}
	return url;
}

function get_upload_url(path,query) {
	return 'http://datalaboratory.org:8040/mountainviewweb/mvupload.html';
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

/*! https://mths.be/startswith v0.2.0 by @mathias */
if (!String.prototype.startsWith) {
	(function() {
		'use strict'; // needed to support `apply`/`call` with `undefined`/`null`
		var defineProperty = (function() {
			// IE 8 only supports `Object.defineProperty` on DOM elements
			try {
				var object = {};
				var $defineProperty = Object.defineProperty;
				var result = $defineProperty(object, object, object) && $defineProperty;
			} catch(error) {}
			return result;
		}());
		var toString = {}.toString;
		var startsWith = function(search) {
			if (this == null) {
				throw TypeError();
			}
			var string = String(this);
			if (search && toString.call(search) == '[object RegExp]') {
				throw TypeError();
			}
			var stringLength = string.length;
			var searchString = String(search);
			var searchLength = searchString.length;
			var position = arguments.length > 1 ? arguments[1] : undefined;
			// `ToInteger`
			var pos = position ? Number(position) : 0;
			if (pos != pos) { // better `isNaN`
				pos = 0;
			}
			var start = Math.min(Math.max(pos, 0), stringLength);
			// Avoid the `indexOf` call if no match is possible
			if (searchLength + start > stringLength) {
				return false;
			}
			var index = -1;
			while (++index < searchLength) {
				if (string.charCodeAt(start + index) != searchString.charCodeAt(index)) {
					return false;
				}
			}
			return true;
		};
		if (defineProperty) {
			defineProperty(String.prototype, 'startsWith', {
				'value': startsWith,
				'configurable': true,
				'writable': true
			});
		} else {
			String.prototype.startsWith = startsWith;
		}
	}());
}
