#!/bin/bash

mkdir output
mountainprocess run-script --_nodaemon alg_scda_008.js params.json --raw=raw.mda --outpath=output | tee sort.log
