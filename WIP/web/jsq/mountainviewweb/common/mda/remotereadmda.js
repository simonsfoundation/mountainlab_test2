function RemoteReadMda(path_in) {
	var that=this;

	this.setPath=function(path,callback) {m_path=path; m_initialized=false; initialize(callback);}
	this.path=function() {return m_path;}
	this.initialize=function(callback) {initialize(callback);}
	this.isInitialized=function() {return m_initialized;}
	this.N1=function() {return get_dim(1);}
	this.N2=function() {return get_dim(2);}
	this.N3=function() {return get_dim(3);}
	this.checksum=function() {return checksum();}
	///Retrieve a chunk of the vectorized data of size 1xN starting at position i
	this.readChunk=function(i,size,callback) {readChunk(i,size,callback);} 
	this.toMda=function(callback) {toMda(callback);}

	function initialize(callback) {
		if (!m_path) {
			console.error('path is empty in RemoteReadMda');
			return;
		}
		if (!callback) callback=function() {};
		if (m_initialized) {
			callback({success:m_initialized_success,error:m_initialized_error});
			return;
		}
		if (m_initializing) {
			setTimeout(function() {
				initialize(callback);
			},50);
			return;
		}
		m_initializing=true;
		if (m_download_failed) {
			m_initialized=true;
			m_initializing=false;
			m_initialized_success=false;
			m_initialized_error='Download already failed.';
			callback({success:m_initialized_success,error:m_initialized_error});
			return;
		}
		var url=m_path;
		var url2=url + "?a=info&output=text";
		$.get(url2,function(txt) {
			if (!txt) {
				m_download_failed=true;
				m_initialized=true;
				m_initializing=false;
				m_initialized_success=false;
				m_initialized_error='RemoteReadMda: Problem initializing mda, txt is empty: '+url2;
				callback({success:m_initialized_success,error:m_initialized_error});
				return;
			}
			var lines=txt.split('\n');
			var sizes=lines[0].split(',');
			m_info.N1=Number(sizes[0]);
			m_info.N2=Number(sizes[1]);
			m_info.N3=Number(sizes[2]);
			m_info.checksum=lines[1];
			m_info.file_last_modified=lines[2];
			m_initialized=true;
			m_initializing=false;
			m_initialized_success=true;
			m_initialized_error='';
			callback({success:m_initialized_success,error:m_initialized_error});
		});
	}
	function get_dim(d) {
		if (!m_initialized) {
			console.error('RemoteReadMda is not initialized -- cannot get dimensions: '+m_path);
			return 1;
		}
		if (d==1) return m_info.N1;
		if (d==2) return m_info.N2;
		if (d==3) return m_info.N3;
		return 1;
	}
	function readChunk(i,size,callback) {
		that.initialize(function() {
			//don't make excessive calls... once we fail, that's it.
			if ((!m_initialized)||(!m_initialized_success)) {
				callback({success:false,error:'RemoteReadMda not initialized successfully'});
				return;
			}
			if (m_download_failed) {
				callback({success:false,error:'RemoteReadMda: Download already failed.'});
				return;
			}
			var X=new Mda();
			X.allocate(size,1);
			var ii1 = i; //start index of the remote array
	    	var ii2 = i + size - 1; //end index of the remote array
	    	var jj1 = Math.floor(ii1 / m_download_chunk_size); //start chunk index of the remote array
	    	var jj2 = Math.floor(ii2 / m_download_chunk_size); //end chunk index of the remote array
	    	if (jj1 == jj2) { //in this case there is only one chunk we need to worry about
	        	download_chunk_at_index(jj1,function(ret) { //download the single chunk
	        		if (!ret.success) {
	        			callback(ret);
	        			m_download_failed=true;
	        			return;
	        		}
	        		callback({success:true,chunk:ret.chunk.getChunk(ii1-jj1*m_download_chunk_size,size)}); //starting reading at the offset of ii1 relative to the start index of the chunk
	        		return;
	        	}); 
	        }
	        else {
	        	callback({success:false,error:'RemoteReadMda: Cannot handle this case yet'});
	        	return;	
	        }
	    });
    }
    function toMda(callback) {
    	that.initialize(function() {
    		if ((!m_initialized)||(!m_initialized_success)) {
				callback({success:false,error:'RemoteReadMda not initialized successfully'});
				return;
			}
    		that.readChunk(0,that.N1()*that.N2()*that.N3(),function(res) {
    			if (res.chunk) {
	    			res.mda=res.chunk;
	    			res.chunk=undefined;
	    			res.mda.reshape(that.N1(),that.N2(),that.N3());
	    		}
    			callback(res);
    		});
    	});
    }
    function download_chunk_at_index(ii,callback) {
    	var Ntot=m_info.N1*m_info.N2*m_info.N3;
    	var size=m_download_chunk_size;
    	if (ii*m_download_chunk_size+size>Ntot) {
    		size=Ntot-ii*m_download_chunk_size;
    	}
    	if (size<=0) {
    		callback({success:false,error:'size is not positive'});
    		return;
    	}
    	if (!m_info.checksum) {
    		callback({success:false,error:'checksum is empty'});
    		return;
    	}
    	var chunk_cache_code = m_info.checksum+'-'+m_download_chunk_size+'-'+ii;
    	if (chunk_cache_code in s_chunk_cache) {
    		callback({success:true,chunk:s_chunk_cache[chunk_cache_code]});
    		return;
    	}
    	var url=m_path;
    	var url0=url+'?a=readChunk&output=text&index='+(ii * m_download_chunk_size)+'&size='+(size)+'&datatype='+(m_remote_datatype);
    	$.get(url0,function(binary_url) {
    		if (!binary_url) {
    			callback({success:false,error:'binary_url is empty'});
    			return;
    		}
    		//the following is ugly
    		var ind=m_path.indexOf('/mdaserver');
    		if (ind>0) {
    			binary_url=m_path.slice(0,ind)+'/mdaserver/'+binary_url;
    		}
    		var chunk=new Mda();
    		chunk.load(binary_url,function(res) {
    			if (!res.success) {
    				callback({success:false,error:'Error loading chunk: '+res.error+': '+binary_url});
    				return;
    			}
    			if (chunk.totalSize()!=size) {
    				callback({success:false,error:'Unexpected size of downloaded chunk: '+chunk.totalSize()+'<>'+size});
    				return;	
    			}

    			/// TODO: handle float32_8

    			s_chunk_cache[chunk_cache_code]=chunk;
    			callback({success:true,chunk:chunk});
    			return;
    		});
    	});
    }

	var m_path='';
	var m_initializing=false;
	var m_initialized=false;
	var m_initialized_error='';
	var m_initialized_success=false;
	/// TODO: important change this to 500000
	var m_download_chunk_size=5000000;
	var m_info={
		N1:1,N2:1,N3:1,checksum:'',file_last_modified:0
	}
	var m_remote_datatype='float64';
	var m_download_failed=false;

	if (path_in) {
		that.setPath(path_in);
	}
}

var s_chunk_cache={};