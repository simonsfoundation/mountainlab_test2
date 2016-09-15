#!/bin/bash

opts="--firing_rate_min=0.5 --firing_rate_max=3 --M=4 --duration=300"

./generate_synth_examples.node.js $opts --dsname=example1 --K=16 --noise_level=1 
./generate_synth_examples.node.js $opts --dsname=example2 --K=16 --noise_level=2 
./generate_synth_examples.node.js $opts --dsname=example3 --K=16 --noise_level=1 --amp_variation_min=0.2
./generate_synth_examples.node.js $opts --dsname=example4 --K=16 --noise_level=2 --amp_variation_min=0.2
./generate_synth_examples.node.js $opts --dsname=example5 --M=4 --K=30 --noise_level=1 --amp_variation_min=0.2
