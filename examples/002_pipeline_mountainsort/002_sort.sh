#!/bin/bash

samplerate="30000" #Hz
freq_min="300" #Hz
freq_max="8000" #Hz
detect_threshold=3 # std.dev's
detect_interval=10 # timepoints
clip_size=50 # timepoints
sign=0 # use 1 to detect only positive spikes, -1 to detect only negative spikes, and 0 for both

mkdir output
mountainprocess run-script mountainsort.pipeline \
	--raw=raw.mda --outpath=output \
	--samplerate=$samplerate --freq_min=$freq_min --freq_max=$freq_max \
	--detect_threshold=$detect_threshold --detect_interval=$detect_interval \
	--clip_size=$clip_size --sign=$sign
