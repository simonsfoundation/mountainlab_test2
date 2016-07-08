#!/bin/bash

echo "Compiling mlcommon"
cd mlcommon/src
qmake
make $1 -j 8
EXIT_CODE=$?
cd ../..
if [[ $EXIT_CODE -ne 0 ]]; then
	false
fi
