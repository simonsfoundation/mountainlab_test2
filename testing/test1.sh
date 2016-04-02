#!/bin/bash

base=/media/storage/ms_franklab/experiments/2016_03_15/sort_dl12_20151208_NNF_r1_tet16_17/output_tet16
base2=http://localhost:8000

mountainview --mode=overview2 --raw=$base/pre0.mda --filt=$base/pre1b.mda --pre=$base2/pre2.mda --firings=$base2/firings.mda --samplerate=30000.000000 
