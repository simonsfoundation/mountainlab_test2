from mdaio import DiskReadMda

class TimeseriesChunkInfo:
	i1=0
	i2=0
	size=0

class TimeseriesChunkReader:
	_chunk_size=0
	_overlap_size=0
	def __init__(self, chunk_size, overlap_size):
		self._chunk_size=chunk_size
		self._overlap_size=overlap_size	
	def run(self, mdafile_path, func):
		X=DiskReadMda(mdafile_path)
		M=X.N1()
		N=X.N2()
		sections=[]
		t=0
		while t < N:
			t1=t
			t2=min(N-1,t+self._chunk_size-1)
			s1=max(0,t1-self._overlap_size)
			s2=min(N-1,t2+self._overlap_size)
			chunk=X.readChunk(i1=0, N1=M, i2=s1, N2=s2-s1+1)
			info=TimeseriesChunkInfo()
			info.i1=t1-s1
			info.i2=t2-s1
			info.size=t2-t1+1
			if not func(chunk, info):
				return False
			t=t+self._chunk_size
		return True

