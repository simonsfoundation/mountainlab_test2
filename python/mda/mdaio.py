import numpy as np
import struct

def readmda(path):
	ret=np.array([])
	f=open(path,"rb")
	try:
		dt_code=_read_int32(f)
		num_bytes_per_entry=_read_int32(f)
		num_dims=_read_int32(f)
		if (num_dims<2) or (num_dims>6):
			print("Invalid number of dimensions: {}".format(num_dims))
			return ret
		dims=[];
		dimprod=1

		for j in range(0,num_dims):
			tmp0=_read_int32(f)
			dimprod=dimprod*tmp0
			dims.append(tmp0)
		if dt_code == -2:
			dt='uint8'
		elif dt_code == -3:
			dt='float32'
		elif dt_code == -4:
			dt='int16'
		elif dt_code == -5:
			dt='int32'
		elif dt_code == -6:
			dt='uint16'
		elif dt_code == -7:
			dt='float64'
		elif dt_code == -8:
			dt='uint32'
		else:
			print("Invalid data type code: {}".format(dt_code))
			return ret
		print(dims)
		ret=np.fromfile(f,dtype=dt,count=dimprod)
		ret=np.reshape(ret,dims)
	finally:
		f.close()
	return ret

def writemda32(X,fname):
	return _writemda(X,fname,'float32')

def writemda64(X,fname):
	return _writemda(X,fname,'float64')

def writemda8(X,fname):
	return _writemda(X,fname,'uint8')

def writemda32i(X,fname):
	return _writemda(X,fname,'int32')

def writemda32ui(X,fname):
	return _writemda(X,fname,'uint32')	

def writemda16i(X,fname):
	return _writemda(X,fname,'int16')	

def writemda16ui(X,fname):
	return _writemda(X,fname,'uint16')	

def _writemda(X,fname,dt):
	print("hello1")
	dt_code=0
	num_bytes_per_entry=0
	if dt == 'uint8':
		dt_code=-2
		num_bytes_per_entry=1
	elif dt == 'float32':
		dt_code=-3
		num_bytes_per_entry=4
	elif dt == 'int16':
		dt_code=-4
		num_bytes_per_entry=2
	elif dt == 'int32':
		dt_code=-5
		num_bytes_per_entry=4
	elif dt == 'uint16':
		dt_code=-6
		num_bytes_per_entry=2
	elif dt == 'float64':
		dt_code=-7
		num_bytes_per_entry=8
	elif dt == 'uint32':
		dt_code=-8
		num_bytes_per_entry=4
	else:
		print("Unexpected data type: {}".format(dt))
		return False

	f=open(fname,'wb')
	try:
		_write_int32(f,dt_code)
		_write_int32(f,num_bytes_per_entry)
		_write_int32(f,X.ndim)
		print(X.shape[0])
		print(X.ndim)
		for j in range(0,X.ndim):
			_write_int32(f,X.shape[j])
		print(X.data)
		X.data.astype(dt).tofile(f)
	finally:
		f.close()
		return True

def _read_int32(f):
	return struct.unpack('<i',f.read(4))[0];

def _write_int32(f,val):
	f.write(struct.pack('<i',val))
