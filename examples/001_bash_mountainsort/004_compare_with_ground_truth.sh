#!/bin/bash

samplerate="30000" #Hz

mountaincompare --firings1=firings_true.mda --firings2=output/firings.mda --raw=raw.mda --filt=output/pre1.mda --pre=output/pre2.mda --samplerate=$samplerate