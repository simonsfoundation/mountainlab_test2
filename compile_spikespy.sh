#!/bin/bash

echo "Compiling spikespy"
cd spikespy/src
qmake
make -j 8

cd ../processing
qmake
make -j 8


