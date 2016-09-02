#!/bin/bash

opts="--M=5 --T=800 --K=20 --duration=600 --amp_variation_min=0.2 --amp_variation_max=1"
opts="$opts --firing_rate_min=0.5 --firing_rate_max=3"
opts="$opts --noise_level=1"

mountainprocess run-process synthesize_timeseries_001_matlab --waveforms=waveforms.mda --timeseries=raw.mda --firings_true=firings_true.mda $opts --_force_run | tee synthesize.log

mkdir output
mountainprocess run-script --_nodaemon alg_scda_009.js params.json --raw=raw.mda --geom=geom.csv --outpath=output | tee sort.log
