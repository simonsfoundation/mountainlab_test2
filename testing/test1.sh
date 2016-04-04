#!/bin/bash

base1=/home/magland/dev/mdaserver/testdata
base2=http://localhost:8000
base3=http://magland.org:8000

base=$base2

mountainview --mode=overview2 --raw=$base/pre0.mda --filt=$base/pre1b.mda --pre=$base/pre2.mda --firings=$base/firings.mda --samplerate=30000.000000 
