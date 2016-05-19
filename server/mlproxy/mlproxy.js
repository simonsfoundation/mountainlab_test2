var http = require('http');
var httpProxy = require('http-proxy');

var cfg=read_json('../labcomputer.json');
console.log(JSON.stringify(cfg));

var local_proxy_port=cfg.labcomputer.local_proxy_port;
var mdaserver_port=cfg.mdaserver.listen_port;
var mscmdserver_port=cfg.mscmdserver.listen_port;
var mountainbrowserserver_port=cfg.mountainbrowserserver.listen_port;
var mpserver_port=cfg.mpserver.listen_port;

var proxy = httpProxy.createProxyServer({});
proxy.on('error',function(err,req,res) {
	 res.writeHead(500, {'Content-Type': 'text/plain'});
     res.end('Proxy Error: '+JSON.stringify(err));
});

var server = http.createServer(function(REQ, RESP) {
  var url_parts = require('url').parse(REQ.url,true);	
  var path=url_parts.pathname;
  if (path.indexOf('/mdaserver')==0) {
  	proxy.web(REQ, RESP, { target: 'http://localhost:'+mdaserver_port});
  }
  else if (path.indexOf('/mscmdserver')==0) {
  	proxy.web(REQ, RESP, { target: 'http://localhost:'+mscmdserver_port});
  }
  else if (path.indexOf('/mountainbrowserserver')==0) {
  	proxy.web(REQ, RESP, { target: 'http://localhost:'+mountainbrowserserver_port});
  }
  else if (path.indexOf('/mpserver')==0) {
  	proxy.web(REQ, RESP, { target: 'http://localhost:'+mpserver_port});
  }
  
  else {
  	RESP.end();
  }
});

console.log("listening on port "+local_proxy_port);
server.listen(local_proxy_port);

function read_json(fname) {
	var fs = require('fs');
	var obj = JSON.parse(fs.readFileSync(fname, 'utf8'));
	return obj;
}