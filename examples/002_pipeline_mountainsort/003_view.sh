#!/bin/bash

samplerate="30000" #Hz

mountainview --firings=output/firings.mda --raw=output/raw.mda.prv --filt=output/filt.mda.prv --pre=output/pre.mda.prv --samplerate=$samplerate
