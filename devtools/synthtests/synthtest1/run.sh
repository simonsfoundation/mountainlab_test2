#!/bin/bash

mountainprocess run-process synthesize1 --waveforms=waveforms.mda --waveforms_oversamp=10 --info=info.mda --timeseries_out=raw.mda --firings_true=firings_true.mda --N=1e7 --noise_level=0.1 --samplerate=30000 | tee synthesize.log

mkdir output
mountainprocess run-script --_nodaemon alg_scda_008.js params.json --raw=raw.mda --geom=geom.csv --outpath=output | tee sort.log
