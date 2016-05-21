var http = require('http');
var httpProxy = require('http-proxy');

var mlhub_listen_port=8080; 
var forwarded_port=8004;

var proxy = httpProxy.createProxyServer({});

var server = http.createServer(function(req, res) {
  proxy.web(req, res, { target: 'http://localhost:'+forwarded_port});
});

console.log("listening on port "+mlhub_listen_port);
server.listen(mlhub_listen_port);