#!/bin/bash

echo "Compiling spikespy"
cd spikespy/src
qmake
make -j 8


