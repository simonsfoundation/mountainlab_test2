#!/usr/bin/env nodejs

/*
Use this to set up a day's worth of analysis at L. Frank's lab.

Suppose the day's data are located at /data/path
For example, the following would already exist:
20160426_kanye_04_r2.mda
20160426_kanye_05_s3.mda
...
and we'd want it to create the following:
20160426_kanye_04_r2.mountain
20160426_kanye_05_s3.mountain
...

Step 1: ./setup_mountainsort.node.js /data/path

Step 2: cd /data/path and do the sorting

For a single tetrode you would do this:
> kron-run ms 05_nt1 --outpath=sorting_results

Take a look at the auto-generated datasets.txt to see which datasets are available

To view the results
> kron-view results ms 05_nt1

To run ALL the sorting:
> kron-run ms all

*/

var fs=require('fs');

function usage() {
	console.log('Usage: ./setup_mountainsort.node.js [directory name]');
}

//command-line parameters
var CLP=new CLParams(process.argv);
var directory=CLP.unnamedParameters[0];
if (!directory) {
	usage();
	process.exit(-1);
}

var top_list=fs.readdirSync('.');
var mda_directories=[];
for (var i in top_list) {
	if (string_ends_with(top_list[i],'.mda')) {
		mda_directories.push(top_list[i]);
	}
}

var datasets='';
var pipelines='ms mountainsort_001.pipeline';

for (var dd in mda_directories) {
	var input_directory=mda_directories[dd];
	var output_directory=input_directory.slice(0,input_directory.length-4)+".mountain";
	var recnum=get_recording_num_from_directory_name(input_directory);

	mkdir_if_needed(output_directory);
	var list=fs.readdirSync(input_directory);
	var raw_fnames=[];
	for (var i in list) {
		if ((string_ends_with(list[i],'.mda'))&&(string_contains(list[i],'.nt'))) {
			raw_fnames.push(list[i]);
		}
	}
	
	for (var i in raw_fnames) {
		var dsname=raw_fnames[i].slice(0,raw_fnames[i].length-4);
		dsname=dsname.slice(dsname.lastIndexOf('.')+1);
		mkdir_if_needed(output_directory+'/'+dsname);
		var bb=output_directory+'/'+dsname;
		var params0={samplerate:30000};
		var exe='prv-create';
		var args=[input_directory+'/'+raw_fnames[i],bb+'/raw.mda.prv'];
		fs.writeFileSync(bb+'/params.json',JSON.stringify(params0));
		datasets+=recnum+'_'+dsname+' '+output_directory+'/'+dsname+'\n';
		run_process_and_read_stdout(exe,args,function(txt) {
			console.log(txt);
		});
	}
}

fs.writeFileSync(directory+'/datasets.txt',datasets);
fs.writeFileSync(directory+'/pipelines.txt',pipelines);

function get_recording_num_from_directory_name(str) {
	return str.split('.')[0].split('_')[2];
}

function CLParams(argv) {
	this.unnamedParameters=[];
	this.namedParameters={};

	var args=argv.slice(2);
	for (var i=0; i<args.length; i++) {
		var arg0=args[i];
		if (arg0.indexOf('--')===0) {
			arg0=arg0.slice(2);
			var ind=arg0.indexOf('=');
			if (ind>=0) {
				this.namedParameters[arg0.slice(0,ind)]=arg0.slice(ind+1);
			}
			else {
				this.namedParameters[arg0]=args[i+1]||'';
				i++;
			}
		}
		else if (arg0.indexOf('-')===0) {
			arg0=arg0.slice(1);
			this.namedParameters[arg0]='';
		}
		else {
			this.unnamedParameters.push(arg0);
		}
	}
}

function mkdir_if_needed(path) {
	var fs=require('fs');
	if (!fs.existsSync(path)){
    	fs.mkdirSync(path);
	}
}

function string_ends_with(str,str2) {
	if (str2.length>str.length) return false;
	return (str.slice(str.length-str2.length)==str2);
}

function string_contains(str,str2) {
	var ind=str.indexOf(str2);
	return (ind>=0);
}

function run_process_and_read_stdout(exe,args,callback) {
	console.log ('RUNNING:'+exe+' '+args.join(' '));
	var P=require('child_process').spawn(exe,args);
	var txt='';
	P.stdout.on('data',function(chunk) {
		txt+=chunk;
	});
	P.on('close',function(code) {
		callback(txt);
	});
}

