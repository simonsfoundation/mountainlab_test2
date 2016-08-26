#!/bin/bash

script_dir=$1

cmd="matlab -nosplash -nojvm -r"
#cmd="octave --no-gui --eval"

$cmd "try, addpath('$script_dir'); $2; exit(0);, catch err, disp(getReport(err,'extended')); exit(-1);, end;"