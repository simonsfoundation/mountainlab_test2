#!/bin/bash

mountainprocess run-process synthesize_timeseries_001_matlab --waveforms=waveforms.mda --timeseries=raw.mda --firings_true=firings_true.mda | tee synthesize.log

mkdir output
mountainprocess run-script --_nodaemon alg_scda_008_prv.js params.json --raw=raw.mda --geom=geom.csv --outpath=output | tee sort.log
