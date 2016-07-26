var http = require('http');
var httpProxy = require('http-proxy'); //this is a module included in node_modules

//read and print the configuration
var cfg=read_json('../labcomputer.json');
console.log(JSON.stringify(cfg));

//get the needed config options -
var mlproxy_port=cfg.mlproxy_listen_port;
var mdaserver_port=cfg.mdaserver_listen_port;
var mscmdserver_port=cfg.mscmdserver_listen_port;
var mbserver_port=cfg.mbserver_listen_port;
var moserver_port=cfg.moserver_listen_port;
var mpserver_port=cfg.mpserver_listen_port;

var proxy = httpProxy.createProxyServer({});
proxy.on('error',function(err,req,res) {
	 res.writeHead(500, {'Content-Type': 'text/plain'});
     res.end('Proxy Error: '+JSON.stringify(err));
});

var server = http.createServer(function(REQ, RESP) {
  var url_parts = require('url').parse(REQ.url,true);	
  var path=url_parts.pathname;

  console.log('REQ: '+REQ.url);
  var handlers=[
    {name:'mdaserver',port:mdaserver_port},
    {name:'mscmdserver',port:mscmdserver_port},
    {name:'mbserver',port:mbserver_port},
    {name:'mpserver',port:mpserver_port}
  ];

  for (var i in handlers) {
    var H=handlers[i];
    if (path.indexOf('/'+H.name)==0) {
      url_parts.pathname=path.slice(('/'+H.name).length);
      REQ.url=require('url').format(url_parts);
      console.log('Forwarding to '+H.name+' on port '+H.port+': '+REQ.url);
      proxy.web(REQ, RESP, { target: 'http://localhost:'+H.port});
      return;
    }  
  }

  RESP.end();
});

console.log("listening on port "+mlproxy_port);
server.listen(mlproxy_port);

function read_json(fname) {
	var fs = require('fs');
	var obj = JSON.parse(fs.readFileSync(fname, 'utf8'));
	return obj;
}


////// KEEP UP THE PORT FORWARDING
var time_interval=10000;

setTimeout(forward_ports,0);

function forward_ports() {
  console.log('forward_ports)');
  if (cfg.mlhub_host) {
    var remote_host=cfg.mlhub_host;
    var remote_port=cfg.mlhub_forwarded_port;
    var user_name=cfg.mlhub_user;
    var local_port=cfg.mlproxy_listen_port;
    
    var exe='/bin/bash';
    var args=['setup_port_forward.sh',remote_host,remote_port,local_port,user_name];
    console.log(exe+' '+args.join(' '));
    var process=require('child_process').spawn(exe,args,{stdio:[0,1,2]});
    process.on('close',function(code) {
      console.log('Exited with code: '+code);
      setTimeout(forward_ports,time_interval);
    }); 
  }
}

