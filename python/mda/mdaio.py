import numpy as np
import struct

class MdaHeader:
	dt_code=0
	dt='float32'
	num_bytes_per_entry=0
	num_dims=2
	dimprod=1
	dims=[]
	header_size=0

class DiskReadMda:
	_path=''
	_header=MdaHeader()
	def __init__(self,path):
		self._path=path
		self._header=_read_header(self._path)
	def dims(self):
		return self._header.dims
	def readChunk(self,i1=-1,i2=-1,i3=-1,N1=1,N2=1,N3=1):
		if (i2<0):
			return self._read_chunk_1d(i1,N1)
		elif (i3<0):
			if N1 != self._header.dims[0]:
				print("Unable to support N1 {} != {}".format(N1,self._header.dims[0]))
				return None
			X=self._read_chunk_1d(i1+N1*i2,N1*N2)
			return np.reshape(X,(N1,N2),order='F')
		else:
			if N1 != self._header.dims[0]:
				print("Unable to support N1 {} != {}".format(N1,self._header.dims[0]))
				return None
			if N2 != self._header.dims[1]:
				print("Unable to support N2 {} != {}".format(N2,self._header.dims[1]))
				return None
			X=self._read_chunk_1d(i1+N1*i2+N1*N2*i3,N1*N2*N3)
			return np.reshape(X,(N1,N2,N3),order='F')
	def _read_chunk_1d(self,i,N):
		f=open(self._path,"rb")
		try:
			f.seek(self._header.header_size+self._header.num_bytes_per_entry*i)
			ret=np.fromfile(f,dtype=self._header.dt,count=N)
			print(ret.shape)
			f.close()
			return ret
		except Exception as e: # catch *all* exceptions
			print(e)
			f.close()
			return None

def _read_header(path):
	f=open(path,"rb")
	try:
		dt_code=_read_int32(f)
		num_bytes_per_entry=_read_int32(f)
		num_dims=_read_int32(f)
		if (num_dims<2) or (num_dims>6):
			print("Invalid number of dimensions: {}".format(num_dims))
			return None
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
			return None
		H=MdaHeader()
		H.dt_code=dt_code
		H.dt=dt
		H.num_bytes_per_entry=num_bytes_per_entry
		H.num_dims=num_dims
		H.dimprod=dimprod
		H.dims=dims
		H.header_size=4*(3+H.num_dims)
		f.close()
		return H;
	except Exception as e: # catch *all* exceptions
		print(e)
		f.close()
		return None

def readmda(path):
	H=_read_header(path)
	if (H is None):
		return None
	ret=np.array([])
	f=open(path,"rb")
	try:
		f.seek(H.header_size)
		#This is how I do the column-major order
		ret=np.fromfile(f,dtype=H.dt,count=H.dimprod)
		ret=np.reshape(ret,H.dims,order='F')
		f.close()
		return ret
	except Exception as e: # catch *all* exceptions
		print(e)
		f.close()
		return None

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
		for j in range(0,X.ndim):
			_write_int32(f,X.shape[j])
		#This is how I do column-major order
		A=np.reshape(X,X.size,order='F').astype(dt)
		A.tofile(f)
	except Exception as e: # catch *all* exceptions
		print(e)
	finally:
		f.close()
		return True

def _read_int32(f):
	return struct.unpack('<i',f.read(4))[0];

def _write_int32(f,val):
	f.write(struct.pack('<i',val))

def mdaio_test():
	M=4
	N=12
	X=np.ndarray((M,N))
	for n in range(0,N):
		for m in range(0,M):
			X[m,n]=n*10+m
	writemda32(X,'tmp1.mda')
	Y=readmda('tmp1.mda')
	print(np.absolute(X-Y).max())
	Z=DiskReadMda('tmp1.mda')
	print(Z.readChunk(i1=0,i2=4,N1=M,N2=N-4))

mdaio_test()