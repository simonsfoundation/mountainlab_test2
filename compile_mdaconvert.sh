#!/bin/bash

echo "Compiling mdaconvert"
cd mdaconvert/src
qmake
make $1 -j 8
EXIT_CODE=$?
cd ../..
if [[ $EXIT_CODE -ne 0 ]]; then
	false
fi
