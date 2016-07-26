function Mda() {
	var that=this;
	this.allocate=function(n1,n2,n3,n4,n5) {
		n1=n1||1; n2=n2||1; n3=n3||1;
		n4=n4||1; n5=n5||1;
		m_total_size=n1*n2*n3*n4*n5;
		m_dims[0]=n1; m_dims[1]=n2; m_dims[2]=n3; m_dims[3]=n4; m_dims[4]=n5;
		m_data=new Float32Array(m_total_size);
		for (var i=0; i<m_total_size; i++) m_data[i]=0;
	};
	this.N1=function() {return m_dims[0];};
	this.N2=function() {return m_dims[1];};
	this.N3=function() {return m_dims[2];};
	this.N4=function() {return m_dims[3];};
	this.N5=function() {return m_dims[4];};
	this.totalSize=function() {return m_total_size;};
	this.value=function(i1,i2,i3,i4,i5) {
		if (i2===undefined) {
			return m_data[i1];
		}
		else if (i3===undefined) {
			return this.value(i1+m_dims[0]*i2);
		}
		else if (i4===undefined) {
			return this.value(i1+m_dims[0]*i2+m_dims[0]*m_dims[1]*i3);
		}
		else if (i5===undefined) {
			return this.value(i1+m_dims[0]*i2+m_dims[0]*m_dims[1]*i3+m_dims[0]*m_dims[1]*m_dims[2]*i4);
		}
		else {
			return this.value(i1 +m_dims[0]*i2 +m_dims[0]*m_dims[1]*i3 +m_dims[0]*m_dims[1]*m_dims[2]*i4 +m_dims[0]*m_dims[1]*m_dims[2]*m_dims[3]*i5);
		}
	};
	this.setValue=function(val,i1,i2,i3,i4,i5) {
		if (i2===undefined) {
			m_data[i1]=val;
		}
		else if (i3===undefined) {
			this.setValue(val,i1+m_dims[0]*i2);
		}
		else if (i4===undefined) {
			this.setValue(val,i1+m_dims[0]*i2+m_dims[0]*m_dims[1]*i3);
		}
		else if (i5===undefined) {
			this.setValue(val,i1+m_dims[0]*i2+m_dims[0]*m_dims[1]*i3+m_dims[0]*m_dims[1]*m_dims[2]*i4);
		}
		else {
			this.setValue(val,i1 +m_dims[0]*i2 +m_dims[0]*m_dims[1]*i3 +m_dims[0]*m_dims[1]*m_dims[2]*i4 +m_dims[0]*m_dims[1]*m_dims[2]*m_dims[3]*i5);
		}
	};
	this.data=function() {return m_data;};
	this.dataCopy=function() {
		var ret=new Float32Array(m_total_size);
		for (var i=0; i<m_total_size.length; i++) {
			ret[i]=m_data[i];
		}
		return ret;
	};
	this.setData=function(data) {
		m_data=data;
	};
	this.clone=function() {
		var ret=new Mda();
		ret.allocate(this.N1(),this.N2(),this.N3(),this.N4(),this.N5());
		ret.setData(this.dataCopy());
		return ret;
	};
	this.reshape=function(n1,n2,n3,n4,n5) {
		n2=n2||1; n3=n3||1; n4=n4||1; n5=n5||1;
		var tot=n1*n2*n3*n4*n5;
		if (tot!=m_total_size) {
			console.error('Unable to reshape... incompatible size: '+n1+'x'+n2+'x'+n3+'x'+n4+'x'+n5+'    '+this.N1()+'x'+this.N2()+'x'+this.N3()+'x'+this.N4()+'x'+this.N5());
			return;
		}
		m_dims[0]=n1; m_dims[1]=n2; m_dims[2]=n3; m_dims[3]=n4; m_dims[4]=n5;		
	};
	this.getChunk=function(i,size) {
		var ret=new Mda();
		ret.allocate(size,1);
		ret.setData(m_data.subarray(i,i+size));
		return ret;
	};
	this.subArray=function(arg1,arg2,arg3,arg4,arg5,arg6) {
		if (arg3===undefined) {
			return that.getChunk(arg1,arg2);
		}
		else if (arg5===undefined) {
			if ((arg3!=that.N1())||(arg1!==0)) {
				console.error('This case not supported yet: subArray.');
				return null;
			}
			var iii=arg2*that.N1();
			var sss=arg4*that.N1();
			var ret=this.getChunk(iii,sss);
			ret.reshape(arg3,arg4);
			return ret;
		}
		else {
			if ((arg4!=that.N1())||(arg1!==0)) {
				console.error('This case not supported yet: subArray.');
				return null;
			}
			if ((arg5!=that.N2())||(arg2!==0)) {
				console.error('This case not supported yet: subArray.');
				return null;
			}
			var iii=arg3*that.N1()*that.N2();
			var sss=arg6*that.N1()*that.N2();
			var ret=this.getChunk(iii,sss);
			ret.reshape(arg4,arg5,arg6);
			return ret;
		}
	};
	this.load=function(url,callback) {
		if (!(url in s_mda_binary_loaders)) {
			s_mda_binary_loaders[url]=new MdaBinaryLoader(url);
		}

		s_mda_binary_loaders[url].load(function(ret) {
			that.allocate(ret.mda.N1(),ret.mda.N2(),ret.mda.N3());
			that.setData(ret.mda.data());
			callback({success:true});
			/*
			if (ret.data.length>0) {
				var dims=ret.dims;
				that.allocate(dims[0],dims[1]||1,dims[2]||1,dims[3]||1,dims[4]||1);
				m_data=ret.data;
				callback({success:true});
			}
			else {
				callback({success:false});
			}
			*/
		});
	};
	this.setFromArrayBuffer=function(buf) {
		var X=new Int32Array(buf.slice(0,64));
		var num_bytes_per_entry=X[1];
		var num_dims=X[2];
		m_dims=[];
		if ((num_dims<2)||(num_dims>5)) {
			console.error('Invalid number of dimensions: '+num_dims);
			return false;
		} 
		for (var i=0; i<num_dims; i++) {
			m_dims.push(X[3+i]);
		}
		var dtype=get_dtype_string(X[0]);
		var header_size=(num_dims+3)*4;
		if (dtype=='float32') {
			m_data=new Float32Array(buf.slice(header_size));
			return true;
		}
		else if (dtype=='float64') {
			m_data=new Float64Array(buf.slice(header_size));
			return true;	
		}
		else {
			console.error('Unsupported dtype: '+dtype);
			m_data=[];
			return false;
		}
	}
	this.minimum=function() {
		if (m_data.length===0) return 0;
		var ret=m_data[0];
		for (var i=0; i<m_data.length; i++) {
			if (m_data[i]<ret) ret=m_data[i];
		}
		return ret;
	};
	this.maximum=function() {
		if (m_data.length===0) return 0;
		var ret=m_data[0];
		for (var i=0; i<m_data.length; i++) {
			if (m_data[i]>ret) ret=m_data[i];
		}
		return ret;
	};
	function get_dtype_string(num) {
		if (num==-2) return 'byte';
		if (num==-3) return 'float32';
		if (num==-4) return 'int16';
		if (num==-5) return 'int32';
		if (num==-6) return 'uint16';
		if (num==-7) return 'float64';
		return '';
	}
	
	var m_data=new Float32Array(1);
	var m_dims=[1,1,1,1,1];
	var m_total_size=1;
}

