#!/bin/bash

# Compile mountainsort/cpp
echo "Compiling mountainsort helper library"
cd mountainsort/src
qmake mountainsortlib.pro
make $1 -j 8
EXIT_CODE=$?
if [[ $EXIT_CODE -ne 0 ]]; then
        false
fi
echo "Compiling mountainsort"
qmake mountainsort.pro
make $1 -j 8
EXIT_CODE=$?
cd ../..
if [[ $EXIT_CODE -ne 0 ]]; then
	false
fi
