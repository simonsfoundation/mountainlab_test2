#!/bin/bash

mountainprocess run-process synthesize1 --waveforms=waveforms.mda --waveforms_oversamp=10 --info=info.mda --timeseries_out=raw.mda --firings_true=firings_true.mda --N=1e7 --noise_level=0.1 --samplerate=30000

mountainprocess run-script --_nodaemon alg_scda_005.js params.par --raw=raw.mda --geom=geom.csv --outpath=.
