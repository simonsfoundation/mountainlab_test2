#!/bin/bash

base1=/mnt/xfs1/home/magland/dev/mountainlab/mdaserver_data
base2=remote://localhost
base3=remote://magland

base=$base1

mountainview --mode=overview2 --raw=$base/pre0.mda --filt=$base/pre1b.mda --pre=$base/pre2.mda --firings=$base/firings.mda --samplerate=30000.000000 
