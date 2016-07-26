// Instructions:
// npm install connect serve-static
// nodejs start_server.js [port_number]
var connect = require('connect');
var args=process.argv.slice(2);
var listen_port=args[0]||8040;
var serveStatic = require('serve-static');
connect().use(serveStatic(__dirname)).listen(listen_port, function(){
    console.log('Server running on port '+listen_port+'...');
});