#!/usr/bin/python3

import sys
from mlprocessorlibrary import MLProcessorLibrary
from mdaio import readmda, writemda32, writemda64

class test_py_processor:
	processor_name='test_py_processor'
	inputs=[{"name":"input","description":"input .mda file"}]
	outputs=[{"name":"output","description":"output .mda file"}]
	parameters=[{"name":"factor","optional":False,"description":"The scale factor"}]
	def run(self,args):
		### The processing
		input_path=args['input']
		output_path=args['output']
		factor=float(args['factor'])
		X=readmda(input_path) #read
		X=X*factor #scale the array
		writemda64(X,output_path) #write
		return True

PL=MLProcessorLibrary()
PL.addProcessor(test_py_processor())
if not PL.run(sys.argv):
	exit(-1)

