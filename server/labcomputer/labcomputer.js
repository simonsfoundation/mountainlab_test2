var time_interval=10000;

setTimeout(forward_ports,0);

var cfg=read_json('../labcomputer.json');

function forward_ports() {
	console.log('forward_ports)');
	var remote_host=cfg.labcomputer.mlhub_host;
	var remote_port=cfg.labcomputer.mlhub_forwarded_port;
	var local_port=cfg.labcomputer.local_proxy_port;
	var user_name=cfg.labcomputer.mlhub_user;
	
	var exe='/bin/bash';
	var args=['setup_port_forward.sh',remote_host,remote_port,local_port,user_name];
	console.log(exe+' '+args.join(' '));
	var process=require('child_process').spawn(exe,args,{stdio:[0,1,2]});
	process.on('close',function(code) {
		console.log('Exited with code: '+code);
		setTimeout(forward_ports,time_interval);
	});	
}

function read_json(fname) {
	var fs = require('fs');
	var obj = JSON.parse(fs.readFileSync(fname, 'utf8'));
	return obj;
}