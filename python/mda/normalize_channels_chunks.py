#!/usr/bin/python3

import sys
from mlprocessorlibrary import MLProcessorLibrary
from mdaio import readmda, writemda32, writemda64, DiskReadMda
import numpy as np
from timeserieschunkreader import TimeseriesChunkReader

class normalize_channels:
	processor_name='normalize_channels'
	inputs=[{"name":"input","description":"input .mda file"}]
	outputs=[{"name":"output","description":"output .mda file"}]
	parameters=[]

	_chunk_size=100
	_overlap_size=0
	_reader=TimeseriesChunkReader(_chunk_size, _overlap_size)
	_sum=0
	def run(self,args):
		input_path=args['input']
		output_path=args['output']
		self._sum=0
		if not self._reader.run(input_path, self._kernel):
			return False
		print("sum={}".format(self._sum))
		M=DiskReadMda(input_path).N1()
		N=DiskReadMda(input_path).N2()
		print("M={}, N={}".format(M,N))
		print("sum2={}".format(np.sum(DiskReadMda(input_path).readChunk(i1=0,i2=0,N1=M,N2=N).ravel())))

	def _kernel(self, X, info):
		print(X.shape)
		print("info i1={} i2={} size={}".format(info.i1,info.i2,info.size))
		M=X.shape[0]
		N=X.shape[1]
		for m in range(0,M):
			row=X[m,info.i1:info.i2+1]
			self._sum=self._sum+np.sum(row)
		return True

PL=MLProcessorLibrary()
PL.addProcessor(normalize_channels())
if not PL.run(sys.argv):
	exit(-1)

