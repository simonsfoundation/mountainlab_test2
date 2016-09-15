#!/bin/bash

noise_level=1

mountainprocess run-process synthesize_timeseries_001_matlab \
	--waveforms=waveforms.mda \
	--timeseries=raw.mda \
	--firings_true=firings_true.mda \
	--M=4 --T=800 --K=20 \
	--duration=600 \
	--amp_variation_min=1 --amp_variation_max=1 \
	--firing_rate_min=0.5 --firing_rate_max=2 \
	--noise_level=$noise_level

