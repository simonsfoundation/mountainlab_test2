#!/bin/bash

# Compile mountainprocess
echo "Compiling mountainprocess"
cd mountainprocess/src
qmake
make -j 8
EXIT_CODE=$?
cd ../..
if [[ $EXIT_CODE -ne 0 ]]; then
	false
fi
