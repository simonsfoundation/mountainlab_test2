function main(params) {
	console.log('testing');

	copy('script1.js','copy.script1.js');

	console.log('testing2');
}

function copy(infile,outfile) {
	var processor_name='copy';
	var params={input:infile,output:outfile};
	run_process(processor_name,params);
}

function run_process(processor_name,params) {
	if (!MP.runProcess(processor_name,JSON.stringify(params))) {
		console.log(processor_name+' '+JSON.stringify(params));
		throw 'Error running process: '+processor_name;
	}
}

var console={
	log:MP.log
};
