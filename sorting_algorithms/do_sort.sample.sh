#!/bin/bash

folder=/home/magland/axellab/raw/datafile001_30_mn_butter_500-6000_trimmin80
mountainprocess queue-script $PWD/alg_scda_003.js $PWD/params.par --raw=$folder/datafile001_30_mn_butter_500-6000_trimmin80.mda --geom=$folder/geom.csv --outpath=$PWD
