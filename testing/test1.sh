#!/bin/bash

base=/home/magland/dev/mdaserver/testdata/
base2=http://localhost:8000

mountainview --mode=overview2 --raw=$base/pre0.mda --filt=$base/pre1b.mda --pre=$base2/pre2.mda --firings=$base2/firings.mda --samplerate=30000.000000 
