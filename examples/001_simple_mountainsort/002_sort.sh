#!/bin/bash

samplerate="30000" #Hz
freq_min="300" #Hz
freq_max="8000" #Hz
detect_threshold=3 # std.dev's
detect_interval=10 # timepoints
clip_size=50 # timepoints
sign=0 # use 1 to detect only positive spikes, -1 to detect only negative spikes, and 0 for both

mkdir output

# Bandpass filter
mountainprocess run-process bandpass_filter --timeseries=raw.mda --timeseries_out=output/pre1.mda --samplerate=$samplerate --freq_min=$freq_min --freq_max=$freq_max

# Normalize channels (to have variance 1)
mountainprocess run-process normalize_channels --timeseries=output/pre1.mda --timeseries_out=output/pre2.mda

# Detect super-threshold events
mountainprocess run-process detect --timeseries=output/pre2.mda --detect_out=output/detect.mda --detect_threshold=$detect_threshold --detect_interval=$detect_interval --clip_size=$clip_size --sign=$sign --individual_channels=1

# Clustering
mountainprocess run-process branch_cluster_v2 --timeseries=output/pre2.mda --detect=output/detect.mda --adjacency_matrix= --firings_out=output/firings1.mda --clip_size=$clip_size --min_shell_size=150 --shell_increment=0 --num_features=10 --num_features2=10 --detect_interval=$detect_interval --consolidation_factor=0.9 --isocut_threshold=1.5

# Merge across channels
mountainprocess run-process merge_across_channels_v2 --timeseries=output/pre2.mda --firings=output/firings1.mda --firings_out=output/firings2.mda --clip_size=$clip_size

# Fit stage
mountainprocess run-process fit_stage --timeseries=output/pre2.mda --firings=output/firings2.mda --firings_out=output/firings3.mda --clip_size=$clip_size --min_shell_size=150 --shell_increment=0

# Copy to firings.mda (final output)
mountainprocess run-process copy --input=output/firings3.mda --output=output/firings.mda

