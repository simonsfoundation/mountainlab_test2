#!/bin/bash

samplerate="30000" #Hz

mountaincompare --firings1=firings_true.mda --firings2=output/firings.mda --raw=output/pre0.mda.prv --filt=output/pre1.mda.prv --pre=output/pre2.mda.prv --samplerate=$samplerate