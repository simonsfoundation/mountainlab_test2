#!/bin/bash

samplerate="30000" #Hz

mountainview --firings=output/firings.mda --raw=raw.mda --filt=output/pre1.mda --pre=output/pre2.mda --samplerate=$samplerate