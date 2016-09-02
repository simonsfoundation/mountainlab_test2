var exports = module.exports = {};
var common=exports;

var fs=require('fs');
var child_process=require('child_process');

exports.read_algs_from_text_file=function(file_path) {
	var algs=[];
	{
		var txt=common.read_text_file(file_path);
		var lines=txt.split('\n');
		for (var i in lines) {
			if (lines[i].trim().slice(0,1)!='#') {
				var vals=lines[i].trim().split(' ');
				if (vals.length>=2) {
					algs.push({
						name:vals[0],
						script:vals[1],
						arguments:vals.slice(2).join(' ')
					});
				}
				else {
					if (lines[i].trim()) {
						throw 'problem in alglist file: '+lines[i].trim();
					}
				}
			}
		}
	}
	return algs;
}

exports.read_datasets_from_text_file=function(file_path) {
	var datasets=[];
	{
		var txt=common.read_text_file(file_path);
		var lines=txt.split('\n');
		for (var i in lines) {
			if (lines[i].trim().slice(0,1)!='#') {
				var vals=lines[i].trim().split(' ');
				if (vals.length==2) {
					datasets.push({
						name:vals[0],
						folder:vals[1]
					});
				}
				else {
					if (lines[i].trim()) {
						throw 'problem in datasetlist file: '+lines[i].trim();
					}
				}
			}
		}
	}
	return datasets;
};

exports.find_alg=function(algs,algname) {
	for (var i in algs) {
		if (algs[i].name==algname) return algs[i];
	}
	return null;
};

exports.find_ds=function(datasets,dsname) {
	for (var i in datasets) {
		if (datasets[i].name==dsname) return datasets[i];
	}
	return null;
};

exports.CLParams=function(argv) {
	this.unnamedParameters=[];
	this.namedParameters={};

	var args=argv.slice(2);
	for (var i=0; i<args.length; i++) {
		var arg0=args[i];
		if (arg0.indexOf('--')==0) {
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
		else {
			this.unnamedParameters.push(arg0);
		}
	}
}

exports.contains_alg=function(algnames,alg) {
	if (algnames=='all') return true;
	algnames=algnames.split(',');
	for (var i in algnames) {
		if (algnames[i]==alg.name)
			return true;
	}
	return false;
}

exports.contains_ds=function(dsnames,ds) {
	if (dsnames=='all') return true;
	dsnames=dsnames.split(',');
	for (var i in dsnames) {
		if (dsnames[i]==ds.name)
			return true;
	}
	return false;
}


exports.mkdir_safe=function(path) {
	try {
		fs.mkdirSync(path);
	}
	catch (err) {

	}
}

exports.read_text_file=function(path) {
	return fs.readFileSync(path,'utf8');
}

exports.copy_file_sync=function(src,dst) {
	if (!fs.existsSync(src)) return;
	var data=fs.readFileSync(src);
	fs.writeFileSync(dst,data);
}

var s_num_system_calls_running=0;
exports.make_system_call=function(cmd,args,callback) {
	console.log ('Running '+cmd+' '+args.join(' '));
	s_num_system_calls_running++;
	var pp=child_process.spawn(cmd,args);
	pp.stdout.setEncoding('utf8');
	pp.stderr.setEncoding('utf8');
	var done=false;
	pp.on('close', function(code) {
  		done=true;
		if (callback) callback();
		s_num_system_calls_running--;
	});
	pp.on('error',function(err) {
		console.log ('Process error: '+cmd+' '+args.join(' '));
		console.log (err);
	});
	var all_stdout='';
	var all_stderr='';
	pp.stdout.on('data',function(data) {
		console.log ('----'+data);
		all_stdout+=data;
	});
	pp.stderr.on('data',function(data) {
		console.log ('===='+data);
		all_stderr+=data;
	});
};

exports.wait_for_system_calls_to_finish=function(callback) {
	setTimeout(check_it,100);
	function check_it() {
		if (s_num_system_calls_running==0) {
			callback();
		}
		else {
			setTimeout(check_it,100);
		}
	}
};

exports.transpose_matrix=function(X) {
	if (X.length==0) return X;
	var Y=[];
	for (var i in X[0]) {
		Y.push([]);
	}
	for (var j in X) {
		for (var i in X[j]) {
			Y[i].push(X[j][i]);
		}
	}
	return Y;
}

exports.read_csv_matrix=function(path) {
	var ret=[];
	var txt=common.read_text_file(path);
	var lines=txt.split('\n');
	for (var i in lines) {
		var vals=lines[i].split(',');
		if (vals.length>0) {
			var row=[];
			for (var k=0; k<vals.length; k++) {
				row.push(Number(vals[k]));
			}
			ret.push(row);
		}
	}
	return common.transpose_matrix(ret); //this is because of a bad decision I made
}

exports.read_csv_vector=function(path) {
	var X=common.read_csv_matrix(path);
	var Y=[];
	for (var i in X) {
		for (var j in X[i]) {
			Y.push(X[i][j]);
		}
	}
	return Y;
}

exports.print_csv_matrix=function(X) {
	var txt='';
	for (var r=0; r<X.length; r++) {
		console.log (X[r].join(','));
	}
}

exports.row_sum=function(X,row) {
	var ret=0;
	for (var i in X[row]) {
		ret=ret+X[row][i];
	}
	return ret;
}
exports.col_sum=function(X,col) {
	var ret=0;
	for (var i in X) {
		ret=ret+X[i][col];
	}
	return ret;
}

exports.topct=function(num) {
	if (isNaN(num)) return '';
	if (num>1) return '>100%';
	if (num<0.1) return Math.round(num*1000)/10+'%';
	else return Math.round(num*1000)/10+'%';
}

exports.clone=function(X) {
	return JSON.parse(JSON.stringify(X));
}