var s_mda_binary_loaders={};
function MdaBinaryLoader(url) {
	this.load=function(callback) {
		if (m_done_loading) {
			callback(m_data.slice());
			return;
		}
		JSQ.connect(m_signaler,'loaded',0,function() {
			callback({mda:m_mda});	
		});
		if (!m_is_loading) {
			m_is_loading=true;
			start_loading();
		}
	}

	var m_signaler=new JSQObject();
	var m_is_loading=false;
	var m_done_loading=false;
	var m_mda=new Mda();

	function start_loading() {
		$.ajax({
			url: url,
			type: "GET",
			dataType: "binary",
			processData: false,
			responseType: 'arraybuffer',
			success: function(result){
				if (result.byteLength<64) {
					console.error('Downloaded file is too small: '+result.byteLength);
					m_data=[];
					m_signaler.emit('loaded');
					return;
				}
				if (m_mda.setFromArrayBuffer(result)) {
					m_signaler.emit('loaded');
				}
				else {
					m_signaler.emit('loaded');
				}
				/*
				var X=new Int32Array(result.slice(0,64));
				var num_bytes_per_entry=X[1];
				var num_dims=X[2];
				m_dims=[];
				if ((num_dims<2)||(num_dims>5)) {
					console.error('Invalid number of dimensions: '+num_dims);
					m_data=[];
					m_signaler.emit('loaded');
					return;
				} 
				for (var i=0; i<num_dims; i++) {
					m_dims.push(X[3+i]);
				}
				var dtype=get_dtype_string(X[0]);
				var header_size=(num_dims+3)*4;
				if (dtype=='float32') {
					m_data=new Float32Array(result.slice(header_size));
					m_signaler.emit('loaded');
					return;
				}
				else if (dtype=='float64') {
					m_data=new Float64Array(result.slice(header_size));
					m_signaler.emit('loaded');
					return;	
				}
				else {
					callback({success:false,error:'Unsupported data type: '+dtype});
					console.error('Unsupported dtype: '+dtype);
					m_data=[];
					m_signaler.emit('loaded');
					return;
				}
				*/
			}
		});
	}
}

/**
 *
 * jquery.binarytransport.js
 *
 * @description. jQuery ajax transport for making binary data type requests.
 * @version 1.0 
 * @author Henry Algus <henryalgus@gmail.com>
 *
 */
 
// use this transport for "binary" data type
$.ajaxTransport("+binary", function(options, originalOptions, jqXHR){
    // check for conditions and support for blob / arraybuffer response type
    if (window.FormData && ((options.dataType && (options.dataType == 'binary')) || (options.data && ((window.ArrayBuffer && options.data instanceof ArrayBuffer) || (window.Blob && options.data instanceof Blob)))))
    {
        return {
            // create new XMLHttpRequest
            send: function(headers, callback){
		// setup all variables
                var xhr = new XMLHttpRequest(),
		url = options.url,
		type = options.type,
		async = options.async || true,
		// blob or arraybuffer. Default is blob
		dataType = options.responseType || "blob",
		data = options.data || null,
		username = options.username || null,
		password = options.password || null;
					
                xhr.addEventListener('load', function(){
			var data = {};
			data[options.dataType] = xhr.response;
			// make callback and send data
			callback(xhr.status, xhr.statusText, data, xhr.getAllResponseHeaders());
                });
 
                xhr.open(type, url, async, username, password);
				
		// setup custom headers
		for (var i in headers ) {
			xhr.setRequestHeader(i, headers[i] );
		}
				
                xhr.responseType = dataType;
                xhr.send(data);
            },
            abort: function(){
                jqXHR.abort();
            }
        };
    }
});