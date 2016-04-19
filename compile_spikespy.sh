#!/bin/bash

echo "Compiling spikespy"
cd spikespy/src
qmake
make -j 8
EXIT_CODE=$?
if [[ $EXIT_CODE -eq 0 ]]; then
	cd ../processing
	qmake
	make -j 8
else
	false
fi
