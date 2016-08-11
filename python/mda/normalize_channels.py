#!/usr/bin/python2

import sys
from mlprocessorlibrary import MLProcessorLibrary
from mdaio import readmda, writemda32, writemda64
import numpy as np

class normalize_channels:
	processor_name='normalize_channels'
	inputs=[{"name":"input","description":"input .mda file"}]
	outputs=[{"name":"output","description":"output .mda file"}]
	parameters=[]
	def run(self,args):
		### The processing
		input_path=args['input']
		output_path=args['output']
		X=readmda(input_path) #read
		M=X.shape[0]
		N=X.shape[1]
		print(X.shape)
		for m in range(0,M):
			row=X[m,:]
			print(row.shape)
			stdev0=np.std(row)
			row=row*(1/stdev0)
			X[m,:]=row
		writemda64(X,output_path) #write
		return True

PL=MLProcessorLibrary()
PL.addProcessor(normalize_channels())
if not PL.run(sys.argv):
	exit(-1)

